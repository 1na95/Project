#include <mysql.h>
#include <string.h>
#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <strsafe.h>
#include <Windows.h>

#define DB_HOST "127.0.0.1" //"172.30.1.24"
#define DB_USER "guest"
#define DB_PASS "1234"
#define DB_NAME "usbsecure"

#pragma warning(disable:4996)
//#pragma comment(lib, "user32.lib")
//#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "libmysql.lib")
#define _CRT_SECURE_NO_WARNINGS

BOOL compareSerial(MYSQL* connection, MYSQL conn, char serial[100]);
void getMySerialNum(char currentSerial[100], char serialPath[255]);
char *replaceAll(char *s, const char *olds, const char *news);
BOOL getFolderName(char* folderName);
BOOL directoryChange(char serial[100], MYSQL* connection, MYSQL conn);
void getTime(char currentTIme[100]);
BOOL transChange(MYSQL* connection, MYSQL conn, char serial[100], char content[100], char currentTime[100]);
void replaceChar(char serial[100]);

int main() {

	MYSQL       *connection = NULL, conn;
	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;
	int       query_stat;
	int*	pquery_stat = &query_stat;

	char id[20];
	char serial[100];
	char* pSerial = serial;
	char content[200];
	char time[100];
	char query[255];
	char dbSerial[100] = { 0 };
	char* pId = id;
	char* pContent = content;
	char* pTime = time;
	char* pBuffer;

	char serialPath[255] = "SYSTEM\\CurrentControlSet\\Enum\\";    // 시리얼 경로 파일을 읽을 때 사용할 임시 공간
	char tmpSerialPath[255];

	char folderName[255];	// 현재 경로를 저장할 배열
	char* pFolderName = folderName;

	mysql_init(&conn);

	connection = mysql_real_connect(&conn, DB_HOST,
		DB_USER, DB_PASS,
		DB_NAME, 3306,
		(char *)NULL, 0);

	if (connection == NULL)
	{
		MessageBox(NULL, "네트워크 설정을 확인해주세요.", "Error", MB_OK);
		fprintf(stderr, "Mysql connection error : %s\n", mysql_error(&conn));
		return 1;
	}
	else {
		printf(">>>>>>>>>> Mysql connect success <<<<<<<<<<\n");
	}

	printf("\n");
	printf(">>>>>>>>>> Getting Serial Path <<<<<<<<<<\n");
	FILE *fp = fopen("serialPath.txt", "r");    // 파일을 읽기 모드로 열기.  

	fgets(serial, sizeof(serial), fp);    // 문자열을 읽음
	fclose(fp);    // 파일 포인터 닫기
	/*strcat(serialPath, tmpSerialPath);
	printf("Serial Path : %s\n", serialPath);    //파일의 내용 출력

	getMySerialNum(pSerial, serialPath);*/

	// 등록된 시리얼이면 디렉터리 변경 감시 시작
	if (compareSerial(connection, conn, pSerial) == TRUE) {
		replaceChar(serial);
		if (!getFolderName(pFolderName)) {
			MessageBox(NULL, "현재 경로를 얻지 못했습니다.", "Error", MB_OK);
			return 1;
		}
		if (!directoryChange(pSerial, connection, conn)) {
			MessageBox(NULL, "파일 변경을 감지할 수 없습니다.", "Error", MB_OK);
			return 1;
		}
	}
	else
		return 1;

	mysql_close(connection);

	while (1);}

BOOL compareSerial(MYSQL* connection, MYSQL conn, char serial[100]) {
	int       query_stat;
	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;
	BOOL	 isRegisterdSerial = FALSE;
	query_stat = mysql_query(connection, "select * from serial");

	if (query_stat != 0)
	{
		fprintf(stderr, ">>>>>>>>>> Mysql query error : %s <<<<<<<<<<", mysql_error(&conn));
		return 1;
	}

	sql_result = mysql_store_result(connection);
	printf("\n>>>>>>>>>> Comparing USB Serial <<<<<<<<<<\n");

	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
	{
		if (strcmp(sql_row[0], serial) == 0) {
			printf("등록된 USB입니다,\n");
			printf("시리얼 : %s\n아이디 : %s\n", sql_row[0], sql_row[1]);
			isRegisterdSerial = TRUE;
			return TRUE;
		}
	}
	if (!isRegisterdSerial) {
		MessageBox(NULL, "등록되지 않은 USB입니다.", "Error", MB_OK);
		return FALSE;
	}

	mysql_free_result(sql_result);
}

