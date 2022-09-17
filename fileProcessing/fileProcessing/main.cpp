#include <stdio.h>
#include <locale.h>
#include <conio.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

int main(int argc, char* argv[]) {

	setlocale(LC_ALL, "rus");
	system("cls");

	std::string buffer;
	std::vector<int> numberOfString;
	LPVOID buf[1];
	DWORD lpNumberOfBytesRead;
	std::string stringToWrite;
	std::vector<int> indexOfBreakLine;

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
	std::string position = argv[indexFirstFile + 2];

	std::cout << "First file: " << argv[indexFirstFile] << std::endl;
	std::cout << "Second file: " << argv[indexFirstFile + 1] << std::endl;
	std::cout << "Position: " << position << std::endl;

	for (int i = 0; i < stringCount; ++i) {
		numberOfString.push_back(atoi(argv[i + 2]));
	}

	std::sort(numberOfString.begin(), numberOfString.end());
	for (int i = 0; i < stringCount; ++i) {
		std::cout << "Number of string: " << numberOfString[i] << std::endl;
	}

	HANDLE h1, h2;
	h1 = CreateFileW(convertCharArrayToLPCWSTR(argv[indexFirstFile]), GENERIC_READ,
		FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);
	h2 = CreateFileW(convertCharArrayToLPCWSTR(argv[indexFirstFile + 1]), GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL,
		OPEN_ALWAYS, 0, NULL);
	
	do {
		ReadFile(h1, buf, 1, &lpNumberOfBytesRead, NULL);
		if (lpNumberOfBytesRead == 0) break;
		buffer += char(buf[0]);

	} while (lpNumberOfBytesRead != 0);
	
	indexOfBreakLine.push_back(0);
	for (int i = 0; i < buffer.size(); ++i) {
		if (buffer[i] == '\n') indexOfBreakLine.push_back(i);
	}

	for (auto i : numberOfString) {
		for (int j = 1; j < indexOfBreakLine.size(); j++) {
			if (j == i) {
				stringToWrite += buffer.substr(indexOfBreakLine[j-1], indexOfBreakLine[j] - indexOfBreakLine[j - 1]);
			}
		}
	}

	if (indexOfBreakLine.size() == numberOfString[numberOfString.size() - 1]) {
		stringToWrite += buffer.substr(indexOfBreakLine[indexOfBreakLine.size() - 1]);
	}
	stringToWrite += '\n';

	if (position == "begin") {
		SetFilePointer(h2, 0, NULL, FILE_BEGIN);
	}
	if (position == "end") {
		SetFilePointer(h2, 0, NULL, FILE_END);
	}
	if (position == "current") {
		SetFilePointer(h2, 0, NULL, FILE_CURRENT);
	}

	WriteFile(h2, stringToWrite.c_str(), stringToWrite.size(), 0, NULL);
	SetFileAttributesA(argv[indexFirstFile + 1], FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
	
	_getch();
	return 0;
}