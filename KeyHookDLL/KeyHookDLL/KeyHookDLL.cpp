#include "pch.h" // 如果您的项目使用预编译头文件

#include <windows.h>
#include <string.h> // 用于 _strdup
#include <stdlib.h> // 用于 free
#include <stdio.h>  // 可选，用于调试输出，最终版本应移除

// 全局变量
HHOOK g_hKeyboardHook = NULL; // 钩子句柄
HWND g_hClientWnd = NULL;     // 客户端窗口句柄


// 钩子过程函数 (低级键盘钩子)
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // 确保处理钩子事件，nCode 小于 0 表示注入事件，应直接传递
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        // 只处理按键按下事件 (WM_KEYDOWN 或 WM_SYSKEYDOWN)
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            // --- 关键部分：获取按键信息并格式化为 char* ---
            char* keyInfo = nullptr; // 用于存储最终要发送的 char*

            // 尝试将虚拟键码转换为字符或按键名称
            char keyString[50]; // 足够存储大多数按键名称或VK码

            // 获取按键名称 (更友好)
            // GetKeyNameTextA 需要 scanCode 的第 16 位设置为 1 表示扩展键
            // 对于 WH_KEYBOARD_LL，lParam->flags 中有 LLKHF_EXTENDED
            if (GetKeyNameTextA(p->scanCode | (p->flags & LLKHF_EXTENDED ? 0x1000000 : 0), keyString, sizeof(keyString)))
            {
                keyInfo = _strdup(keyString); // 使用 _strdup 分配内存并复制字符串
            }
            else
            {
                // 如果获取名称失败，发送虚拟键码的字符串形式
                sprintf_s(keyString, "VK_%X", p->vkCode);
                keyInfo = _strdup(keyString);
            }

            // --- 发送消息给客户端窗口 ---
            // 检查客户端窗口句柄是否有效
            if (g_hClientWnd && IsWindow(g_hClientWnd) && keyInfo)
            {
                BOOL postResult = PostMessage(g_hClientWnd, WM_USER + 100, (WPARAM)keyInfo, 0);

                if (!postResult) {
                    free(keyInfo);
                    keyInfo = nullptr; // 清空指针，避免后续误用
                }

            }
            else if (keyInfo) {
                // 如果客户端窗口句柄无效或为 NULL，也需要释放内存
                free(keyInfo);
                keyInfo = nullptr; // 清空指针
            }
        }
    }

    // 调用链中的下一个钩子
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// DLL 导出函数：设置钩子
// 参数 hWnd 是客户端窗口句柄，用于发送消息
extern "C" __declspec(dllexport) void SetHook(HWND hWnd)
{
    // 检查传入的窗口句柄是否有效
    if (!IsWindow(hWnd))
    {
        g_hClientWnd = NULL; // 确保句柄为 NULL
        return;
    }

    g_hClientWnd = hWnd; // 保存客户端窗口句柄

    g_hKeyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,       // 钩子类型：低级键盘钩子
        LowLevelKeyboardProc, // 钩子过程函数
        GetModuleHandle(NULL),// DLL 实例句柄
        0                     // 线程 ID (0 表示系统范围)
    );

    if (!g_hKeyboardHook)
    {
        g_hClientWnd = NULL; // 钩子失败，清除客户端句柄
    }
    else {
    }
}

// DLL 导出函数：释放钩子
extern "C" __declspec(dllexport) void ReleaseHook()
{
    if (g_hKeyboardHook)
    {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }

    g_hClientWnd = NULL; // 清除客户端窗口句柄
}
