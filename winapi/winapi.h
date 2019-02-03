#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


#ifndef LIBWIN32
#pragma comment(lib,"winapi.lib")
#endif

namespace winapi
{
	typedef char	utf8;
	typedef wchar_t utf16;
	typedef char	ansi;


#ifdef _UNICODE
	typedef wchar_t tchar;
#else
	typedef char tchar;
#endif

	class string
	{
		const wchar_t* text;
	public:
		string(const wchar_t* r = nullptr) { text = r; }
		string(const string& r) { text = r.text; }
		string(ATOM rhs) { text = (const wchar_t*)(rhs); }
		const wchar_t* operator *() { return text; }
	};
}

namespace win32
{
	using namespace winapi;

#if _MSC_VER >= 1400

	EXTERN_C IMAGE_DOS_HEADER __ImageBase;

	inline HMODULE GetCurrentModuleHandle()
	{
		return (HMODULE)&__ImageBase;
	}

#endif

	typedef unsigned int uint;
	typedef unsigned long long uintptr;
	typedef long long intptr;
	typedef wchar_t UniChar;

	// compatible with size_t
	typedef long long IntPtr;
	typedef unsigned long long UIntPtr;

#ifdef _UNICODE
	typedef wchar_t TextChar;
#else
	typedef char TextChar;
#endif

	class String {
		const TextChar* ptr;
	public:
		String(const TextChar* rhs=nullptr) { ptr = rhs; }
		String(const String& rhs) { ptr = rhs.ptr; }
		String(ATOM rhs) { ptr = (const TextChar*)(rhs); }
		const TextChar* operator *() { return ptr; }
	};

	class StringBuffer
	{
	public:
		TextChar* inner;
		TextChar* operator*() { return inner; }
		StringBuffer(IntPtr size) { inner = new TextChar[size]; }
		StringBuffer(const StringBuffer& rhs) = delete;
		StringBuffer(StringBuffer&& rhs) { inner = rhs.inner; rhs.inner = nullptr; }
		StringBuffer& operator =(const StringBuffer& rhs) = delete;
		StringBuffer& operator=(StringBuffer&& rhs) { inner = rhs.inner; rhs.inner = nullptr; return *this; }
		~StringBuffer() { delete inner; }
	};

	class Convert {
		// returns the size of a buffer necessary to hold a zero terminated array of CharacterT
		template<typename CharT>
		static IntPtr size(const CharT* first)
		{
			auto last=first;
			while (*last++);
			return last - first;
		}

		template<typename CharT>
		static StringBuffer Latin1ToUnicode(const CharT*source)
		{
			auto len = size(source);
			StringBuffer buffer(len);
			UniChar* dest = *buffer;
			for (auto i = 0; i < len; i++)
				dest[i] = source[i];
			return buffer;
		}
	public:
#ifdef _UNICODE
		static StringBuffer ToString(const char* source)
		{
			return Latin1ToUnicode(source);
		}
		static StringBuffer ToString(const UCHAR* source)
		{
			return Latin1ToUnicode(source);
		}

		static StringBuffer ToString(int val)
		{
			StringBuffer buffer(10);
			wsprintf(*buffer, TEXT("%d"), val);
			return buffer;
		}

#endif
	};

	class Application
	{
	public:
		static int Run();
	};

	typedef class Application application;
}
