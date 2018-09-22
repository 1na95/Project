// FolderWatcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <tchar.h>
#include "dirent.h"
#include <string>
#include <stdio.h>  
#include <windows.h>  
#include <winreg.h>  
#include <tchar.h>
#include <strsafe.h>
#include <Dbt.h>
#include <WinUser.h>
#include <WinBase.h>
#include <stdlib.h>
#include <time.h>
#include <fileapi.h>
#include <direct.h>
#include <mysql.h>

#pragma warning(disable:4996)
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "libmysql.lib")
#define _CRT_SECURE_NO_WARNINGS

void RefreshDirectory(LPTSTR);
void RefreshTree(LPTSTR);
void WatchDirectory(LPTSTR);
void GetDirList(char* lpDir, int indent);
void printByIndentLevel(char* str, int indent);
char* concatPath(char* prefix, char* path);


using namespace std;

HANDLE hDir = CreateFileW(L"D:\\", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
	0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
CONST DWORD cbBuffer = 1024 * 1024;
BYTE* pBuffer = (PBYTE)malloc(cbBuffer);
BOOL bWatchSubtree = FALSE;
DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
DWORD bytesReturned;
WCHAR temp[MAX_PATH] = { 0 };


/*void _tmain(int argc, TCHAR *argv[])
{
	if (argc != 2)
	{
		_tprintf(TEXT("Usage: %s <dir>\n"), argv[0]);
		return;
	}

	WatchDirectory(argv[1]);
}*/


#pragma warning(disable:4996)
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "libmysql.lib")

#define DB_HOST "127.0.0.1"
#define DB_USER "guest"
#define DB_PASS "1234"
#define DB_NAME "USBSecure"

void getMySerialNum(char* currentSerial, char serialPath[255]);
BOOL isRegisterdSerial(char* currentSerial, char* DBSerial);
void detection(char* pFolderName, char* pMySerial);
void getFolderName(char* currentSerial, char* folderName);
//void connectMysql();



int main()
{
	char serial_buffer[255];
	char mySerial[255];
	char* pMySerial = mySerial;
	char DBSerial[255];
	char* pDBSerial = DBSerial;
	char folderName[255];
	char* pFolderName = folderName;
	
	printf("\n");
	printf(">>>>>>>>>> Getting Serial Path <<<<<<<<<<\n");
	char serialPath[255];    // 파일을 읽을 때 사용할 임시 공간
	FILE *fp = fopen("serialPath.txt", "r");    // 파일을 읽기 모드로 열기.  
	fgets(serialPath, sizeof(serialPath), fp);    // 문자열을 읽음
	fclose(fp);    // 파일 포인터 닫기
	printf("Serial Path : %s\n", serialPath);    //파일의 내용 출력

	getMySerialNum(pMySerial, serialPath);

	printf("\tREAD  REGISTER\n");
	printf("\tKEY   [HKEY_LOCAL_MACHINE"
		"\\SYSTEM\\ControlSet001\\Enum\\USB ] \n");
	printf("\tVALUE [HardwareID] =  [%p]\n", mySerial);

	if (isRegisterdSerial(pMySerial, pMySerial) == TRUE) {
		getFolderName(pMySerial, pFolderName);
		// Start Detecting
		//detection(pFolderName, pMySerial);
		WatchDirectory(folderName);
	}
	else {
		printf("Unregisterd Device");
	}
	while (1);
}

// 출처: http://bmfrog.tistory.com/entry/레지스트리-키값을-읽는-예제 [일타삼피]

