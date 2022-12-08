#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
using namespace std;
#define QUEUE_SIZE 9
#define CLIENT_COUNT 9
#define SOURCE_BUFF 256
#define SEARCH_TEXT_BUFF 256

struct serverData {
	char text[SOURCE_BUFF];
	char searchText[SEARCH_TEXT_BUFF];
	int res = 0;
};

int compare_function(serverData* serverDataPointer)
{
	char* buff = new char[SOURCE_BUFF];
	char* new_str = serverDataPointer->searchText;
	int length = strlen(new_str);

	for (int i = 0; i < length; i++) {
		cout << new_str[i];
		buff[i] = new_str[length - i - 1];
	}

	buff[length] = '\n';
	cout << endl;

	for (int i = 0; i < length; i++) {
		cout << buff[i];
	}
	
	cout << endl;

	delete[] buff;

	return 0;
}

int main()
{
	setlocale(LC_ALL, "ru");
	struct serverData* serverDataPointer;

	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(serverData),
		L"mappedMemory"
	);
	if (hMapFile == NULL)
	{
		cout << "������ ��� �������� ������� ����������� ����� " << GetLastError() << endl;
		return 1;
	}

	serverDataPointer = (serverData*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS, 0,
		0,
		sizeof(serverData)
	);
	if (serverDataPointer == NULL)
	{
		cout << "������ ��� ����������� ������� ����� �� �������� ������������" << GetLastError() << endl;
		return 1;
	}

	HANDLE hSemaphore = CreateSemaphore(
		NULL,
		QUEUE_SIZE, QUEUE_SIZE,
		L"QueueSemaphore"
	);

	HANDLE hMutexAccessToServerProcessing = CreateMutex(
		NULL,
		FALSE,
		L"AccessToServerProcessing"
	);

	HANDLE hEventOpenMutexToProcessing = CreateEventW(
		NULL,
		TRUE,
		FALSE,
		L"eventOpenMutexToProcessing"
	);

	HANDLE hMutexAccessToWrite = CreateMutex(
		NULL,
		FALSE,
		L"AccessToWriteMutex"
	);

	HANDLE hMutexAccessToRead = CreateMutex(
		NULL,
		TRUE,
		L"AccessToReadFromClient"
	);

	HANDLE hMutexServerReady = CreateMutexW(
		NULL,
		TRUE,
		L"ServerReadyMutex"
	);

	HANDLE hEventReadToServer = CreateEventW(
		NULL,
		TRUE,
		FALSE,
		L"ReadToServerEvent"
	);


	PROCESS_INFORMATION procInfo[CLIENT_COUNT];
	STARTUPINFO startInfo;
	ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&startInfo, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);
	bool bStatus = false;
	for (int i = 0; i < CLIENT_COUNT; i++)
	{
		bStatus = CreateProcess(
			L"D:\\Release\\clientServer.exe",
			NULL,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&startInfo,
			&procInfo[i]
		);
		if (bStatus == NULL)
		{
			cout << "������� #" << (i + 1) << " �� ��� ������" << endl;
		}
	}

	for (int i = 0; i < QUEUE_SIZE; i++)
	{
		ReleaseMutex(hMutexServerReady); //���������� �������
		WaitForSingleObject(hEventOpenMutexToProcessing, INFINITE); //�������� ���� ������ �������� ������� �� ��������� ��������
		WaitForSingleObject(hMutexAccessToServerProcessing, INFINITE); //�������� �������� �� ��������� ������
		WaitForSingleObject(hMutexServerReady, INFINITE); //������ �������� �� ���������� �������
		int x = compare_function(serverDataPointer); //��������� ������
		if (x != 0) {
			ReleaseMutex(hMutexAccessToServerProcessing); //����������� ������� �� ���������
			ReleaseMutex(hMutexAccessToRead); //����������� ������� �� ������ ��������
			break;
		}
		ReleaseMutex(hMutexAccessToServerProcessing); //����������� ������� �� ���������
		ReleaseMutex(hMutexAccessToRead); //����������� ������� �� ������ ��������
		WaitForSingleObject(hEventReadToServer, INFINITE); //�������� ������ ��������� ������ ��������
		ResetEvent(hEventReadToServer); //����� ������ �� ������ ��������
		WaitForSingleObject(hMutexAccessToRead, INFINITE); //������ �������� �� ������ � ������
		cout << "������� #" << (i + 1) << " ���������" << endl;
	}

	for (int i = 0; i < CLIENT_COUNT; i++)
	{
		WaitForSingleObject(procInfo[i].hProcess, INFINITE);
		CloseHandle(procInfo[i].hProcess);
		CloseHandle(procInfo[i].hThread);
	}

	UnmapViewOfFile(serverDataPointer);
	CloseHandle(hMutexAccessToServerProcessing);
	CloseHandle(hEventReadToServer);
	CloseHandle(hMutexServerReady);
	CloseHandle(hEventOpenMutexToProcessing);
	CloseHandle(hSemaphore);
	CloseHandle(hMutexAccessToRead);
	CloseHandle(hMutexAccessToWrite);
	CloseHandle(hMapFile);
	return 0;

}
