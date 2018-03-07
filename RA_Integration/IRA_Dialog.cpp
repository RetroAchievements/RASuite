#include "common.h"

#include "IRA_Dialog.h"
#include "RA_Core.h"

IRA_Dialog::IRA_Dialog(int resId, HWND parent) :
	ResourceId{resId},
	instance{g_hThisDLLInst},
	ResourceName{MAKEINTRESOURCE(resId)}
{
	if ( !parent )
		parent = g_RAMainWnd;
}

INT_PTR IRA_Dialog::DoModal()
{
	return DialogBox(instance, ResourceName.data(), hwnd_parent, DlgProc);
}

HWND IRA_Dialog::Create()
{
	hDlg = CreateDialog(instance, ResourceName.data(), hwnd_parent, DlgProc);
	return hDlg;
}

INT_PTR IRA_Dialog::MsgQueue(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// This pointer could be any child, this function will attempt to call 
	// child functions first and the abstract as a fail-safe
	IRA_Dialog* ira{ nullptr };

	// The Message Queue
	switch (uMsg)
	{
		HANDLE_MSG(hDlg, WM_INITDIALOG, ira->OnInitDialog);
		HANDLE_MSG(hDlg, WM_COMMAND, ira->OnCommand);
		HANDLE_MSG(hDlg, WM_NCCREATE, ira->OnNCCreate);
		HANDLE_MSG(hDlg, WM_NCDESTROY, ira->OnNCDestroy);
		HANDLE_MSG(hDlg, WM_PAINT, ira->OnPaint);
		HANDLE_MSG(hDlg, WM_ERASEBKGND, ira->OnEraseBkgnd);
		HANDLE_MSG(hDlg, WM_MOUSEWHEEL, ira->OnMouseWheel);
		HANDLE_MSG(hDlg, WM_LBUTTONUP, ira->OnLButtonUp);
		HANDLE_MSG(hDlg, WM_KEYDOWN, ira->OnKeyDown);
		HANDLE_MSG(hDlg, WM_CHAR, ira->OnChar);
		HANDLE_MSG(hDlg, WM_DESTROY, ira->OnDestroy);
		HANDLE_MSG(hDlg, WM_NOTIFY, ira->OnNotify);
		HANDLE_MSG(hDlg, WM_TIMER, ira->OnTimer);
		HANDLE_MSG(hDlg, WM_GETMINMAXINFO, ira->OnGetMinMaxInfo);
		HANDLE_MSG(hDlg, WM_DRAWITEM, ira->OnDrawItem);
		HANDLE_MSG(hDlg, WM_MEASUREITEM, ira->OnMeasureItem);
		HANDLE_MSG(hDlg, WM_SIZE, ira->OnSize);
	default:
		return ira->DialogProc(hDlg, uMsg, wParam, lParam);
	}
}

IRA_Dialog::~IRA_Dialog()
{
	OnClose(hDlg);
	OnClose(hwnd_parent);
	FreeLibrary(instance);
}

// HANDLE_WM_NCCREATE
// See HANDLE_WM_NCCREATE in WindowsX.h
BOOL IRA_Dialog::OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	return SetWindowLongPtr(hwnd, GWL_USERDATA, 
		reinterpret_cast<LONG_PTR>(lpCreateStruct->lpCreateParams));
}

void IRA_Dialog::OnNCDestroy(HWND hwnd)
{
}

void IRA_Dialog::OnPaint(HWND hDlg)
{
}

BOOL IRA_Dialog::OnEraseBkgnd(HWND hDlg, HDC hdc)
{
	return 0;
}

void IRA_Dialog::OnChar(HWND hDlg, TCHAR ch, int cRepeat)
{
}

// mainly for templating
void IRA_Dialog::OnKeyDown(HWND hDlg, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if ( fDown != TRUE )
		throw std::invalid_argument{ "Key is not down" };

	switch ( vk )
	{
	case VK_ACCEPT:
	case VK_LBUTTON:
	case VK_RBUTTON:
	case VK_CANCEL:
	case VK_MBUTTON:
	case VK_XBUTTON1:
	case VK_XBUTTON2:
	case VK_BACK:
	case VK_TAB:
	case VK_CLEAR:
	case VK_RETURN:
	case VK_SHIFT:
	case VK_CONTROL:
	case VK_MENU:
	case VK_PAUSE:
	case VK_CAPITAL:
	case VK_FINAL:
	case VK_ESCAPE:
	case VK_SPACE:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_END:
	case VK_HOME:
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_SELECT:
	case VK_PRINT:
	case VK_EXECUTE:
	case VK_SNAPSHOT:
	case VK_INSERT:
	case VK_DELETE:
	case VK_HELP:
	default:
		break;
	}
}

void IRA_Dialog::OnLButtonUp(HWND hDlg, int x, int y, UINT keyFlags)
{
}

void IRA_Dialog::OnMouseWheel(HWND hDlg, int xPos, int yPos, int zDelta, UINT fwKeys)
{
}

void IRA_Dialog::OnDestroy(HWND hDlg)
{
	PostQuitMessage(EXIT_SUCCESS);
}

LRESULT IRA_Dialog::OnNotify(HWND hDlg, int idFrom, NMHDR * pnmhdr)
{
	// template stuff
	switch ( pnmhdr->code )
	{
	default:
		break;
	}
	return 0;
}

void IRA_Dialog::OnTimer(HWND hDlg, UINT id)
{
}

void IRA_Dialog::OnGetMinMaxInfo(HWND hDlg, LPMINMAXINFO lpMinMaxInfo)
{
}

void IRA_Dialog::OnDrawItem(HWND hDlg, const DRAWITEMSTRUCT* lpDrawItem)
{
}

void IRA_Dialog::OnMeasureItem(HWND hDlg, MEASUREITEMSTRUCT* lpMeasureItem)
{
}

void IRA_Dialog::OnSize(HWND hDlg, UINT state, int cx, int cy)
{
}

#pragma region Button Event Handlers
void IRA_Dialog::OnClose(HWND hDlg)
{
	EndDialog(hDlg, IDCLOSE);
}

void IRA_Dialog::OnOK(HWND hDlg)
{
	EndDialog(hDlg, IDOK);
}

void IRA_Dialog::OnCancel(HWND hDlg)
{
	EndDialog(hDlg, IDCANCEL);
}
HWND IRA_Dialog::MakeControl(int ControlId, HWND hDlg)
{
	if ( !hDlg )
		hDlg = hwnd_parent;
	return GetDlgItem(hDlg, ControlId);
}
#pragma endregion

