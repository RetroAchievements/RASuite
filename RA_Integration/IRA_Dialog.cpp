#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>

#include "IRA_Dialog.h"
#include "RA_Core.h"

long IRA_Dialog::storage{ 0 };

IRA_Dialog::IRA_Dialog(int resId, bool isModal) :
	ResourceId{ resId },
	is_modal{ isModal }
{
	storage = reinterpret_cast<long>(this);
}

IRA_Dialog::~IRA_Dialog()
{
	storage = 0;
}

INT_PTR IRA_Dialog::DoModal()
{
	return DialogBox(g_hThisDLLInst, MAKEINTRESOURCE(ResourceId), g_RAMainWnd, MsgQueue);
}

HWND IRA_Dialog::Create()
{
	return CreateDialog(g_hThisDLLInst, MAKEINTRESOURCE(ResourceId), g_RAMainWnd, MsgQueue);
}

INT_PTR IRA_Dialog::MsgQueue(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Messages encountered we don't have handlers for yet, soley for
	// reference just incase we want to handle them Using decimal instead of
	// hex because that's what shows up

	// WM_SETFONT(48), 
	// WM_NOTIFYFORMAT(129), Since this is MBCS it could just be ANSI 
	// WM_QUERYUISTATE(297),
	// WM_NCACTIVATE(134), WM_GETICON(127), WM_WINDOWPOSCHANGING(70)
	// WM_CHANGEUISTATE(295), WM_UPDATEUISTATE(296)

	IRA_Dialog* ira = reinterpret_cast<IRA_Dialog*>(storage);

	switch ( uMsg )
	{
		HANDLE_MSG(hDlg, WM_CHAR, ira->OnChar);
		HANDLE_MSG(hDlg, WM_CLOSE, ira->OnClose); // forgot this one
		HANDLE_MSG(hDlg, WM_COMMAND, ira->OnCommand);
		HANDLE_MSG(hDlg, WM_DESTROY, ira->OnDestroy);
		HANDLE_MSG(hDlg, WM_DRAWITEM, ira->OnDrawItem);
		HANDLE_MSG(hDlg, WM_ERASEBKGND, ira->OnEraseBkgnd);
		HANDLE_MSG(hDlg, WM_GETMINMAXINFO, ira->OnGetMinMaxInfo);
		HANDLE_MSG(hDlg, WM_INITDIALOG, ira->OnInitDialog);
		HANDLE_MSG(hDlg, WM_KEYDOWN, ira->OnKeyDown);
		HANDLE_MSG(hDlg, WM_LBUTTONUP, ira->OnLButtonUp);
		HANDLE_MSG(hDlg, WM_MEASUREITEM, ira->OnMeasureItem);
		HANDLE_MSG(hDlg, WM_MOUSEWHEEL, ira->OnMouseWheel);
		HANDLE_MSG(hDlg, WM_NCCREATE, ira->OnNCCreate);
		HANDLE_MSG(hDlg, WM_NCDESTROY, ira->OnNCDestroy);
		HANDLE_MSG(hDlg, WM_NOTIFY, ira->OnNotify);
		HANDLE_MSG(hDlg, WM_PAINT, ira->OnPaint);
		HANDLE_MSG(hDlg, WM_SIZE, ira->OnSize);
		HANDLE_MSG(hDlg, WM_TIMER, ira->OnTimer);
	default:
		return ira->DialogProc(hDlg, uMsg, wParam, lParam);
	}
}




// Right-click and go to definition on each function if you want to know where
// it came from (not the '// ')


// HANDLE_WM_NCCREATE
BOOL IRA_Dialog::OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	return SetWindowLongPtr(hwnd, GWLP_USERDATA,
		reinterpret_cast<LONG_PTR>(lpCreateStruct->lpCreateParams));
}

// NB. Don't use DefDlgProc, it's useless and always throws expections

// HANDLE_WM_NCDESTROY
void IRA_Dialog::OnNCDestroy(HWND hwnd)
{
	DefWindowProc(hwnd, WM_NCDESTROY, 0_z, 0_i);
}

// HANDLE_WM_PAINT
void IRA_Dialog::OnPaint(HWND hDlg)
{
	DefWindowProc(hDlg, WM_PAINT, 0_z, 0_i);
}

// HANDLE_WM_ERASEBKGND
BOOL IRA_Dialog::OnEraseBkgnd(HWND hDlg, HDC hdc)
{
	return DefWindowProc(hDlg, WM_ERASEBKGND, reinterpret_cast<WPARAM>(hdc), 0_i);
}

// HANDLE_WM_CHAR
void IRA_Dialog::OnChar(HWND hDlg, TCHAR ch, int cRepeat)
{
	DefWindowProc(hDlg, WM_CHAR, static_cast<WPARAM>(ch), MAKELPARAM(cRepeat, 0));
}

