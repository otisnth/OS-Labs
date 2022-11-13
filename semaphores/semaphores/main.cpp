#include<Windows.h>
#include<iostream>
#include<ctime>
#include <tchar.h>
#include<locale>
using namespace std;
#define DATA_SIZE_SERVER 22
#define DATA_SIZE_CLIENT 21
#define SIZE_BUFFER 50
struct RingBuffer {
	int* data;
	int indexFirstEmpty;
	int indexFirstFilled;
};
HANDLE hFilled, hEmpty, hDataAccess;

DWORD WINAPI creatorThread(LPVOID data) {
	srand(time(NULL));
	RingBuffer* buffer = (RingBuffer*)data;
	for (int i = 0; i < 5; i++) {
		WaitForSingleObject(hEmpty, INFINITE);
		WaitForSingleObject(hDataAccess, INFINITE);
		for (int j = 0; j <= DATA_SIZE_SERVER; j++) {
			buffer->data[buffer->indexFirstEmpty] = (1 + rand() % 9);
			cout << "**Server thread(creator)**" << "[ " << j << " ]:" << "\t";
			cout << " [" << buffer->indexFirstEmpty << "]=" << buffer -> data[buffer->indexFirstEmpty] << endl;
			if (buffer->indexFirstEmpty >= SIZE_BUFFER) buffer -> indexFirstEmpty = 0;
			else buffer->indexFirstEmpty++;
		}
		cout << endl << "Value: ";
		for (int k = 0; k < SIZE_BUFFER; k++) {
			cout << buffer->data[k] << " ";
		}
		cout << endl << endl;
		ReleaseSemaphore(hDataAccess, 1, NULL);
		ReleaseSemaphore(hFilled, 1, NULL);
	}
	return 0;
}

DWORD WINAPI consumerThread(LPVOID data) {
	RingBuffer* buffer = (RingBuffer*)data;
	for (int i = 0; i < 5; i++) {
		WaitForSingleObject(hFilled, INFINITE);
		WaitForSingleObject(hDataAccess, INFINITE);
		for (int j = 0; j <= DATA_SIZE_CLIENT; j++) {
			cout << "Client thread(consumer)" << "[ " << j << " ]:" << '\t';
			cout << " [" << buffer->indexFirstFilled << "]=" << buffer -> data[buffer->indexFirstFilled] << endl;
			buffer->data[buffer->indexFirstFilled] = 0;
			if (buffer->indexFirstFilled >= SIZE_BUFFER) buffer -> indexFirstFilled = 0;
			else buffer->indexFirstFilled++;
		}
		cout << endl << "Value: ";
		for (int k = 0; k < SIZE_BUFFER; k++) {
			cout << buffer->data[k] << " ";
		}
		cout << endl << endl;
		ReleaseSemaphore(hDataAccess, 1, NULL);
		ReleaseSemaphore(hEmpty, 1, NULL);
	}
	return 0;
}

void main() {
	srand(time(NULL));
	HANDLE hThreads[2];
	DWORD threadId;
	RingBuffer ringBuffer;
	if ((hDataAccess = CreateSemaphore(NULL, 1, 1, NULL)) == NULL ||
		(hFilled = CreateSemaphore(NULL, 0, SIZE_BUFFER, NULL)) == NULL ||
		(hEmpty = CreateSemaphore(NULL, SIZE_BUFFER, SIZE_BUFFER, NULL)) == NULL)
	{
		printf("CreateSemaphores error: %d\n", GetLastError());
		return;
	}
	ringBuffer.indexFirstEmpty = 0;
	ringBuffer.indexFirstFilled = 0;
	ringBuffer.data = new int[SIZE_BUFFER];
	for (int i = 0; i < SIZE_BUFFER; i++) {
		ringBuffer.data[i] = 0;
	}
	hThreads[0] = CreateThread(0, 0, creatorThread, &ringBuffer, 0, &threadId);
	hThreads[1] = CreateThread(0, 0, consumerThread, &ringBuffer, 0, &threadId);
	WaitForMultipleObjects(2, hThreads, true, INFINITE);
	for (int i = 0; i < 2; i++) {
		CloseHandle(hThreads[i]);
	}
	CloseHandle(hDataAccess);
	CloseHandle(hFilled);
	CloseHandle(hEmpty);
	system("pause");
}