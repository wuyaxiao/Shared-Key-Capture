// dllmain.cpp (示例)
#include "pch.h"
#include <windows.h>

extern "C" __declspec(dllexport) void ReleaseHook(); 

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule); 
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        // 在 DLL 卸载时调用 ReleaseHook 释放钩子
        ReleaseHook();
        break;
    }
    return TRUE;
}
