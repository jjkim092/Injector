// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include "pch.h"
#include <Windows.h>
#include "detours.h"
#include <stdio.h>
#include <wchar.h>

//int (WINAPI *myMsgBox)(HWND hwnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) = MessageBox;
BOOL(WINAPI *myWriteProcessMemory)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) = WriteProcessMemory;

//int WINAPI hookMsgBox(HWND hwnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
//{
//    printf("YOU HOOKED\n");
//    return 0;
//}

BOOL WINAPI hookWriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten)
{
    WCHAR buffer[200] = { 0, };
    swprintf(buffer, 200, L"BaseAddr %p", lpBaseAddress);
    MessageBoxW(NULL, (LPCWSTR)lpBuffer, L"DLL", NULL);
    MessageBoxW(NULL, buffer, L"DETOURS", NULL);
    return myWriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    LONG error;

    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxW(NULL, L"ATTACH!", L"DETOURS", NULL);
        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        //DetourAttach(&(PVOID&)myMsgBox, hookMsgBox);
        DetourAttach(&(PVOID&)myWriteProcessMemory, hookWriteProcessMemory);
        error = DetourTransactionCommit();
        if (error == NO_ERROR)
            MessageBoxW(NULL, L"ACTIVE!", L"DETOURS", NULL);
        else
            MessageBoxW(NULL, L"FAILED!", L"DETOURS", NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

