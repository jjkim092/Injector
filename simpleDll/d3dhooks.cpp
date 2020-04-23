#include "d3dhooks.h"
#include <string>
#include "detours.h"

FILE* pFile = NULL;
__wchar_t tmp[100] = { 0, };

#define ENDSCENE 42
#define DRAWINDEXEDPRIMITIVE 83
#define SETRENDERSTATE 57

DXGH DXGameHook;
//typedef HRESULT(__stdcall* EndScene_t)(LPDIRECT3DDEVICE9);
//HRESULT(*realEndScene)(LPDIRECT3DDEVICE9);
//EndScene_t org_EndScene;
const D3DCOLOR txtPink = D3DCOLOR_ARGB(255, 255, 0, 255);
HRESULT(WINAPI* myEndScene)(LPDIRECT3DDEVICE9 pDevice);
HRESULT(WINAPI* myDrawIndexedPrimitive)(D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
HRESULT(WINAPI* mySetRenderState)(D3DRENDERSTATETYPE, DWORD);

//void* DetourFunc(BYTE* src, const BYTE* dst, const int len)
//{
//    BYTE* jmp = (BYTE*)malloc(len + 5);
//    DWORD dwback;
//    VirtualProtect(src, len, PAGE_READWRITE, &dwback);
//    memcpy(jmp, src, len); jmp += len;
//    jmp[0] = 0xE9;
//    *(DWORD*)(jmp + 1) = (DWORD)(src + len - jmp) - 5;
//    src[0] = 0xE9;
//    *(DWORD*)(src + 1) = (DWORD)(dst - src) - 5;
//    VirtualProtect(src, len, dwback, &dwback);
//    return (jmp - len);
//}

bool bDataCompare(const BYTE* pData, const BYTE* bMask, const WCHAR* szMask)
{
    ////swprintf_s(tmp, 100, L"bDataCompare pData %08x\n", pData);
    ////fputws(tmp, pFile);
     //x = 120
     //? = 63
    for (DWORD i = 0; i < wcslen(szMask); i++)
    {
        if (szMask[i] == 120)
        {
            //swprintf_s(tmp, 100, L"pData %02x bData %02x\n", pData[i], bMask[i]);
            //fputws(tmp, pFile);
            if (pData[i] != bMask[i])
            {
                break;
            }
            else
            {
                //fputws(L"Correct\n", pFile);
                if (i == wcslen(szMask) - 1)
                {
                    //fputws(L"All Correct\n", pFile);
                    //swprintf_s(tmp, 100, L"pData %08x\n", pData);
                    //fputws(tmp, pFile);
                    return true;
                }
            }
        }
    }

    return false;
}

DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, const WCHAR* szMask)
{
    for (DWORD i = 0; i < wcslen(szMask); i++)
    {
        //swprintf_s(tmp, 100, L"szMask %c %d\n", szMask[i], szMask[i]);
        //fputws(tmp, pFile);
    }
    //swprintf_s(tmp, 100, L"FindPattern dwAddress %08x dwLen %08x\n", dwAddress, dwLen);
    //fputws(tmp, pFile);
    //swprintf_s(tmp, 100, L"FindPattern bMask %02x %02x %02x %02x\n", bMask[0], bMask[1], bMask[2], bMask[3]);
    //fputws(tmp, pFile);
    //swprintf_s(tmp, 100, L"FindPattern szMask %s\n", szMask);
    //fputws(tmp, pFile);
    for (DWORD i = 0; i < dwLen; i++)
    {
        if (bDataCompare((BYTE*)(dwAddress + i), bMask, szMask))
        {
            //swprintf_s(tmp, 100, L"FindPattern FIND !! dwAddress %p\n", dwAddress + i);
            //fputws(tmp, pFile);
            return (DWORD)(dwAddress + i);
        }
    }
    return 0;
}

void DXGH::DrawRect(LPDIRECT3DDEVICE9 Device_t, int X, int Y, int L, int H, D3DCOLOR color)
{
    D3DRECT rect = { X, Y, X + L, Y + H };
    Device_t->Clear(1, &rect, D3DCLEAR_TARGET, color, 0, 0);
}

HRESULT WINAPI DXGH::h_EndScene(LPDIRECT3DDEVICE9 pDevice)
{
    DXGameHook.DrawRect(pDevice, 10, 10, 200, 200, txtPink);
    //return org_EndScene(pDevice);
    return NULL;
}

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

HRESULT WINAPI hookEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    DXGameHook.DrawRect(pDevice, 10, 10, 200, 200, txtPink);
    return myEndScene(pDevice);
}

HRESULT WINAPI hookDrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT startIndex, UINT PrimitiveCount)
{
    mySetRenderState(D3DRS_ZENABLE, false);

    myDrawIndexedPrimitive(type, BaseVertexIndex, MinIndex, NumVertices, startIndex, PrimitiveCount);

    mySetRenderState(D3DRS_ZENABLE, true);

    return myDrawIndexedPrimitive(type, BaseVertexIndex, MinIndex, NumVertices, startIndex, PrimitiveCount);
}

int StartD3DHooks()
{
    DetourIsHelperProcess();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    //DetourSetIgnoreTooSmall(TRUE);

    //pFile = fopen("C:\\test\\info.txt", "w");
    DWORD D3DPattern = 0, * vTable, DXBase = NULL;
    DXBase = (DWORD)LoadLibraryA("d3d9.dll");

    D3DPattern = FindPattern(DXBase, 0x18e000,
        (PBYTE)"\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86", L"xx????xx????xx");

    LONG error;

    if (D3DPattern)
    {
        //MessageBox(NULL, L"HOOK", L"FIND PATTERN!!", NULL);
        memcpy(&vTable, (void*)(D3DPattern + 2), 4);

        myEndScene = (EndScene_t)vTable[ENDSCENE];
        myDrawIndexedPrimitive = (DrawIndexedPrimitive_t)vTable[DRAWINDEXEDPRIMITIVE];
        mySetRenderState = (SetRenderState_t)vTable[SETRENDERSTATE];

        //swprintf_s(tmp, 100, L"FindPattern FIND !! vTable %p ENDSCENE %p\n",vTable, vTable[ENDSCENE]);
        //fputws(tmp, pFile);
        DetourAttach(&(PVOID&)myEndScene, hookEndScene);
        DetourAttach(&(PVOID&)myDrawIndexedPrimitive, hookDrawIndexedPrimitive);
        error = DetourTransactionCommit();
        //if (error == NO_ERROR)
        //{
        //    MessageBoxW(NULL, L"ACTIVE!", L"DETOURS", NULL);
        //}
        //else
        //{
        //    MessageBoxW(NULL, L"FAILED!", L"DETOURS", NULL);
        //}
        //memcpy(&vTable, (void*)(D3DPattern + 2), 4);
        //org_EndScene = (EndScene_t)DetourFunc((PBYTE)vTable[ENDSCENE],
        //    (PBYTE)DXGameHook.h_EndScene, 5);
    }
    //fclose(pFile);
    return 0;
}