void getMySerialNum(char currentSerial[100], char serialPath[255]) {
	// Get current USB serial num
	printf("\n>>>>>>>>>> Getting USB Serial <<<<<<<<<<\n");

	LONG    ret;
	HKEY    hKey;
	TCHAR    data_buffer[255];
	DWORD   data_type;
	DWORD   data_size;
	char	slash = '\\';

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,                                // 키 값  
		serialPath,    // 서브 키값   
		0,                                                  // 옵션 항상 0  
		KEY_READ | KEY_WOW64_64KEY,                                    // 접근 권한  
		&hKey                                               // 키 핸들  
	);

	if (ret == ERROR_SUCCESS)
	{
		printf("\n>>>>>>>>>> Getting Registry Value <<<<<<<<<<\n");
		memset(data_buffer, 0, sizeof(data_buffer));
		data_size = sizeof(data_buffer);
		RegQueryValueEx(hKey,                // RegOpenKeyEx에 얻어진 키 핸들  
			TEXT("HardwareID"),                    // 키 안에 값 이름   
			0,                                    // 옵션 항상 0   
			&data_type,                           // 얻어진 데이터 타입  
			(LPBYTE)data_buffer,                  // 얻어진 데이터  
			(DWORD *)&data_size                  // 얻어진 데이터 크기   
		);
		strcpy(currentSerial, data_buffer);

		printf("나의 시리얼 : %s\n", currentSerial);
		RegCloseKey(hKey);
	}
	else {
		printf("키 오픈 실패\n");
	}
}

char *replaceAll(char *s, const char *olds, const char *news) {
	char *result, *sr;
	size_t i, count = 0;
	size_t oldlen = strlen(olds); if (oldlen < 1) return s;
	size_t newlen = strlen(news);

	if (newlen != oldlen) {
		for (i = 0; s[i] != '\0';) {
			if (memcmp(&s[i], olds, oldlen) == 0) count++, i += oldlen;
			else i++;
		}
	}
	else i = strlen(s);

	result = (char *)malloc(i + 1 + count * (newlen - oldlen));
	if (result == NULL) return NULL;

	sr = result;
	while (*s) {
		if (memcmp(s, olds, oldlen) == 0) {
			memcpy(sr, news, newlen);
			sr += newlen;
			s += oldlen;
		}
		else *sr++ = *s++;
	}
	*sr = '\0';

	return result;
}

BOOL getFolderName(char* folderName) {
	if (getcwd(folderName, 256)) {
		printf("\n>>>>>>>>>>Trying to get drive name<<<<<<<<<<\n");
		printf("Drive Path : %s\n", (LPCSTR)folderName);
		return TRUE;
	}
	else FALSE;
}