void getMySerialNum(char* currentSerial, char serialPath[255]) {
	// Get current USB serial num
	printf("\n>>>>>>>>>> Getting USB Serial <<<<<<<<<<\n");

	LONG    ret;
	HKEY    hKey;
	TCHAR    data_buffer[255];
	DWORD   data_type;
	DWORD   data_size;
	//LPCSTR	regPath = "SYSTEM\CurrentControlSet\Enum\USB\VID_04E8&PID_61F4\MSFT301234567AE9B2\\HardwareID";

	printf("Serial Path : %s\n", (LPCSTR)serialPath);
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,                                // 키 값  
		(LPCSTR)serialPath,    // 서브 키값   
		0,                                                  // 옵션 항상 0  
		KEY_READ | KEY_WOW64_64KEY,                                    // 접근 권한  
		&hKey                                               // 키 핸들  
	);

	printf("ret : %ld\n", ret);

	if (ret == ERROR_SUCCESS)
	{
		printf("\n>>>>>>>>>> Getting Registry Value <<<<<<<<<<\n");
		memset(data_buffer, 0, sizeof(data_buffer));
		data_size = sizeof(data_buffer);
		RegQueryValueEx(hKey,                // RegOpenKeyEx에 얻어진 키 핸들  
			TEXT("HarwareID"),                    // 키 안에 값 이름   
			0,                                    // 옵션 항상 0   
			&data_type,                           // 얻어진 데이터 타입  
			(LPBYTE)data_buffer,                  // 얻어진 데이터  
			(DWORD *)&data_size                  // 얻어진 데이터 크기   
		);

		currentSerial = data_buffer;
		RegCloseKey(hKey);
	}
	else {
		printf("키 오픈 실패\n");
	}

	printf("\tKEY   [HKEY_LOCAL_MACHINE"
		"\\SYSTEM\\ControlSet001\\Enum\\USB ] \n");
	printf("\tVALUE [HardwareID] =  [%s]\n", *data_buffer);
}

void getDBSerialNum(char* DBSerial) {
	// Get USB Serial num stored in DB
}

void getFolderName(char* currentSerial, char* folderName) {
	getcwd(folderName, 256);
	printf("\n\t...Trying to get drive name...\n");
	printf("\t Drive Path : %s\n", (LPCSTR)folderName);
}

BOOL isRegisterdSerial(char* currentSerial, char* DBSerial) {
	//  Compare Current Serial with Registerd Serial
	if (currentSerial == DBSerial)
		return TRUE;
	else
		return FALSE;
}
/*
int detection(char* pFolderName, char* pMySerial) {
	// Detect
	MYSQL		*connection = NULL, conn;
	MYSQL_RES	*sql_result;
	MYSQL_ROW	sql_row;
	int			query_stat;
	printf("\t...Connecting to DB...\n");

	char name[12];
	char address[80];
	char tel[12];
	char query[255];

	// 사용자 정보
	char id[20] = "hanna";

	BOOL isDetected = FALSE;
	char dMsg[30];
	time_t now;

	char folderName[256] = "D:\\";

	struct RequestDiskInfoData {
		DWORD buffer_size;
		char driver_register_value[8];
		char drive_num;
		char reserved[20];
		char dummy[1];
	};

	struct DiskInfoData {
		DWORD buffer_size;
		char status[12];
		char buffer[512];
	};

	RequestDiskInfoData request_data;
	DiskInfoData result_data;
	DWORD copy_bytes = 0;

	HANDLE h =
		mysql_init(&conn);

	connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);

	if (connection == NULL) {
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
		return 1;
	}

	// 디렉토리 변경 사항 알림
	HANDLE hDir = CreateFileA(folderName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, 0);

	CONST DWORD cbBuffer = 1024 * 1024;
	BYTE* pBuffer = (PBYTE)malloc(cbBuffer);
	BOOL bWatchSubtree = FALSE;
	DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
	DWORD bytesReturned;
	WCHAR temp[MAX_PATH] = { 0 };

	for (;;)
	{
		FILE_NOTIFY_INFORMATION* pfni;
		BOOL fOk = ReadDirectoryChangesW(hDir, pBuffer, cbBuffer,
			bWatchSubtree, dwNotifyFilter, &bytesReturned, 0, 0);

		if (!fOk)
		{
			DWORD dwLastError = GetLastError();
			printf("error : %d\n", dwLastError);
			break;
		}

		pfni = (FILE_NOTIFY_INFORMATION*)pBuffer;

		// DB로 변경사항 전송
		do {
			printf("NextEntryOffset(%d)\n", pfni->NextEntryOffset);
			switch (pfni->Action)
			{
			case FILE_ACTION_ADDED:
				isDetected = TRUE;
				strcpy_s(dMsg, "파일이 추가되었습니다.");
				wprintf(L"FILE_ACTION_ADDED\n");
				break;
			case FILE_ACTION_REMOVED:
				isDetected = TRUE;
				strcpy_s(dMsg, "파일이 삭제되었습니다.");
				wprintf(L"FILE_ACTION_REMOVED\n");
				break;
				case FILE_ACTION_MODIFIED:
					isDetected = TRUE;
					strcpy_s(dMsg, "FILE_ACTION_MODIFIED");
					wprintf(L"FILE_ACTION_MODIFIED\n");
					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					isDetected = TRUE;
					strcpy_s(dMsg, "FILE_ACTION_OLD_NAME");
					wprintf(L"FILE_ACTION_RENAMED_OLD_NAME\n");
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					isDetected = TRUE;
					strcpy_s(dMsg, "FILE_ACTION_NEW_NAME");
					wprintf(L"FILE_ACTION_RENAMED_NEW_NAME\n");
					break;
			default:
				if (isDetected) {
					time(&now);
					char currentSerial[10] = "1234";
					sprintf_s(query, "insert into board values ('%s', '%s', '%s', '%s')",
						id, currentSerial, ctime(&now), dMsg);
					query_stat = mysql_query(connection, query);
				}
				if (query_stat != 0) {
					fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
					return 1;
				}
				isDetected = FALSE;
				break;
			}
			printf("FileNameLength(%d)\n", pfni->FileNameLength);

			StringCbCopyNW(temp, sizeof(temp), pfni->FileName, pfni->FileNameLength);

			wprintf(L"FileName(%s)\n", temp);

			pfni = (FILE_NOTIFY_INFORMATION*)((PBYTE)pfni + pfni->NextEntryOffset);
		} while (pfni->NextEntryOffset > 0);
	}
}
*/

