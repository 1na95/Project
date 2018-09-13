#include <windows.h>
#include <fileapi.h>
#include <WinUser.h>
#include <Winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>
#include <mysql.h>
#include <string.h>
#include <time.h>
#include <direct.h>

#pragma warning(disable:4996)
#pragma comment(lib, "libmysql.lib")

#define DB_HOST "127.0.0.1"
#define DB_USER "guest"
#define DB_PASS "1234"
#define DB_NAME "USBSecure"

int main() {

	// 현재 드라이브명
	char drive = char(_getdrive()+'A'-1);
	char path[10];
	char t1[] = ":\\";
	sprintf_s(path, "%c%s", drive, t1);

	// DB 연결
	MYSQL		*connection = NULL, conn;
	MYSQL_RES	*sql_result;
	MYSQL_ROW	sql_row;
	int			query_stat;

	char name[12];
	char address[80];
	char tel[12];
	char query[255];

	// 사용자 정보
	char id[20] = "1234";

	BOOL isDetected = FALSE;
	char dMsg[30];
	time_t now;
		
	mysql_init(&conn);

	connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
	
	if (connection == NULL) {
		return 1;
	}

	// 디렉토리 변경 사항 알림
	HANDLE hDir = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0,
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
				strcpy_s(dMsg, "FILE_ACTION_ADDED");
				wprintf(L"FILE_ACTION_ADDED\n");
				break;
			case FILE_ACTION_REMOVED:
				isDetected = TRUE;
				strcpy_s(dMsg, "FILE_ACTION_REMOVED");
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
					sprintf_s(query, "insert into board values ('%s', '%s', '%s')",
						id, ctime(&now), dMsg);
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