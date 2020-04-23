#pragma once

#include "d3d9.h"

class DXGH {
public:
    static HRESULT WINAPI h_EndScene(LPDIRECT3DDEVICE9 pDevice);
    void DrawRect(LPDIRECT3DDEVICE9 Device_t, int X, int Y, int L, int H, D3DCOLOR color);
};

int StartD3DHooks();

typedef HRESULT(__stdcall* EndScene_t)(LPDIRECT3DDEVICE9);
typedef HRESULT(__stdcall* DrawIndexedPrimitive_t)(D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
typedef HRESULT(__stdcall* SetRenderState_t)(D3DRENDERSTATETYPE, DWORD);

extern DXGH DXGameHook;