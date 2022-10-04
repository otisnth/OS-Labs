#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

vector<string> splitString(string& str) {
	vector<string> substrings;
	int substringLength = 0;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == ' ') {
			substrings.push_back(str.substr(i - substringLength, substringLength));
			substringLength = 0;
		}
		else
		{
			substringLength++;
		}
	}
	substrings.push_back(str.substr(str.size() - substringLength, substringLength));
	return substrings;
}

void executeInner(const vector<string>& args) {
	char buffer;
	DWORD lpNumberOfBytesRead;
	if (args.size() == 1) {
		std::cout << "Нет аргументов!" << std::endl;
		return;
	}
	int stringCount = stoi(args[1]);
	if (args.size() < 5 + stringCount) {
		std::cout << "Недостаточно аргументов!" << std::endl;
		return;
	}
	int indexFirstFile = 2 + stringCount;
	int position = stoi(args[indexFirstFile + 2]);
	std::cout << "First file: " << args[indexFirstFile] << std::endl;
	std::cout << "Second file: " << args[indexFirstFile + 1] << std::endl;
	std::cout << "Position: " << position << std::endl;
	HANDLE h1 = CreateFile(convertCharArrayToLPCWSTR(args[indexFirstFile].c_str()), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE h2 = CreateFile(convertCharArrayToLPCWSTR(args[indexFirstFile + 1].c_str()), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hTemp = CreateFile(L"temp.txt", GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	int stringCounter = 0;
	while (stringCounter != stoi(args[args.size() - 1]) - 1) {
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
		if (stringCounter == stoi(args[2 + i]) - 1) {
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
	SetFileAttributesA(args[indexFirstFile + 1].c_str(), FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
	CloseHandle(h1);
	CloseHandle(h2);
	CloseHandle(hTemp);
	DeleteFile(L"temp.txt");
}

bool isOpened(HANDLE h) {
	return h != NULL && h != INVALID_HANDLE_VALUE;
}

HANDLE openReadFile(const char* path, SECURITY_ATTRIBUTES secAttr) {
	return CreateFileW(convertCharArrayToLPCWSTR(path), GENERIC_READ, NULL, &secAttr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
}

HANDLE openWriteFile(const char* path, SECURITY_ATTRIBUTES secAttr) {
	return CreateFileW(convertCharArrayToLPCWSTR(path), GENERIC_WRITE, NULL, &secAttr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
}

void executeOuter(const vector<string>& args) {

	SECURITY_ATTRIBUTES secAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE
	};
	HANDLE inputFile = NULL;
	HANDLE outputFile = NULL;
	if (args.size() == 3 && args[1][0] == '>') {
		if ((inputFile = openReadFile(args[0].c_str(), secAttr)) == INVALID_HANDLE_VALUE) {
			if ((outputFile = openWriteFile(args[2].c_str(), secAttr)) == INVALID_HANDLE_VALUE) {
				cout << "Не удалось открыть файл!" << endl;
				return;
			}
		}
	}
	else if (args.size() >= 3) {
		if (args[1][0] == '>') {
			if ((inputFile = openReadFile(args[0].c_str(), secAttr)) == INVALID_HANDLE_VALUE) {
				cout << "Не удалось открыть исходный файл!" << endl;
				return;
			}
		}
		if (args[args.size() - 2][0] == '>') {
			if ((outputFile = openWriteFile(args[args.size() - 1].c_str(), secAttr)) == INVALID_HANDLE_VALUE) {
				cout << "Не удалось открыть файл назначения!" << endl;
				if (isOpened(inputFile)) CloseHandle(inputFile);
				return;
			}
		}
	}

	STARTUPINFOW startupInfo = {};
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	int startIdxCommand = 0, endIdxCommand = args.size();
	if (isOpened(inputFile)) {
		startupInfo.hStdInput = inputFile;
		startIdxCommand = 2;
	}
	if (isOpened(outputFile)) {
		startupInfo.hStdOutput = outputFile;
		endIdxCommand = args.size() - 2;
	}
	string stringCommands = args[startIdxCommand];
	for (int i = startIdxCommand + 1; i < endIdxCommand; i++)
		stringCommands += ' ' + args[i];
	PROCESS_INFORMATION processInfo = {};
	ZeroMemory(&processInfo, sizeof(processInfo));
	if (!CreateProcessW(NULL, convertCharArrayToLPCWSTR(stringCommands.c_str()), &secAttr, NULL, TRUE,
		NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
		NULL, NULL, &startupInfo, &processInfo)) {
		cout << "Не удалось запустить процесс!" << endl;
	}
	WaitForSingleObject(processInfo.hProcess, INFINITE);
	if (isOpened(inputFile)) CloseHandle(inputFile);
	if (isOpened(outputFile)) CloseHandle(outputFile);
}



int main(int argc, CHAR* argv[]) {

	string command;

	while (true) {
		cout << ": ";
		getline(cin, command);
		if (!strcmp(command.c_str(), "quit"))
			break;
		if (command.empty()) continue;
		vector<string> args = splitString(command);
		const char* firstArg = args.at(0).c_str();
		try {
			if (!strcmp(firstArg, "inner")) executeInner(args);
			else executeOuter(args);
		}
		catch (exception e) {
			cout << "Ошибка: " << e.what() << endl;
		}
		cout << endl;
	}
	return 0;
}