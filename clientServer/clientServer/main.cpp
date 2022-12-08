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
	cout << "Процесс #" << GetCurrentProcessId() << " запущен" << endl;
	struct serverData* serverDataPointer;

	HANDLE hSemaphore = OpenSemaphore(
		SYNCHRONIZE,
		FALSE,
		L"QueueSemaphore"
	);
	if (hSemaphore == NULL)
	{
		cout << "Семафор не найден" << endl;
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
		cout << "Мьютекс на запись не найден" << endl;
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
		cout << "Мьютекс на чтение клиентом не найден" << endl;
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
		cout << "Мьютекс на статус сервера не найден" << endl;
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
		cout << "Мьютекс на обработку сервером не найден" << endl;
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
		cout << "Очередь заполнена" << endl;
		ExitProcess(1);
	}

	cout << "Ожидание в очереди" << endl;

	WaitForSingleObject(hMutexAccessToWrite, INFINITE); //ждем доступ к вводу данных
	WaitForSingleObject(hMutexServerReady, INFINITE); //ждем подтвержение готовности сервера
	WaitForSingleObject(hMutexAccessToServerProcessing, INFINITE); //захватыем мьютекс на обработку данных
	SetEvent(hEventOpenMutexToProcessing); //вызываем ивент на подтвержение захвата на обработку

	char buff[SEARCH_TEXT_BUFF];

	cout << "Введите строку для инвертирования: ";
	try {
		cin.getline(buff, SOURCE_BUFF, '\n');
	}
	catch (...)
	{
		cout << "Ошибка при записи строки" << endl;
	}
	if (SetEvent(hEventOpenMutexToProcessing) == FALSE)
	{
		cout << "Ивент не был переведен в сигнальное положение" << endl;
	}
	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		L"mappedMemory"
	);
	if (hMapFile == NULL)
	{
		cout << "Ошибка при открытии объекта отображения файла " << GetLastError() << endl;
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
		cout << "Ошибка при отображении области файла на адресное пространство" << GetLastError() << endl;
		ExitProcess(1);
	}
	strcpy(serverDataPointer->searchText, "");
	strcpy(serverDataPointer->searchText, buff);
	cout << "Количество символов = " << strlen(buff) << endl;

	ResetEvent(hEventOpenMutexToProcessing); //сбрасываем ивент на на подтвержение захвата на обработку
	ReleaseMutex(hMutexServerReady); //освобождаем мьютекс на готовность сервера, чтобы сервер его занял
	ReleaseMutex(hMutexAccessToServerProcessing); //освобождаем мьютекс на обработку данных (начинаем обработку)
	WaitForSingleObject(hMutexAccessToRead, INFINITE); //ждем доступа на чтение

	SetEvent(hEventReadToServer); //вызываем ивент на захват мьютекса на чтение сервером
	ReleaseMutex(hMutexAccessToRead); //освобождаем мьютека на чтение для сервера
	ReleaseMutex(hMutexAccessToWrite); //передаем очередь другому процессу
	ReleaseSemaphore(hSemaphore, 1, NULL); //изменяем значение семафора

	getchar();
	getchar();
	ExitProcess(0);
}
