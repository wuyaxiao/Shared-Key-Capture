#ifdef KEYHOOKDLL_EXPORTS // ͨ����DLL��Ŀ�ж��������
#define KEYHOOKDLL_API __declspec(dllexport)
#else
#define KEYHOOKDLL_API __declspec(dllimport) // ����ʹ��DLL����Ŀ������ʹ��dllimport
#endif

// ������������
extern "C" KEYHOOKDLL_API void SetHook(HWND hWnd);
extern "C" KEYHOOKDLL_API void ReleaseHook();
