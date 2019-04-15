//===============================================================================================//
/*
	Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
	
	Reference: 
	https://stackoverflow.com/questions/865152/how-can-i-get-a-process-handle-by-its-name-in-c
	http://blog.opensecurityresearch.com/2013/01/windows-dll-injection-basics.html
	https://github.com/stephenfewer/ReflectiveDLLInjection
*/
//===============================================================================================//
#include "pch.h"
#include "windows.h"
#include "tlhelp32.h"
#include "LoadLibraryR.h"
#include <iostream>
#include <string>
using namespace std;

//===============================================================================================//
HANDLE getHandle(wstring process_name);
DWORD inject_DLL_Reflective(const char* file_name, HANDLE h_process);
DWORD inject_DLL_LoadLibrary(const char* file_name, HANDLE h_process);

//===============================================================================================//
/* MAIN FUNCTION */
int main()
{
	string option;
	HANDLE hProcess = NULL;

	// Name of the injecting DLL
	const char* file_name = "dll_code.dll";

	cout << endl;
	cout << "==============================================================" << endl;
	cout << "CSCE 451 FINAL PROJECT: SOLITAIRE ANALYSIS - DLL CODE INJECTOR" << endl;
	cout << "==============================================================" << endl;

	hProcess = getHandle(L"sol.exe");
	if (hProcess == NULL) return -1;

	cout << endl;
	cout << "[1]. Load Library Injection \n";
	cout << "[2]. Reflective Injection \n\n";
	cout << "Please choose an injection method or Q to quit: ";
	cin >> option;

	cout << endl;
	if (option == "1") inject_DLL_LoadLibrary(file_name, hProcess);
	else if (option == "2") inject_DLL_Reflective(file_name, hProcess);
	else if (option == "Q" or "q") return 0;
	else cout << "[!] ERROR: Unrecognized option." << endl;

	return 0;
}

//===============================================================================================//
/* GET PROCESS ID AND OPEN NEW PROCESS */
HANDLE getHandle(wstring process_name)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == true)
	{
		while (Process32Next(snapshot, &entry) == true)
		{
			if (process_name == entry.szExeFile)
			{
				cout << "Process found, PID = " << entry.th32ProcessID << endl;
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
				CloseHandle(snapshot);
				return hProcess;
			}
		}
	}
	cout << "Process not found." << endl;
	return NULL;
}
//===============================================================================================//
/* REFLECTIVE DLL INJECTION METHOD */
DWORD inject_DLL_Reflective(const char* file_name, HANDLE h_process) {
	HANDLE hFile = NULL;
	DWORD dwLength = 0;
	DWORD dwBytesRead = 0;
	DWORD dwReflectiveLoaderOffset = 0;
	LPVOID lpWriteBuff = NULL;
	LPVOID lpDllAddr = NULL;

	LPTHREAD_START_ROUTINE lpStartExecAddr = NULL;
	DWORD exit_code;

	char fullDLLPath[_MAX_PATH];
	GetFullPathNameA(file_name, _MAX_PATH, fullDLLPath, NULL);

	hFile = CreateFileA(fullDLLPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("\n[!] ERROR: Failed to open DLL file!\n");
		return -1;
	}

	dwLength = GetFileSize(hFile, NULL);
	if (dwLength == INVALID_FILE_SIZE || dwLength == 0) {
		printf("\n[!] ERROR: Invalid DLL file size!\n");
		return -1;
	}

	lpWriteBuff = HeapAlloc(GetProcessHeap(), 0, dwLength);
	if (!lpWriteBuff) {
		printf("\n[!] ERROR: Failed to allocate memory for DLL!\n");
		return -1;
	}

	if (ReadFile(hFile, lpWriteBuff, dwLength, &dwBytesRead, NULL) == FALSE) {
		printf("\n[!] ERROR: Failed to read DLL code to memory!\n");
		return -1;
	}

	lpDllAddr = VirtualAllocEx(h_process, NULL, dwLength, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	printf("[+] Writing into the current process space at 0x%08x\n", lpDllAddr);

	if (WriteProcessMemory(h_process, lpDllAddr, lpWriteBuff, dwLength, NULL) == 0) {
		printf("\n[!] WriteProcessMemory Failed [%u]\n", GetLastError());
		return -1;
	}

	dwReflectiveLoaderOffset = GetReflectiveLoaderOffset(lpWriteBuff);

	HeapFree(GetProcessHeap(), 0, lpWriteBuff);

	if (!dwReflectiveLoaderOffset) {
		printf("\n[!] Error calculating Offset - Wrong Architecture?\n");
		return -1;
	}

	lpStartExecAddr = (LPTHREAD_START_ROUTINE)((ULONG_PTR)lpDllAddr + dwReflectiveLoaderOffset);
	if (lpStartExecAddr == NULL) {
		printf("\n[!] ERROR: Could not allocate memory!!\n");
		return -1;
	}

	printf("\n[+] Using CreateRemoteThread() to Create Thread\n");
	HANDLE h_rThread_load = CreateRemoteThread(h_process, NULL, 0, lpStartExecAddr, NULL, 0, NULL);
	if (h_rThread_load == NULL) {
		printf("\n[!] CreateRemoteThread Failed! [%d] Exiting....\n", GetLastError());
		return -1;
	}
	printf("[+] Remote Thread created! [%d]\n", GetLastError());
	WaitForSingleObject(h_rThread_load, INFINITE);

	GetExitCodeThread(h_rThread_load, &exit_code);
	HeapFree(GetProcessHeap(), 0, lpWriteBuff);
	CloseHandle(h_rThread_load);
	return exit_code;
}

//===============================================================================================//
/* LOAD_LIBRARY_A DLL INJECTION METHOD */

//The following function is modified from wikipedia page
DWORD inject_DLL_LoadLibrary(const char* file_name, HANDLE h_process)
{
	//getting the full path of the dll file
	char fullDLLPath[_MAX_PATH];
	GetFullPathNameA(file_name, _MAX_PATH, fullDLLPath, NULL);

	//allocating memory in the target processs
	LPVOID DLLPath_addr = VirtualAllocEx(h_process, NULL, _MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//writing the dll path to that memory
	WriteProcessMemory(h_process, DLLPath_addr, fullDLLPath, strlen(fullDLLPath), NULL);

	//getting LoadLibraryA address (same across all processes) to start execution at it
	LPVOID LoadLib_addr = GetProcAddress(GetModuleHandleA("Kernel32"), "LoadLibraryA");

	DWORD exit_code;
	//starting a remote execution thread at LoadLibraryA and passing the dll path as
	//an argument then waiting for it to be finished
	HANDLE h_rThread_load = CreateRemoteThread(h_process, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLib_addr, DLLPath_addr, 0, NULL);
	WaitForSingleObject(h_rThread_load, INFINITE);

	//Retrieving the module handle returned by LoadLibraryA
	GetExitCodeThread(h_rThread_load, &exit_code);

	//Freeing the thread handle and the memory
	CloseHandle(h_rThread_load);

	//allocated for the DLL path
	VirtualFreeEx(h_process, DLLPath_addr, 0, MEM_RELEASE);
	return exit_code;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
