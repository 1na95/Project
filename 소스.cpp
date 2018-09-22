
#include <tchar.h>
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

#pragma warning(disable:4996)
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shell32.lib")
#define _CRT_SECURE_NO_WARNINGS

void RefreshDirectory(LPTSTR);
void RefreshTree(LPTSTR);
void WatchDirectory(LPTSTR);

#pragma warning(disable:4996)

int main() {

	char folderName[255];
	char* pFolderName = folderName;
	getcwd(pFolderName, 256);
	printf("\n\t...Trying to get drive name...\n");
	printf("\t Drive Path : %s\n", (LPCSTR)folderName);
	const wchar_t *folderPath;

	int nChars = MultiByteToWideChar(CP_ACP, 0, pFolderName, -1, NULL, 0);
	folderPath = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, pFolderName, -1, (LPWSTR)folderPath, nChars);
	
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
			break;
		}

		pfni = (FILE_NOTIFY_INFORMATION*)pBuffer;

		do {
			printf("NextEntryOffset(%d)\n", pfni->NextEntryOffset);
			switch (pfni->Action)
			{
			case FILE_ACTION_ADDED:
				wprintf(L"FILE_ACTION_ADDED\n");
				break;
			case FILE_ACTION_REMOVED:
				wprintf(L"FILE_ACTION_REMOVED\n");
				break;
			case FILE_ACTION_MODIFIED:
				wprintf(L"FILE_ACTION_MODIFIED\n");
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				wprintf(L"FILE_ACTION_RENAMED_OLD_NAME\n");
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:
				wprintf(L"FILE_ACTION_RENAMED_NEW_NAME\n");
				break;
			default:
				break;
			}
			printf("FileNameLength(%d)\n", pfni->FileNameLength);

			StringCbCopyNW(temp, sizeof(temp), pfni->FileName, pfni->FileNameLength);

			wprintf(L"FileName(%s)\n", (LPCSTR)temp);
			// 시간함수
			time(&rawtime);
			printf("%s\n\n", ctime(&rawtime));

			time_t curr;
			struct tm *d;
			curr = time(NULL);
			d = localtime(&curr);
			char cTm;
			cTm = d->tm_year + 1900 + '0';
			printf("%c", cTm);
			// 시리얼번호와 함께 DB로 전송

			pfni = (FILE_NOTIFY_INFORMATION*)((PBYTE)pfni + pfni->NextEntryOffset);
		} while (pfni->NextEntryOffset > 0);
	}
	while (1);
}