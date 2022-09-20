#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
using namespace std;

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");

	HANDLE waitList[2];

	SECURITY_ATTRIBUTES securityAttributes;
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;

	LPCWSTR childProcess = convertCharArrayToLPCWSTR(argv[1]);

	HANDLE hOutput1 = CreateFile(L"output1.txt",
		GENERIC_WRITE, FILE_SHARE_WRITE,
		&securityAttributes, OPEN_ALWAYS,
		0, NULL);

	HANDLE hOutput2 = CreateFile(L"output2.txt",
		GENERIC_WRITE, FILE_SHARE_WRITE,
		&securityAttributes, OPEN_ALWAYS,
		0, NULL);

	HANDLE hInput2 = CreateFile(L"input2.txt",
		GENERIC_READ, FILE_SHARE_READ,
		&securityAttributes, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE hInput3 = CreateFile(L"input3.txt",
		GENERIC_READ, FILE_SHARE_READ,
		&securityAttributes, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	SetFilePointer(hOutput1, 0, NULL, FILE_END);
	SetFilePointer(hOutput2, 0, NULL, FILE_END);

	STARTUPINFO startup1 = { sizeof(STARTUPINFO) };
	startup1.dwFlags = STARTF_USESTDHANDLES;
	startup1.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startup1.hStdOutput = hOutput1;
	PROCESS_INFORMATION process1;

	STARTUPINFO startup2 = { sizeof(STARTUPINFO) };
	startup2.dwFlags = STARTF_USESTDHANDLES;
	startup2.hStdInput = hInput2;
	startup2.hStdOutput = hOutput2;
	PROCESS_INFORMATION process2;

	STARTUPINFO startup3 = { sizeof(STARTUPINFO) };
	startup3.dwFlags = STARTF_USESTDHANDLES;
	startup3.hStdInput = hInput3;
	startup3.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	PROCESS_INFORMATION process3;

	STARTUPINFO startup4 = { sizeof(STARTUPINFO) };
	startup4.dwFlags = STARTF_USESTDHANDLES;
	startup4.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startup4.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	PROCESS_INFORMATION process4;

	CreateProcess(childProcess, NULL, &securityAttributes, NULL, TRUE, 0, 0, 0, &startup1, &process1);
	WaitForSingleObject(process1.hProcess, INFINITE);
	CreateProcess(childProcess, NULL, &securityAttributes, NULL, TRUE, 0, 0, 0, &startup2, &process2);

	waitList[0] = process1.hProcess;
	waitList[1] = process2.hProcess;
	WaitForMultipleObjects(2, waitList, FALSE, INFINITE);

	CreateProcess(childProcess, NULL, &securityAttributes, NULL, TRUE, 0, 0, 0, &startup3, &process3);
	WaitForSingleObject(process3.hProcess, INFINITE);
	CreateProcess(childProcess, NULL, &securityAttributes, NULL, TRUE, 0, 0, 0, &startup4, &process4);
	WaitForSingleObject(process4.hProcess, INFINITE);

	CloseHandle(hOutput1);
	CloseHandle(hOutput2);
	CloseHandle(hInput2);
	CloseHandle(hInput3);
	CloseHandle(process1.hThread);
	CloseHandle(process2.hThread);
	CloseHandle(process3.hThread);
	CloseHandle(process4.hThread);
	system("pause");
	return 0;
}

