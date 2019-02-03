#include "winapi.h"

using namespace win32;

int Application::Run()
{
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return int(msg.wParam);
}
