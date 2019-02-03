#include "listview.h"
#include <commctrl.h>
#pragma comment(lib,"Comctl32.lib")


using namespace win32;

class listview_init
{
public:
	listview_init()
	{
		INITCOMMONCONTROLSEX icc = { 0 };
		icc.dwSize = sizeof(icc);
		icc.dwICC = ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&icc);
	}

};

listview_init init;


void listview::create(Window& owner)
{
	RECT rect;
	GetClientRect(*owner, &rect);
	handle = CreateWindowExW(0L, WC_LISTVIEW, L"", WS_CHILD|LVS_REPORT|LVS_EDITLABELS|WS_VISIBLE, 0, 0, rect.right - rect.left, rect.bottom - rect.top, *owner, (HMENU)0, GetCurrentModuleHandle(), NULL);
}