// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include "pch.h"
#include <Windows.h>
#include <string>
#include <tchar.h>
#include "d3dhooks.h"
#include "analyze.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    char Debug = 0;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //MessageBox(NULL, _T("Injected"), _T("Dll Process Attach"), NULL);
        DisableThreadLibraryCalls(hModule);
        //pFile = fopen("C:\\test\\info.txt", "w");
        //StartD3DHooks();
        //fclose(pFile);
        //MessageBox(NULL, _T("Injected"), _T("Dll Process Attach End"), NULL);

        printf("Debug %d\n", (int)Debug);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

