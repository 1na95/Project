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

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,                                // Ű ��  
		"SYSTEM\\ControlSet001\\Enum\\USB\\ROOT_HUB30\\4&119983ef&0&0",    // ���� Ű��   
		0,                                                  // �ɼ� �׻� 0  
		KEY_QUERY_VALUE,                                    // ���� ����  
		&hKey                                               // Ű �ڵ�  
	);

	if (ret == ERROR_SUCCESS)
	{
		memset(data_buffer, 0, sizeof(data_buffer));
		data_size = sizeof(data_buffer);
		RegQueryValueEx(hKey,                // RegOpenKeyEx�� ����� Ű �ڵ�  
			"ParentidPrefix",                    // Ű �ȿ� �� �̸�   
			0,                                    // �ɼ� �׻� 0   
			&data_type,                           // ����� ������ Ÿ��  
			(BYTE*)data_buffer,                  // ����� ������  
			(DWORD *)&data_size                  // ����� ������ ũ��   
		);
		RegCloseKey(hKey);
	}

	printf("READ  REGISTER\n");
	printf("KEY   [HKEY_LOCAL_MACHINE"
		"\\SYSTEM\\ControlSet001\\Enum\\USB ] \n");
	printf("VALUE [cygdrive prefix] =  [%s]\n", data_buffer);

	while (1);
}


// ��ó: http://bmfrog.tistory.com/entry/������Ʈ��-Ű����-�д�-���� [��Ÿ����]