BOOL directoryChange(char serial[100], MYSQL* connection, MYSQL conn) {
	char *pSerial = serial;
	char folderName[255];
	char* pFolderName = folderName;
	getcwd(pFolderName, 256);
	const wchar_t *folderPath;
	char action[50];
	char content[100] = { 0 };
	char* pContent = content;
	char filename[100] = { 0 };
	char* pFileName;
	char currentTime[100] = { 0 };
	char* pCurrentTime = currentTime;
	BOOL isChangedCorrectly = TRUE;

	int nChars = MultiByteToWideChar(CP_ACP, 0, pFolderName, -1, NULL, 0);
	folderPath = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, pFolderName, -1, (LPWSTR)folderPath, nChars);

	time_t curr;
	struct tm *d;
	curr = time(NULL);

	HANDLE hDir = CreateFileW(folderPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
	CONST DWORD cbBuffer = 1024 * 1024;
	BYTE* pBuffer = (PBYTE)malloc(cbBuffer);
	BOOL bWatchSubtree = FALSE;
	DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
	DWORD bytesReturned;
	WCHAR temp[MAX_PATH] = { 0 };

	time_t rawtime;

	for (;;)
	{
		FILE_NOTIFY_INFORMATION* pfni;
		BOOL fOk = ReadDirectoryChangesW(hDir, pBuffer, cbBuffer,
			bWatchSubtree, dwNotifyFilter, &bytesReturned, 0, 0);
		if (!fOk)
		{
			DWORD dwLastError = GetLastError();
			printf("error : %d\n", dwLastError);
			return FALSE;
			break;
		}

		pfni = (FILE_NOTIFY_INFORMATION*)pBuffer;

		do {
			printf("\n");
			switch (pfni->Action)
			{
			case FILE_ACTION_ADDED:
				strcpy(action, "_ADDED");
				break;
			case FILE_ACTION_REMOVED:
				strcpy(action, "_REMOVED");
				break;
			case FILE_ACTION_MODIFIED:
				strcpy(action, "_MODIFIED");
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				strcpy(action, "_RENAMEDE_OLD_NAME");
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:
				strcpy(action, "_RENAMED_NEW_NAME");
				break;
			default:
				break;
			}

			pfni = (FILE_NOTIFY_INFORMATION*)((PBYTE)pfni + pfni->NextEntryOffset);
			
			StringCbCopyNW(temp, sizeof(temp), pfni->FileName, pfni->FileNameLength);
			

			// temp wchar -> char
			wcstombs(filename, temp, 100);

			strcpy(content, filename);
			strcat(content, action);

			// 시리얼, 내용, 시간을 DB로 전송
			printf(">>>>>>>>>> 전송할 내용 <<<<<<<<<<\n");
			wprintf(L"파일 이름 : %s\n", temp);
			printf("시리얼 : %s\n", serial);
			printf("내용 : %s\n", content);
			// 시간함수
			getTime(pCurrentTime);
			printf("시간 : %s\n", currentTime);

			transChange(connection, conn, pSerial, pContent, pCurrentTime);

		} while (pfni->NextEntryOffset > 0);
	}
}

void getTime(char currentTime[100]) {
	time_t curr;
	struct tm *d;
	char mt[100] = {};

	char buffer[65] = { 0 };
	char year[] = "년 ";
	char month[] = "월 ";
	char day[] = "일 ";
	char hour[] = "시 ";
	char min[] = "분 ";
	char sec[] = "초 ";

	curr = time(NULL);
	d = localtime(&curr);

	int num;
	num = d->tm_year + 1900;
	itoa(num, buffer, 10);
	strcat(mt, buffer);
	strcat(mt, year);

	num = d->tm_mon + 1;
	itoa(num, buffer, 10);
	strcat(mt, buffer);
	strcat(mt, month);

	num = d->tm_mday;
	itoa(num, buffer, 10);
	strcat(mt, buffer);
	strcat(mt, day);

	num = d->tm_hour;
	itoa(num, buffer, 10);
	strcat(mt, buffer);
	strcat(mt, hour);

	num = d->tm_min;
	itoa(num, buffer, 10);
	strcat(mt, buffer);
	strcat(mt, min);

	num = d->tm_sec + 1;
	itoa(num, buffer, 10);
	strcat(mt, buffer);
	strcat(mt, sec);

	strcpy(currentTime, mt);
}

BOOL transChange(MYSQL* connection, MYSQL conn, char serial[100], char content[100], char currentTime[100]) {
	char	query[255];
	int       query_stat;

	sprintf(query, "insert into board values "
		"('%s', '%s', '%s')",
		serial, content, currentTime);

	query_stat = mysql_query(connection, query);

	if (query_stat != 0)
	{
		fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
		return FALSE;
	}
}

void replaceChar(char serial[100]) {
	int i, j;

	char str1[100] = { 0 };

	strcpy(str1, serial);

	char str2[100];

		for (i = 0; i < strlen(str1); i++) { // 역슬래시 탐색
			if (str1[i] == '\\') {
				for (j = 0; j <= i; j++) {
					str2[j] = str1[j];
				}
				serial[i + 1] = '\\';
				i++;
			}
		}
}