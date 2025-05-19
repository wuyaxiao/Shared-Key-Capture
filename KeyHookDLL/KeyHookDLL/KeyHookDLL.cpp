#include "pch.h" // ���������Ŀʹ��Ԥ����ͷ�ļ�

#include <windows.h>
#include <string.h> // ���� _strdup
#include <stdlib.h> // ���� free
#include <stdio.h>  // ��ѡ�����ڵ�����������հ汾Ӧ�Ƴ�

// ȫ�ֱ���
HHOOK g_hKeyboardHook = NULL; // ���Ӿ��
HWND g_hClientWnd = NULL;     // �ͻ��˴��ھ��


// ���ӹ��̺��� (�ͼ����̹���)
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // ȷ���������¼���nCode С�� 0 ��ʾע���¼���Ӧֱ�Ӵ���
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        // ֻ�����������¼� (WM_KEYDOWN �� WM_SYSKEYDOWN)
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            // --- �ؼ����֣���ȡ������Ϣ����ʽ��Ϊ char* ---
            char* keyInfo = nullptr; // ���ڴ洢����Ҫ���͵� char*

            // ���Խ��������ת��Ϊ�ַ��򰴼�����
            char keyString[50]; // �㹻�洢������������ƻ�VK��

            // ��ȡ�������� (���Ѻ�)
            // GetKeyNameTextA ��Ҫ scanCode �ĵ� 16 λ����Ϊ 1 ��ʾ��չ��
            // ���� WH_KEYBOARD_LL��lParam->flags ���� LLKHF_EXTENDED
            if (GetKeyNameTextA(p->scanCode | (p->flags & LLKHF_EXTENDED ? 0x1000000 : 0), keyString, sizeof(keyString)))
            {
                keyInfo = _strdup(keyString); // ʹ�� _strdup �����ڴ沢�����ַ���
            }
            else
            {
                // �����ȡ����ʧ�ܣ��������������ַ�����ʽ
                sprintf_s(keyString, "VK_%X", p->vkCode);
                keyInfo = _strdup(keyString);
            }

            // --- ������Ϣ���ͻ��˴��� ---
            // ���ͻ��˴��ھ���Ƿ���Ч
            if (g_hClientWnd && IsWindow(g_hClientWnd) && keyInfo)
            {
                BOOL postResult = PostMessage(g_hClientWnd, WM_USER + 100, (WPARAM)keyInfo, 0);

                if (!postResult) {
                    free(keyInfo);
                    keyInfo = nullptr; // ���ָ�룬�����������
                }

            }
            else if (keyInfo) {
                // ����ͻ��˴��ھ����Ч��Ϊ NULL��Ҳ��Ҫ�ͷ��ڴ�
                free(keyInfo);
                keyInfo = nullptr; // ���ָ��
            }
        }
    }

    // �������е���һ������
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// DLL �������������ù���
// ���� hWnd �ǿͻ��˴��ھ�������ڷ�����Ϣ
extern "C" __declspec(dllexport) void SetHook(HWND hWnd)
{
    // ��鴫��Ĵ��ھ���Ƿ���Ч
    if (!IsWindow(hWnd))
    {
        g_hClientWnd = NULL; // ȷ�����Ϊ NULL
        return;
    }

    g_hClientWnd = hWnd; // ����ͻ��˴��ھ��

    g_hKeyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,       // �������ͣ��ͼ����̹���
        LowLevelKeyboardProc, // ���ӹ��̺���
        GetModuleHandle(NULL),// DLL ʵ�����
        0                     // �߳� ID (0 ��ʾϵͳ��Χ)
    );

    if (!g_hKeyboardHook)
    {
        g_hClientWnd = NULL; // ����ʧ�ܣ�����ͻ��˾��
    }
    else {
    }
}

// DLL �����������ͷŹ���
extern "C" __declspec(dllexport) void ReleaseHook()
{
    if (g_hKeyboardHook)
    {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }

    g_hClientWnd = NULL; // ����ͻ��˴��ھ��
}
