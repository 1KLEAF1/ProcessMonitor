#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>

int main() {
	while (true){
		system("cls");
		std::cout << "==== Process List, every 5 second update ===="<<'\n' << std::endl;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hSnapshot == INVALID_HANDLE_VALUE) {
			std::cerr << "Error: snapshot not created";
			return 1;
		}
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hSnapshot, &pe32)) {
			std::cerr << "Error:firts process not get";
			CloseHandle(hSnapshot);
			return 1;
		}
		do {
			std::wcout << pe32.th32ProcessID << L"\t| " << pe32.szExeFile << std::endl;
		} while (Process32Next(hSnapshot, &pe32));
		CloseHandle(hSnapshot);
		Sleep(5000);
	}
	return 0;
}