// HANDLE_WM_KEYDOWN
void IRA_Dialog::OnKeyDown(HWND hDlg, UINT vk, BOOL fDown, int cRepeat,
	UINT flags)
{
	DefWindowProc(hDlg, WM_KEYDOWN, static_cast<WPARAM>(vk),
		MAKELPARAM(cRepeat, flags));
}

// HANDLE_WM_LBUTTONUP
void IRA_Dialog::OnLButtonUp(HWND hDlg, int x, int y, UINT keyFlags)
{
	DefWindowProc(hDlg, WM_LBUTTONUP, static_cast<WPARAM>(keyFlags),
		MAKELPARAM(x, y));
}

// HANDLE_WM_MOUSEWHEEL
void IRA_Dialog::OnMouseWheel(HWND hDlg, int xPos, int yPos, int zDelta,
	UINT fwKeys)
{
	DefWindowProc(hDlg, WM_MOUSEWHEEL, MAKEWPARAM(fwKeys, zDelta),
		MAKELPARAM(xPos, yPos));
}

// HANDLE_WM_DESTROY
void IRA_Dialog::OnDestroy(HWND hDlg)
{
	FORWARD_WM_DESTROY(hDlg, PostMessage);
//	PostQuitMessage(0);
}

// HANDLE_WM_NOTIFY
LRESULT IRA_Dialog::OnNotify(HWND hDlg, int idFrom, NMHDR * pnmhdr)
{
	return DefWindowProc(hDlg, WM_NOTIFY, static_cast<WPARAM>(idFrom),
		reinterpret_cast<LPARAM>(pnmhdr));
}

// HANDLE_WM_TIMER
void IRA_Dialog::OnTimer(HWND hDlg, UINT id)
{
	DefWindowProc(hDlg, WM_TIMER, static_cast<WPARAM>(id), 0_i);
}

// HANDLE_WM_GETMINMAXINFO
void IRA_Dialog::OnGetMinMaxInfo(HWND hDlg, LPMINMAXINFO lpMinMaxInfo)
{
	DefWindowProc(hDlg, WM_GETMINMAXINFO, 0_z,
		reinterpret_cast<LPARAM>(lpMinMaxInfo));
}

// HANDLE_WM_DRAWITEM
void IRA_Dialog::OnDrawItem(HWND hDlg, const DRAWITEMSTRUCT* lpDrawItem)
{
	DefWindowProc(hDlg, WM_DRAWITEM, static_cast<WPARAM>(lpDrawItem->CtlID),
		reinterpret_cast<LPARAM>(lpDrawItem));
}

// HANDLE_WM_MEASUREITEM
void IRA_Dialog::OnMeasureItem(HWND hDlg, MEASUREITEMSTRUCT* lpMeasureItem)
{
	DefWindowProc(hDlg, WM_MEASUREITEM, static_cast<WPARAM>(lpMeasureItem->CtlID),
		reinterpret_cast<LPARAM>(lpMeasureItem));
}

// HANDLE_WM_SIZE
void IRA_Dialog::OnSize(HWND hDlg, UINT state, int cx, int cy)
{
	DefDlgProc(hDlg, WM_SIZE, static_cast<WPARAM>(state), MAKELPARAM(cx, cy));
}

// HANDLE_WM_INITDIALOG
BOOL IRA_Dialog::OnInitDialog(HWND hDlg, HWND hDlgFocus, LPARAM lParam)
{
	return FORWARD_WM_INITDIALOG(hDlg, hDlgFocus, lParam, PostMessage);
	//return DefWindowProc(hDlg, WM_INITDIALOG,
	//	reinterpret_cast<WPARAM>(hDlgFocus), lParam);
}

// HANDLE_WM_COMMAND
void IRA_Dialog::OnCommand(HWND hDlg, int id, HWND hDlgCtl, UINT codeNotify)
{
	FORWARD_WM_COMMAND(hDlg, id, hDlgCtl, codeNotify, PostMessage);
	//DefWindowProc(hDlg, WM_COMMAND, MAKEWPARAM(id, codeNotify),
	//	reinterpret_cast<LPARAM>(hDlgCtl));
}

#pragma region Button Event Handlers

// HANDLE_WM_CLOSE
void IRA_Dialog::OnClose(HWND hDlg)
{
	// Note: We aren't calling DefWindowProc or forwarding WM_CLOSE to it
	// because it will call DestroyWindow always.

	if ( is_modal )
		OnOK(hDlg);
	else
		DestroyWindow(hDlg);
}

void IRA_Dialog::OnOK(HWND hDlg)
{
	EndDialog(hDlg, IDOK);
}

void IRA_Dialog::OnCancel(HWND hDlg)
{
	EndDialog(hDlg, IDCANCEL);
}
#pragma endregion