/*
void connectMysql() {
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char server[10] = "localhost";
	char user[10] = "guest";
	char password[10] = "1234";
	char database[100] = "usbsecure";
	conn = mysql_init(NULL);

	if (!mysql_real_connect(conn, server,
		user, password, database, 0, NULL, 0)) {
		printf("%s\n", mysql_error(conn));
		exit(1);
	}


	if (mysql_query(conn, "show tables")) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	res = mysql_use_result(conn);


	printf("MySQL Tables in mysql database:\n");
	while ((row = mysql_fetch_row(res)) != NULL)
		printf("%s \n", row[0]);

	mysql_free_result(res);
	mysql_close(conn);
}
*/

void detection(char* pPath, char* pMySerial) {

}

void WatchDirectory(LPTSTR lpDir)
{
	DWORD dwWaitStatus;
	HANDLE dwChangeHandles[3];

	// Watch the directory for file creation and deletion. 

	dwChangeHandles[0] = FindFirstChangeNotification(
		lpDir,                         // directory to watch 
		TRUE,                         // do not watch subtree 
		FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes 

	if (dwChangeHandles[0] == INVALID_HANDLE_VALUE)
	{
		printf("\n ERROR: FindFirstChangeNotification function failed.\n");
		ExitProcess(GetLastError());
	}

	// Watch the subtree for directory creation and deletion. 

	dwChangeHandles[1] = FindFirstChangeNotification(
		lpDir,                       // directory to watch 
		TRUE,                          // watch the subtree 
		FILE_NOTIFY_CHANGE_DIR_NAME);  // watch dir name changes 

	if (dwChangeHandles[1] == INVALID_HANDLE_VALUE)
	{
		printf("\n ERROR: FindFirstChangeNotification function failed.\n");
		ExitProcess(GetLastError());
	}

	// Watch the subtree for directory creation and deletion. 

	dwChangeHandles[2] = FindFirstChangeNotification(
		lpDir,                       // directory to watch 
		TRUE,                          // watch the subtree 
		FILE_NOTIFY_CHANGE_SIZE);  // watch dir name changes 

	if (dwChangeHandles[2] == INVALID_HANDLE_VALUE)
	{
		printf("\n ERROR: FindFirstChangeNotification function failed.\n");
		ExitProcess(GetLastError());
	}

	// Make a final validation check on our handles.

	if ((dwChangeHandles[0] == nullptr) || (dwChangeHandles[1] == nullptr) || (dwChangeHandles[2] == nullptr))
	{
		printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
		ExitProcess(GetLastError());
	}

	// Change notification is set. Now wait on both notification 
	// handles and refresh accordingly. 

	_tprintf(TEXT("Waiting for change notification on %s...\n"), lpDir);

	while (true)
	{
		/*printf("Current Directory:\n");
		GetDirList(static_cast<char*>(lpDir), 0);
		printf("------------------------------------------------\n");*/

		// Wait for notification.
		dwWaitStatus = WaitForMultipleObjects(3, dwChangeHandles, FALSE, INFINITE);

		switch (dwWaitStatus)
		{
		case WAIT_OBJECT_0:

			// A file was created, renamed, or deleted in the directory.
			// Refresh this directory and restart the notification.

			RefreshDirectory(lpDir);
			if (FindNextChangeNotification(dwChangeHandles[0]) == FALSE)
			{
				printf("\n ERROR: FindNextChangeNotification function failed.\n");
				ExitProcess(GetLastError());
			}
			break;

		case WAIT_OBJECT_0 + 1:

			// A directory was created, renamed, or deleted.
			// Refresh the tree and restart the notification.

			RefreshTree(lpDir);
			if (FindNextChangeNotification(dwChangeHandles[1]) == FALSE)
			{
				printf("\n ERROR: FindNextChangeNotification function failed.\n");
				ExitProcess(GetLastError());
			}
			break;

		case WAIT_OBJECT_0 + 2:

			// A file was modified.
			// Refresh the tree and restart the notification.

			RefreshTree(lpDir);
			if (FindNextChangeNotification(dwChangeHandles[2]) == FALSE)
			{
				printf("\n ERROR: FindNextChangeNotification function failed.\n");
				ExitProcess(GetLastError());
			}
			break;

		case WAIT_TIMEOUT:

			// A timeout occurred, this would happen if some value other 
			// than INFINITE is used in the Wait call and no changes occur.
			// In a single-threaded environment you might not want an
			// INFINITE wait.

			printf("\nNo changes in the timeout period.\n");
			break;

		default:
			printf("\n ERROR: Unhandled dwWaitStatus.\n");
			ExitProcess(GetLastError());
		}
	}
}

void RefreshDirectory(LPTSTR lpDir)
{
	_tprintf(TEXT("Directory (%s) changed.\n"), lpDir);
}

void RefreshTree(LPTSTR lpDir)
{
	_tprintf(TEXT("Directory tree (%s) changed.\n"), lpDir);
}

void GetDirList(char* lpDir, int indent)
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(lpDir)) != nullptr) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != nullptr) {
			if (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
				printByIndentLevel(ent->d_name, indent);
				char* newName = concatPath(lpDir, ent->d_name);
				GetDirList(newName, indent + 2);
				free(newName);
			}
			else if (ent->d_type == DT_REG) {
				printByIndentLevel(ent->d_name, indent);
			}
		}
		closedir(dir);
	}
	else {
		/* could not open directory */
		perror("");
	}
}

void printByIndentLevel(char* str, int indent)
{
	for (auto i = 0; i < indent; i++) printf(" ");
	printf("%s\n", str);
}

char* concatPath(char* prefix, char* path)
{
	auto newName = static_cast<char*>(malloc(512 * sizeof(char)));
	strcpy(newName, prefix);
	strcat(newName, path);
	strcat(newName, "\\");
	return newName;
}