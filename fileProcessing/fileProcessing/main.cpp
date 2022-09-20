#include <stdio.h>
#include <locale.h>
#include <conio.h>
#include <windows.h>
#include <iostream>

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

int main(int argc, char* argv[]) {

	setlocale(LC_ALL, "rus");
	system("cls");

	char buffer;
	DWORD lpNumberOfBytesRead;

	if (argc == 1) {
		std::cout << "Нет аргументов!" << std::endl;
		return 0;
	}
	int stringCount = atoi(argv[1]);

	if (argc < 5 + stringCount) {
		std::cout << "Недостаточно аргументов!" << std::endl;
		return 0;
	}
	
	int indexFirstFile = 2 + stringCount;
	int position = atoi(argv[indexFirstFile + 2]);

	std::cout << "First file: " << argv[indexFirstFile] << std::endl;
	std::cout << "Second file: " << argv[indexFirstFile + 1] << std::endl;
	std::cout << "Position: " << position << std::endl;

	HANDLE h1 = CreateFile(convertCharArrayToLPCWSTR(argv[indexFirstFile]), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE h2 = CreateFile(convertCharArrayToLPCWSTR(argv[indexFirstFile + 1]), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE hTemp = CreateFile(L"temp.txt", GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	int stringCounter = 0;
	
	while (stringCounter != atoi(argv[argc - 1]) - 1) {
		if (ReadFile(h2, &buffer, sizeof(buffer), &lpNumberOfBytesRead, NULL) && lpNumberOfBytesRead == 0) {
			break;
		}
		if (buffer == '\n') {
			stringCounter++;
		}
	}
	LARGE_INTEGER filePos;
	SetFilePointerEx(h2, { { 0,0 } }, &filePos, FILE_CURRENT);

	while (ReadFile(h2, &buffer, sizeof(buffer), &lpNumberOfBytesRead, NULL) && lpNumberOfBytesRead != 0) {
			WriteFile(hTemp, &buffer, sizeof(buffer), &lpNumberOfBytesRead, NULL);
	}
	SetFilePointerEx(h2, filePos, NULL, FILE_BEGIN);
	SetFilePointerEx(hTemp, { { 0,0 } }, NULL, FILE_BEGIN);

	stringCounter = 0;
	int i = 0;
	while (i != stringCount) {
		if (ReadFile(h1, &buffer, sizeof(buffer), &lpNumberOfBytesRead, NULL) && lpNumberOfBytesRead == 0) {
			break;
		}
		if (stringCounter == atoi(argv[2 + i]) - 1) {
			WriteFile(h2, &buffer, sizeof(buffer), &lpNumberOfBytesRead, NULL);
			if (buffer == '\n') {
				stringCounter++;
				i++;
			}
		}
		else {
			if (buffer == '\n') {
				stringCounter++;
			}
		}
	}

	while (ReadFile(hTemp, &buffer, sizeof(buffer), &lpNumberOfBytesRead, NULL) &&
		lpNumberOfBytesRead != 0) {
		WriteFile(h2, &buffer, sizeof(buffer), &lpNumberOfBytesRead, NULL);
	}

	SetFileAttributesA(argv[indexFirstFile + 1], FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);

	CloseHandle(h1);
	CloseHandle(h2);
	CloseHandle(hTemp);
	DeleteFile(L"temp.txt");

	system("pause");
	return 0;
}