#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>

std::string getCurrentTime() {
    time_t now = time(0);
    struct tm timeinfo;

    // Используем безопасную версию localtime_s
    localtime_s(&timeinfo, &now);

    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return buffer;
}

bool isSystemProcess(const std::wstring& name) {
    std::vector<std::wstring> systemProcs = {
        L"svchost.exe", L"explorer.exe", L"System", L"csrss.exe"
    };

    for (const auto& proc : systemProcs) {
        if (_wcsicmp(name.c_str(), proc.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

int main() {
    std::ofstream logFile("system_log.txt", std::ios::app);
    logFile << "=== System Activity Log started at " << getCurrentTime() << " ===\n";

    std::vector<DWORD> previousPIDs;

    // Получаем начальный список процессов
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (!isSystemProcess(pe32.szExeFile)) {
                    previousPIDs.push_back(pe32.th32ProcessID);
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }

    std::cout << "Monitoring started. Press Ctrl+C to stop.\n";

    while (true) {
        Sleep(3000); // Проверка каждые 3 секунды

        std::vector<DWORD> currentPIDs;

        // Получаем текущие процессы
        hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32W pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32W);

            if (Process32FirstW(hSnapshot, &pe32)) {
                do {
                    if (!isSystemProcess(pe32.szExeFile)) {
                        currentPIDs.push_back(pe32.th32ProcessID);

                        // Ищем новые процессы
                        auto it = std::find(previousPIDs.begin(), previousPIDs.end(), pe32.th32ProcessID);
                        if (it == previousPIDs.end()) {
                            // Новая программа!
                            std::wstring wideName = pe32.szExeFile;
                            std::string name(wideName.begin(), wideName.end());

                            std::cout << "[NEW] " << name << " at " << getCurrentTime() << "\n";
                            logFile << getCurrentTime() << " | PROCESS | " << name << " | PID: " << pe32.th32ProcessID << "\n";
                        }
                    }
                } while (Process32NextW(hSnapshot, &pe32));
            }
            CloseHandle(hSnapshot);
        }

        previousPIDs = currentPIDs;
    }

    logFile.close();
    return 0;
}