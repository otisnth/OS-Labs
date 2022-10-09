#include <iostream>
#include "Windows.h"
#include <time.h>

using namespace std;
#define READERS 4
#define WRITERS 1
#define COUNT_GENERATE 4
#define BUFFER_LENGTH 21

int buffer[BUFFER_LENGTH] = { 0 };
int readers = 0;
HANDLE mutexStorage;
HANDLE mutexFirstReader;
HANDLE mutexReaderQueue;

DWORD WINAPI writing(void* data) {
	srand(GetCurrentThreadId());
	int randomData[COUNT_GENERATE];
	for (int i = 0; i < COUNT_GENERATE; i++) {
		randomData[i] = rand() % 90 + 10;
	}
	WaitForSingleObject(mutexStorage, INFINITE);
	for (int i = 0; i < COUNT_GENERATE; i++) {
		((int*)data)[rand() % BUFFER_LENGTH] = randomData[i];
	}
	ReleaseMutex(mutexStorage);
	return 0;
}

DWORD WINAPI reading(void* data) {
	WaitForSingleObject(mutexFirstReader, INFINITE);
	readers++;
	if (readers == 1) {
		WaitForSingleObject(mutexStorage, INFINITE);
	}
	ReleaseMutex(mutexFirstReader);
	WaitForSingleObject(mutexReaderQueue, INFINITE);
	cout << "Поток Reader (id - " << GetCurrentThreadId() << ") : ";
	for (int i = 0; i < BUFFER_LENGTH; i++) {
		cout << ((int*)data)[i] << " ";
	}
	cout << endl;
	ReleaseMutex(mutexReaderQueue);
	WaitForSingleObject(mutexFirstReader, INFINITE);
	readers--;
	if (readers == 0) {
		ReleaseMutex(mutexStorage);
	}
	ReleaseMutex(mutexFirstReader);
	return 0;
}

int main(int argc, char* argv[])
{
	HANDLE threadsWriters[WRITERS];
	HANDLE threadsReaders[READERS];
	HANDLE threadsAll[WRITERS + READERS];
	mutexStorage = CreateMutex(NULL, FALSE, NULL);
	mutexFirstReader = CreateMutex(NULL, FALSE, NULL);
	mutexReaderQueue = CreateMutex(NULL, FALSE, NULL);
	int choose = 1;
	while (choose != 0) {
		for (int i = 0; i < WRITERS; i++) {
			threadsWriters[i] = CreateThread(NULL, 0, writing, &buffer, 0, NULL);
			threadsAll[i] = threadsWriters[i];
		}
		for (int i = 0; i < READERS; i++) {
			threadsReaders[i] = CreateThread(NULL, 0, reading, &buffer, 0, NULL);
			threadsAll[WRITERS + i] = threadsReaders[i];
		}
		WaitForMultipleObjects(WRITERS + READERS, threadsAll, TRUE, INFINITE);
		cout << "Введите 0 для выхода или любое значение для продолжения: ";
		cin >> choose;
	}
	CloseHandle(mutexStorage);
	CloseHandle(mutexFirstReader);
	CloseHandle(mutexReaderQueue);
	return 0;
}