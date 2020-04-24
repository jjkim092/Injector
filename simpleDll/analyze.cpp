#include <Windows.h>
#include "analyze.h"

BOOL isDebugged()
{
    BYTE Debug = 0;
    __asm
    {
        push eax
        mov eax, fs: [0x30]   // Get PEB
        mov ah, [eax + 2]    // PEB->BeingDebugged
        mov Debug, ah
        pop eax
    }

    return Debug;
}

void unsetDebugRegister()
{
    __asm
    {
        push eax
        push ebx
        xor ebx, ebx
        mov eax, fs:[0x30]
        mov [eax + 2], bh
        pop ebx
        pop eax
    }
}
