#ifdef KEYHOOKDLL_EXPORTS // 通常在DLL项目中定义这个宏
#define KEYHOOKDLL_API __declspec(dllexport)
#else
#define KEYHOOKDLL_API __declspec(dllimport) // 对于使用DLL的项目，可以使用dllimport
#endif

// 导出函数声明
extern "C" KEYHOOKDLL_API void SetHook(HWND hWnd);
extern "C" KEYHOOKDLL_API void ReleaseHook();
