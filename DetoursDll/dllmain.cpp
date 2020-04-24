// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include "pch.h"
#include <Windows.h>
#include "detours.h"
#include <stdio.h>
#include <wchar.h>
#include <atlbase.h>

#define LOGPATH "C:\\test\\info.txt"

//int (WINAPI *myMsgBox)(HWND hwnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) = MessageBox;
//int WINAPI hookMsgBox(HWND hwnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
//{
//    printf("YOU HOOKED\n");
//    return 0;
//}

BOOL(WINAPI *myWriteProcessMemory)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) = WriteProcessMemory;
BOOL WINAPI hookWriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten)
{
    FILE* pFile = NULL;
    WCHAR buffer[300] = { 0, };
    pFile = fopen(LOGPATH, "a");
    if (pFile!= NULL)
    {
        swprintf(buffer, 300, L"Path %ls BaseAddr %p\n", (LPCTSTR)lpBuffer, lpBaseAddress);
        fputws(buffer, pFile);
    }
    fclose(pFile);
    return myWriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}

LPVOID(WINAPI* myVirtualAllocEx)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) = VirtualAllocEx;
LPVOID WINAPI hookVirtualAllocEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
    LPVOID retValue = myVirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
    FILE* pFile = NULL;
    WCHAR buffer[300] = { 0, };
    pFile = fopen(LOGPATH, "a");
    if (pFile != NULL)
    {
        swprintf(buffer, 300, L"BaseAddress %08x Size %lu Type %08x Protect %08x\n", retValue, dwSize, flAllocationType, flProtect);
        fputws(buffer, pFile);
    }
    fclose(pFile);
    return retValue;
}

#define A2W_S(lpa) (\
    ((_lpa = lpa) == NULL) ? NULL : (\
        _convert = (static_cast<int>(strlen(_lpa))+1),\
        (INT_MAX/2<_convert)? NULL :  \
        ATLA2WHELPER((LPWSTR) _malloca(_convert*sizeof(WCHAR)), _lpa, _convert, _acp)))

FARPROC(WINAPI* myGetProcAddress)(HMODULE hMod, LPCSTR lpProcName) = GetProcAddress;
FARPROC WINAPI hookGetProcAddress(HMODULE hMod, LPCSTR lpProcName)
{
    FILE* pFile = NULL;
    WCHAR buffer[300] = { 0, };
    pFile = fopen(LOGPATH, "a");
    if (pFile != NULL)
    {
        USES_CONVERSION;
        swprintf(buffer, 300, L"ProcName %s\n", A2W_S(lpProcName));
        fputws(buffer, pFile);
    }
    fclose(pFile);
    return myGetProcAddress(hMod, lpProcName);
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
        DisableThreadLibraryCalls(hModule);
        remove(LOGPATH);
        //MessageBoxW(NULL, L"ATTACH!", L"DETOURS", NULL);
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        //DetourAttach(&(PVOID&)myMsgBox, hookMsgBox);
        DetourAttach(&(PVOID&)myWriteProcessMemory, hookWriteProcessMemory);
        DetourAttach(&(PVOID&)myVirtualAllocEx, hookVirtualAllocEx);
        DetourAttach(&(PVOID&)myGetProcAddress, hookGetProcAddress);

        error = DetourTransactionCommit();
        /*if (error == NO_ERROR)
            MessageBoxW(NULL, L"ACTIVE!", L"DETOURS", NULL);
        else
            MessageBoxW(NULL, L"FAILED!", L"DETOURS", NULL);*/

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

