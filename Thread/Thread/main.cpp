#include <iostream>
#include "Windows.h"
#include <time.h>

using namespace std;
#define MATRIX_SIZE 4

struct matrixRow {
	int value[MATRIX_SIZE];
	int rowResult;
	int seed;
};

struct matrix {
	matrixRow row[MATRIX_SIZE];
};

DWORD WINAPI generateAndCalculate(void* data) {
	matrixRow* row = (matrixRow*)data;
	srand(time(NULL));
	srand(row->seed);
	row->rowResult = 1;
	for (int i = 0; i < MATRIX_SIZE; i++) {
		row->value[i] = rand() % 20 + 10;
		row->rowResult *= row->value[i];
	}
	return 0;
}

DWORD WINAPI printMatrix(void* data) {
	matrix* A = (matrix*)data;
	for (int i = 0; i < MATRIX_SIZE; i++) {
		for (int j = 0; j < MATRIX_SIZE; j++) {
			cout << A->row[i].value[j] << " ";
		}
		cout << "Произведение = " << A->row[i].rowResult << " " << endl;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	srand(time(NULL));
	matrix A;
	HANDLE hThreads[MATRIX_SIZE];
	HANDLE hThread;
	for (int i = 0; i < MATRIX_SIZE; i++) {
		A.row[i].seed = rand();
		hThreads[i] = CreateThread(NULL, 0, generateAndCalculate,
			&A.row[i], 0, NULL);
	}
	WaitForMultipleObjects(MATRIX_SIZE, hThreads, TRUE, INFINITE);
	hThread = CreateThread(NULL, 0, printMatrix, &A, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	return 0;
}