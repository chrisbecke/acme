#pragma once

#include "winapi.h"
#include <stdint.h>

namespace win32
{
	template <class ClassT>
	class Scoped
	{
		ClassT* inner;
	public:
		Scoped(ClassT* inner) { this->inner = inner; }
		~Scoped() { delete inner; }
		ClassT& operator*() { return *inner; }
	};

	class Window
	{
	protected:
		HWND handle;
	public:
		Window(HWND hwnd) { handle = hwnd; }
		~Window();
		HWND operator*() const { return handle; }

		void show();
	};

	class Control : public Window
	{
	protected:
		int id;
	public:
		Control(HWND hwnd, int id) : Window(hwnd) {
			this->id = id;
		}
	};

	class ListBox : public Control
	{
	public:
		ListBox(HWND hwnd, int id) :Control(hwnd,id) {}

		ListBox& AddString(String string)
		{
			SendDlgItemMessage(handle, id, LB_ADDSTRING, 0, LPARAM(*string));
			return *this;
		}
	};

	// windows that can have event handlers attached derive from eventwindow
	class userwindow : public Window
	{
	public:
		userwindow() : Window(NULL) {}
	};

	class frame : public userwindow
	{
	public:
		void create(string title);

	public:
		intptr WindowProc(uint uMsg, uintptr wParam, intptr lParam);
	};

	class dialog : public Window
	{
	protected:
		bool DialogProc();

	public:
		dialog() : Window(NULL){}
		static dialog* createFromResource(HMODULE module, LPCTSTR resource,HWND owner);
		static dialog* createFromResource(HMODULE module, LPCTSTR resource) { return createFromResource(module, resource, NULL); }
		static dialog* createFromResource(HMODULE module, INT resource) { return createFromResource(module, MAKEINTRESOURCE(resource), NULL); }
		static dialog* createFromResource(HMODULE module, LPCTSTR resource, Window& owner) { return createFromResource(module, resource, *owner); }
		static dialog* createFromResource(HMODULE module, INT resource, Window& owner) { return createFromResource(module, MAKEINTRESOURCE(resource), owner); }

		// Fluent interface.
		template <class ControlT>
		ControlT Control(INT ctlId)
		{
			return ControlT(handle, ctlId);
		}
	};

	class WindowClassBuilder
	{
		WNDCLASSEX wcex;
	public:
		WindowClassBuilder(HINSTANCE hInstance, String classname) {
			ZeroMemory(&wcex, sizeof(wcex));
			wcex.hInstance = hInstance;
			wcex.lpszClassName = *classname;
			wcex.cbSize = sizeof(wcex);
		}
		WNDCLASSEX* operator*() { return &wcex; }
		WindowClassBuilder& asGlobal() { wcex.style |= CS_GLOBALCLASS; return *this; }
		WindowClassBuilder& withIcon(HICON hIcon) { wcex.hIcon = wcex.hIconSm = hIcon; return *this; }
		WindowClassBuilder& withCursor(HINSTANCE hInstance, LPCTSTR id) { wcex.hCursor = LoadCursor(hInstance, id); return *this; }
		WindowClassBuilder& withWindowProc(WNDPROC wndProc) { wcex.lpfnWndProc = wndProc; return *this; }
		WindowClassBuilder& withBackground(HBRUSH hbrush) { wcex.hbrBackground = hbrush; return *this; }

		WindowClassBuilder& withIcon(HINSTANCE hInstance, LPCTSTR id) { return withIcon(LoadIcon(hInstance, id)); }
		WindowClassBuilder& withIcon(HINSTANCE hInstance, int id) { return withIcon(hInstance, MAKEINTRESOURCE(id)); }
		WindowClassBuilder& withIcon(LPCTSTR id) { return withIcon(NULL, id); }
		WindowClassBuilder& withIcon(int id) { return withIcon(NULL, id); }
		WindowClassBuilder& withCursor() { return withCursor(IDC_ARROW); }
		WindowClassBuilder& withCursor(LPCTSTR id) { return withCursor(NULL, id); }
		WindowClassBuilder& withCursor(HINSTANCE hInstance, int id) { return withCursor(hInstance, MAKEINTRESOURCE(id)); }
		WindowClassBuilder& withCursor(int id) { return withCursor(NULL, id); }
		WindowClassBuilder& withBackground(int color) { return withBackground((HBRUSH)(intptr_t)(color + 1)); }
	};


	class WindowCreateBuilder
	{
		DWORD dwStyleEx;
		DWORD dwStyle;
		POINT position;
		SIZE size;
		String title;
		HINSTANCE hInstance;
		HMENU hMenu;
		HWND hwndParent;
	public:
		WindowCreateBuilder(HINSTANCE hInstance) : hInstance(hInstance)
		{
			dwStyleEx = 0l;
			dwStyle = 0l;
			position = { CW_USEDEFAULT, 0 };
			size = { CW_USEDEFAULT, 0 };
			hMenu = NULL;
			hwndParent = NULL;
		}
		WindowCreateBuilder& atPoint(POINT pt) { position = pt; return *this; }
		WindowCreateBuilder& withSize(SIZE size) { this->size = size; return *this; }
		WindowCreateBuilder& asOverlapped() { return *this; }
		WindowCreateBuilder& asOverlappedWindow() { dwStyle |= WS_OVERLAPPEDWINDOW; dwStyleEx |= WS_EX_OVERLAPPEDWINDOW; return *this; }
		WindowCreateBuilder& asPopup() { dwStyle |= WS_POPUP; return *this; }
		WindowCreateBuilder& asPopupWindow() { dwStyle |= WS_POPUPWINDOW; return *this; }
		WindowCreateBuilder& asChild(Window& parent) { hwndParent = *parent; dwStyle |= WS_CHILD | WS_VISIBLE; return *this; }
		WindowCreateBuilder& withTitle(String title) { this->title = title; return *this; }
		WindowCreateBuilder& withStyle(unsigned int styleBits) { dwStyle |= styleBits; return *this; }
		WindowCreateBuilder& withId(UIntPtr id) { hMenu = (HMENU)id; return *this; }


		HWND Create(String className, void* createParam = nullptr)
		{
			return CreateWindowEx(dwStyleEx, *className, *title, dwStyle, position.x, position.y, size.cx, size.cy, hwndParent, hMenu, hInstance, createParam);
		}

	};

	// This is a window builder.
	class WindowClass
	{
		HINSTANCE hInstance;
		ATOM classAtom;
	protected:
		struct CreateThunkBase
		{
			virtual void* operator()(HWND hwnd) = 0;
		};

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

	public:
		// call this with a lambda
		// [](WindowClassBuilder& builder){}
		template<typename lambda_t>
		WindowClass(HINSTANCE hInstance, String classname, lambda_t callback)
		{
			this->hInstance = hInstance;
			WindowClassBuilder builder(hInstance, classname);
			builder
				.withIcon(IDI_APPLICATION)
				.withCursor()
				.withWindowProc(WindowProc);

			callback(builder);

			classAtom = RegisterClassEx(*builder);
		}

		// signature: void (CreateWindowBuilder)
		template<typename window_t, typename lambda_t>
		window_t* Create(lambda_t callback)
		{
			auto builder = WindowCreateBuilder(hInstance);
			callback(builder);
			return new window_t(builder.Create(classAtom, nullptr));
		}
	};
}
