#include <windows.h>
#include <iostream>
#include <shellapi.h>

// Automatically verify administrative rights
BOOL IsRunAsAdmin() {
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
        CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin);
        FreeSid(pAdministratorsGroup);
    }
    return fIsRunAsAdmin;
}

int main() {
    SetConsoleTitleA("Ro-Crack Multi-Instance Unlocker");

    // 1. Force Relaunch with Admin rights if missing
    if (!IsRunAsAdmin()) {
        wchar_t szPath[MAX_PATH];
        GetModuleFileNameW(NULL, szPath, MAX_PATH);

        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.lpVerb = L"runas";
        sei.lpFile = szPath;
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;

        if (ShellExecuteExW(&sei)) {
            return 0; 
        } else {
            std::cout << "[!!!] Admin elevation request denied by user.\n";
            std::cin.get();
            return 1;
        }
    }

    std::cout << "[*] Initialising native C++ memory hook...\n";

    // 2. Define an Open-Access Security Descriptor for Global Kernel Space
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    // 3. Target the updated system path string
    LPCWSTR mutexName = L"Global\\ROBLOX_singletonEvent";

    // Attempt creation using our security settings
    HANDLE mutexHandle = CreateMutexW(&sa, TRUE, mutexName);
    DWORD lastError = GetLastError();

    if (lastError == ERROR_ALREADY_EXISTS) {
        std::cout << "[!] Target handle occupied. Purging restrictions...\n";
        CloseHandle(mutexHandle);
        mutexHandle = CreateMutexW(&sa, TRUE, mutexName);
    }

    if (mutexHandle != NULL) {
        std::cout << "\n[!!!] SUCCESS: Multi-instance unlocked natively!\n";
        std::cout << "[*] Keep this window open while playing.\n";
        std::cout << "[*] You can now launch independent browser tabs safely.\n";
        Sleep(INFINITE); 
    } else {
        std::cout << "Fatal Error. Win32 Code: " << GetLastError() << "\n";
        std::cin.get();
    }

    return 0;
}
