#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
using namespace std;
#define SOURCE_BUFF 256
#define SEARCH_TEXT_BUFF 64

struct serverData {
	char text[SOURCE_BUFF];
	char searchText[SEARCH_TEXT_BUFF]; int res = 0;
};


int main()
{
	setlocale(LC_ALL, "ru");
	cout << "������� #" << GetCurrentProcessId() << " �������" << endl;
	struct serverData* serverDataPointer;

	HANDLE hSemaphore = OpenSemaphore(
		SYNCHRONIZE,
		FALSE,
		L"QueueSemaphore"
	);
	if (hSemaphore == NULL)
	{
		cout << "������� �� ������" << endl;
		getchar();
		getchar();
		ExitProcess(1);
	}
	HANDLE hMutexAccessToWrite = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		L"AccessToWriteMutex"
	);
	if (hMutexAccessToWrite == NULL)
	{
		cout << "������� �� ������ �� ������" << endl;
		getchar();
		getchar();
		ExitProcess(1);
	}
	HANDLE hMutexAccessToRead = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		L"AccessToReadFromClient"
	);
	if (hMutexAccessToRead == NULL)
	{
		cout << "������� �� ������ �������� �� ������" << endl;
		getchar();
		getchar();
		ExitProcess(1);
	}
	HANDLE hMutexServerReady = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		L"ServerReadyMutex"
	);
	if (hMutexServerReady == NULL)
	{
		cout << "������� �� ������ ������� �� ������" << endl;
		getchar();
		getchar();
		ExitProcess(1);
	}
	HANDLE hMutexAccessToServerProcessing = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		L"AccessToServerProcessing"
	);
	if (hMutexAccessToServerProcessing == NULL)
	{
		cout << "������� �� ��������� �������� �� ������" << endl;
		getchar();
		getchar();
	}
	HANDLE hEventOpenMutexToProcessing = OpenEventW(
		EVENT_ALL_ACCESS,
		FALSE,
		L"eventOpenMutexToProcessing"
	);
	HANDLE hEventReadToServer = OpenEventW(
		EVENT_ALL_ACCESS,
		FALSE,
		L"ReadToServerEvent"
	);
	if (WaitForSingleObject(hSemaphore, 1) == WAIT_TIMEOUT)
	{
		cout << "������� ���������" << endl;
		ExitProcess(1);
	}

	cout << "�������� � �������" << endl;

	WaitForSingleObject(hMutexAccessToWrite, INFINITE); //���� ������ � ����� ������
	WaitForSingleObject(hMutexServerReady, INFINITE); //���� ������������ ���������� �������
	WaitForSingleObject(hMutexAccessToServerProcessing, INFINITE); //��������� ������� �� ��������� ������
	SetEvent(hEventOpenMutexToProcessing); //�������� ����� �� ������������ ������� �� ���������

	char buff[SEARCH_TEXT_BUFF];

	cout << "������� ������ ��� ��������������: ";
	try {
		cin.getline(buff, SOURCE_BUFF, '\n');
	}
	catch (...)
	{
		cout << "������ ��� ������ ������" << endl;
	}
	if (SetEvent(hEventOpenMutexToProcessing) == FALSE)
	{
		cout << "����� �� ��� ��������� � ���������� ���������" << endl;
	}
	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		L"mappedMemory"
	);
	if (hMapFile == NULL)
	{
		cout << "������ ��� �������� ������� ����������� ����� " << GetLastError() << endl;
		ExitProcess(1);
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
		ExitProcess(1);
	}
	strcpy(serverDataPointer->searchText, "");
	strcpy(serverDataPointer->searchText, buff);
	cout << "���������� �������� = " << strlen(buff) << endl;

	ResetEvent(hEventOpenMutexToProcessing); //���������� ����� �� �� ������������ ������� �� ���������
	ReleaseMutex(hMutexServerReady); //����������� ������� �� ���������� �������, ����� ������ ��� �����
	ReleaseMutex(hMutexAccessToServerProcessing); //����������� ������� �� ��������� ������ (�������� ���������)
	WaitForSingleObject(hMutexAccessToRead, INFINITE); //���� ������� �� ������

	SetEvent(hEventReadToServer); //�������� ����� �� ������ �������� �� ������ ��������
	ReleaseMutex(hMutexAccessToRead); //����������� ������� �� ������ ��� �������
	ReleaseMutex(hMutexAccessToWrite); //�������� ������� ������� ��������
	ReleaseSemaphore(hSemaphore, 1, NULL); //�������� �������� ��������

	getchar();
	getchar();
	ExitProcess(0);
}
