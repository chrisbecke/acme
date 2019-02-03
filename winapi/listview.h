#pragma once

#include "winapi.h"
#include "window.h"

#include <commctrl.h>
#pragma comment(lib,"Comctl32.lib")


namespace win32 {



	class listview : public Control
	{
	public:
//		listview();
//		~listview();
		void create(Window& parent);
	};






	class ListView : public Window
	{
	public:
		ListView(HWND hwnd) : Window(hwnd) {}

		// Signature: void(WindowCreateBuilder&)
		template<typename lambda_t>
		static ListView* Create(HINSTANCE hInstance, Window& parent, lambda_t callback)
		{
			WindowCreateBuilder builder = WindowCreateBuilder(hInstance)
				.asChild(parent);

			callback(builder);

			return new ListView(builder.Create(WC_LISTVIEW));
		}



		class ListViewColumnBuilder
		{
			String _label;
		protected:
			LVCOLUMN lvc;
		public:
			ListViewColumnBuilder() :lvc({ 0 }) {}
			ListViewColumnBuilder& withLabel(String label) { lvc.mask |= LVCF_TEXT; _label = label; lvc.pszText = (LPTSTR)*_label; return *this; }
			ListViewColumnBuilder& withWidth(int cx) { lvc.mask |= LVCF_WIDTH; lvc.cx = cx; return *this; }
			ListViewColumnBuilder& withColumn(int iCol) { lvc.mask |= LVCF_SUBITEM; lvc.iSubItem = iCol; return *this; }
			ListViewColumnBuilder& alignLeft() { lvc.mask |= LVCF_FMT | LVCFMT_LEFT; return *this; }
			ListViewColumnBuilder& alignRight() { lvc.mask |= LVCF_FMT | LVCFMT_RIGHT; return *this; }
		};

		class ColumnAdder : public ListViewColumnBuilder
		{
			HWND lv;
			int iCol;
			friend class ListView;
			ColumnAdder(HWND lv, int iCol) :lv(lv), iCol(iCol) {}
		public:
			~ColumnAdder() {
				ListView_InsertColumn(lv, iCol, &lvc);
			}
		};

		class ColumnOps
		{
			HWND _hwnd;
		public:
			ColumnOps(HWND hwndListView) : _hwnd(hwndListView) {}

			ColumnAdder Add(int iCol) {
				return ColumnAdder(_hwnd, iCol);
			}
		};

		ColumnOps Column()
		{
			return ColumnOps(handle);
		}

		class ItemBuilder
		{
		protected:
			LVITEM item;
			ItemBuilder() :item({ 0 }) {}
		public:
			ItemBuilder& withText(LPCTSTR text) { item.pszText = (LPTSTR)text; item.mask |= LVIF_TEXT; return *this; }
		};

		class InsertItem : public ItemBuilder
		{
			friend class ListView;
			HWND hwnd;
			InsertItem(HWND hwnd, int iItem) :hwnd(hwnd) { item.iItem = iItem; }
		public:
			~InsertItem() {
				item.iSubItem = 0; // required
				ListView_InsertItem(hwnd, &item);
			}
		};

		class ItemOperations
		{
			HWND hwnd;
			int iItem;
			int iSubItem;
			ItemOperations(HWND hwnd, int iItem=-1, int iSubItem=-1) :hwnd(hwnd),iItem(iItem),iSubItem(iSubItem) {}
			friend class ListView;
		public:
			InsertItem Insert(int iItem) { return InsertItem(hwnd, iItem); }
			ItemOperations setText(int iSubItem, String text) {	ListView_SetItemText(hwnd, iItem, iSubItem, (LPWSTR)*text);	return *this; }
		};

		ItemOperations Item(int iItem=-1,int iSubItem=-1) {
			return ItemOperations(handle,iItem);
		}


	};
}

