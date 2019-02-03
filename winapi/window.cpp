#include "window.h"
#include <assert.h>


using namespace win32;

Window::~Window()
{
}

void Window::show()
{
	ShowWindow(handle, SW_SHOWNORMAL);
}


static const WCHAR* ClassName = L"FrameWindow";

void frame::create(string title)
{
	DWORD dwExStyle = 0;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	HWND hwnd = CreateWindowEx(dwExStyle, ClassName, *title, dwStyle, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, GetCurrentModuleHandle(), this);
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);
}

intptr frame::WindowProc(uint uMsg, uintptr wParam, intptr lParam)
{
	return DefWindowProc(handle,uMsg,wParam,lParam);
}



class frame_impl : public frame
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMSg, WPARAM wParam, LPARAM lParam);

	static void registerClass()
	{
		HINSTANCE hInstance = GetCurrentModuleHandle();

		WNDCLASSEXW wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = ClassName;

		ATOM atm = RegisterClassExW(&wc);
		assert(atm);
	}
};

LRESULT frame_impl::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	frame_impl* self = (frame_impl*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_CREATE:
		CREATESTRUCT * cs;
		cs = (CREATESTRUCT*)lParam;
		self = (frame_impl*)cs->lpCreateParams;
		self->handle = hwnd;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		if (self)
			return ((frame*)self)->WindowProc(uMsg, wParam, lParam);

		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
}

class dialog_impl : public dialog
{

	static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	dialog_impl(HMODULE module, LPCTSTR resource, HWND owner)
	{
		handle = CreateDialogParamW(module, resource, owner, DialogProc, (LPARAM)this);
	}
};

INT_PTR dialog_impl::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}



dialog* dialog::createFromResource(HMODULE module, LPCTSTR resource, HWND owner)
{
	dialog* dlg = new dialog_impl(module,resource,owner);

	return dlg;
}

class frame_register
{
public:
	frame_register()
	{
		frame_impl::registerClass();
	}
};

static frame_register frame_init;


