#include <stdio.h>  
#include <windows.h>  
#include <winreg.h>  

int main(int argc, char **argv)
{
	LONG    ret;

	HKEY    hKey;

	char    data_buffer[256];
	DWORD   data_type;
	DWORD   data_size;

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,                                // 키 값  
		"SYSTEM\\ControlSet001\\Enum\\USB\\ROOT_HUB30\\4&119983ef&0&0",    // 서브 키값   
		0,                                                  // 옵션 항상 0  
		KEY_QUERY_VALUE,                                    // 접근 권한  
		&hKey                                               // 키 핸들  
	);

	if (ret == ERROR_SUCCESS)
	{
		memset(data_buffer, 0, sizeof(data_buffer));
		data_size = sizeof(data_buffer);
		RegQueryValueEx(hKey,                // RegOpenKeyEx에 얻어진 키 핸들  
			"ParentidPrefix",                    // 키 안에 값 이름   
			0,                                    // 옵션 항상 0   
			&data_type,                           // 얻어진 데이터 타입  
			(BYTE*)data_buffer,                  // 얻어진 데이터  
			(DWORD *)&data_size                  // 얻어진 데이터 크기   
		);
		RegCloseKey(hKey);
	}

	printf("READ  REGISTER\n");
	printf("KEY   [HKEY_LOCAL_MACHINE"
		"\\SYSTEM\\ControlSet001\\Enum\\USB ] \n");
	printf("VALUE [cygdrive prefix] =  [%s]\n", data_buffer);

	while (1);
}


// 출처: http://bmfrog.tistory.com/entry/레지스트리-키값을-읽는-예제 [일타삼피]