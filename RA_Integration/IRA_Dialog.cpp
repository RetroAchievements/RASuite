#include "IRA_Dialog.h"
#include "RA_Core.h"

namespace ra {
/// <summary>
/// The storage{CC2D43FA-BBC4-448A-9D0B-7B57ADF2655C}
/// </summary>
LRESULT IRA_Dialog::lStorage{ 0_i };

// Note to self: Never call DefWindowProc or DefDlgProc for dialogs
// Note to self: Never Post anything to Windows
// DefWindowProc seems to mess up the focus

IRA_Dialog::IRA_Dialog(int resId) :
	nResourceId{ resId }
{
	lStorage = reinterpret_cast<LRESULT>(this);
}

IRA_Dialog::~IRA_Dialog() noexcept
{
	lStorage = 0;

}

INT_PTR IRA_Dialog::DoModal()
{
	bIsModal = true;
	return DialogBox(g_hThisDLLInst, MAKEINTRESOURCE(nResourceId), g_RAMainWnd,
		MsgQueue);
}


HWND IRA_Dialog::Create()
{
	// alright, all the modeless need their own HWND
	bIsModal = false;

	return CreateDialog(g_hThisDLLInst, MAKEINTRESOURCE(nResourceId),
		g_RAMainWnd, MsgQueue);
}

void IRA_Dialog::PostNcDestroy() noexcept
{
	OnNCDestroy(GetActiveWindow());
}

// OK, bBigIcon was resolved, but not dpi, might need to get rid of it
HICON IRA_Dialog::GetIcon(HWND hwnd, BOOL bBigIcon)
{
	if (!hIcon)
	{
		// Post it, or recurision will happen, it's a queued message
		hIcon = FORWARD_WM_GETICON(hwnd, bBigIcon, PostMessage);
		return nullptr;
	}
	return Static_GetIcon(hwnd, hIcon);
}

HICON IRA_Dialog::SetIcon(HWND hwnd, BOOL bBigIcon, HICON hicon)
{
	if (!hicon)
		return FORWARD_WM_SETICON(hwnd, bBigIcon, hicon, PostMessage);

	return Static_SetIcon(hwnd, hicon);
}

INT_PTR IRA_Dialog::MsgQueue(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// test
	auto ira{ reinterpret_cast<IRA_Dialog*>(lStorage) };


	// BN_CLICKED
	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_ACTIVATE, ira->Activate);
		HANDLE_MSG(hwnd, WM_ACTIVATEAPP, ira->ActivateApp);
		HANDLE_MSG(hwnd, WM_CANCELMODE, ira->CancelMode);
		HANDLE_MSG(hwnd, WM_CHAR, ira->OnChar);
		HANDLE_MSG(hwnd, WM_CLOSE, ira->Close);
		HANDLE_MSG(hwnd, WM_COMMAND, ira->OnCommand);
		HANDLE_MSG(hwnd, WM_CTLCOLORBTN, ira->OnCtlColorBtn);
		HANDLE_MSG(hwnd, WM_CTLCOLORDLG, ira->OnCtlColorDlg);
		HANDLE_MSG(hwnd, WM_CTLCOLOREDIT, ira->OnCtlColorEdit);
		HANDLE_MSG(hwnd, WM_CTLCOLORLISTBOX, ira->OnCtlColorListbox);
		HANDLE_MSG(hwnd, WM_CTLCOLORMSGBOX, ira->OnCtlColorMsgbox);
		HANDLE_MSG(hwnd, WM_CTLCOLORSCROLLBAR, ira->OnCtlColorScrollbar);
		HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC, ira->OnCtlColorStatic);
		HANDLE_MSG(hwnd, WM_DESTROY, ira->Destroy);
		HANDLE_MSG(hwnd, WM_DRAWITEM, ira->DrawItem);
		HANDLE_MSG(hwnd, WM_ENABLE, ira->Enable);
		HANDLE_MSG(hwnd, WM_ERASEBKGND, ira->EraseBkgnd);
		HANDLE_MSG(hwnd, WM_GETFONT, ira->GetFont);
		HANDLE_MSG(hwnd, WM_GETICON, ira->GetIcon); // custom
		HANDLE_MSG(hwnd, WM_GETMINMAXINFO, ira->GetMinMaxInfo);
		HANDLE_MSG(hwnd, WM_GETTEXT, ira->GetText);
		HANDLE_MSG(hwnd, WM_GETTEXTLENGTH, ira->GetTextLength);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ira->OnInitDialog);
		HANDLE_MSG(hwnd, WM_KEYDOWN, ira->OnKeyDown);
		HANDLE_MSG(hwnd, WM_KILLFOCUS, ira->KillFocus);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, ira->OnLButtonUp);
		HANDLE_MSG(hwnd, WM_MEASUREITEM, ira->MeasureItem);
		HANDLE_MSG(hwnd, WM_MOUSEWHEEL, ira->OnMouseWheel);
		HANDLE_MSG(hwnd, WM_MOVE, ira->Move);
		HANDLE_MSG(hwnd, WM_NCACTIVATE, ira->OnNCActivate);
		HANDLE_MSG(hwnd, WM_NCCREATE, ira->OnNCCreate);
		HANDLE_MSG(hwnd, WM_NCDESTROY, ira->OnNCDestroy);
		HANDLE_MSG(hwnd, WM_NCPAINT, ira->OnNCPaint);
		HANDLE_MSG(hwnd, WM_NOTIFY, ira->OnNotify);
		HANDLE_MSG(hwnd, WM_PAINT, ira->OnPaint);
		HANDLE_MSG(hwnd, WM_QUIT, ira->OnQuit);
		HANDLE_MSG(hwnd, WM_SETFOCUS, ira->SetFocus);
		HANDLE_MSG(hwnd, WM_SETFONT, ira->SetFont);
		HANDLE_MSG(hwnd, WM_SETICON, ira->SetIcon); // custom
		HANDLE_MSG(hwnd, WM_SETTEXT, ira->SetText);
		HANDLE_MSG(hwnd, WM_SHOWWINDOW, ira->OnShowWindow);
		HANDLE_MSG(hwnd, WM_SIZE, ira->OnSize); 
		HANDLE_MSG(hwnd, WM_TIMER, ira->OnTimer);
		HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED, ira->OnWindowPosChanged);
		HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING, ira->OnWindowPosChanging);
	default:
		return ira->DialogProc(hwnd, uMsg, wParam, lParam);
	}
}

inline void IRA_Dialog::delete_font() {
	DeleteFont(hFont);
	hFont = nullptr;
}

inline void IRA_Dialog::delete_icon() noexcept
{
	DestroyIcon(hIcon);
	hIcon = nullptr;
}




// Right-click and go to definition on each function if you want to know where
// it came from (not the '// ')


// HANDLE_WM_NCCREATE
BOOL IRA_Dialog::OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	return _CSTD SetWindowLongPtr(hwnd, GWLP_USERDATA,
		reinterpret_cast<LRESULT>(lpCreateStruct->lpCreateParams));
}

// NB. Don't use DefDlgProc, it's useless and always throws expections

// HANDLE_WM_NCDESTROY
void IRA_Dialog::OnNCDestroy(HWND hwnd)
{
	if (hFont)
		delete_font();
	if (hIcon)
		delete_icon();
	if (hwnd)
		DestroyControl(hwnd); // could use a better name
}

// HANDLE_WM_PAINT
void IRA_Dialog::OnPaint(_UNUSED HWND hwnd)
{
}

// HANDLE_WM_ERASEBKGND
BOOL IRA_Dialog::EraseBkgnd(HWND hwnd, HDC hdc)
{
	return 0;
}

// HANDLE_WM_CHAR
void IRA_Dialog::OnChar(HWND hwnd, TCHAR ch, int cRepeat)
{
}

// HANDLE_WM_KEYDOWN
void IRA_Dialog::OnKeyDown(HWND hwnd, UINT vk, _UNUSED BOOL fDown, int cRepeat,
	UINT flags)
{
}

// HANDLE_WM_LBUTTONUP
void IRA_Dialog::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
}

// HANDLE_WM_MOUSEWHEEL
void IRA_Dialog::OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta,
	UINT fwKeys)
{
}

// HANDLE_WM_DESTROY
void IRA_Dialog::Destroy(_UNUSED HWND hwnd)
{
	PostQuitMessage(0);
} // end function Destroy

void IRA_Dialog::OnNCPaint(HWND hwnd, HRGN hrgn)
{
	// found on MSDN
	auto hdc{ GetDCEx(hwnd, hrgn, DCX_WINDOW | DCX_INTERSECTRGN) };
	// Paint into this DC 
	ReleaseDC(hwnd, hdc);
}

// HANDLE_WM_NOTIFY
LRESULT IRA_Dialog::OnNotify(HWND hwnd, int idFrom, NMHDR* pnmhdr)
{
	return 0;
}

// HANDLE_WM_TIMER
void IRA_Dialog::OnTimer(_UNUSED HWND hwnd, _UNUSED UINT id)
{
}

// HANDLE_WM_GETMINMAXINFO
void IRA_Dialog::GetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
}

// HANDLE_WM_DRAWITEM
void IRA_Dialog::DrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
}

// HANDLE_WM_MEASUREITEM
void IRA_Dialog::MeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
}

void IRA_Dialog::Move(HWND hwnd, _UNUSED int x, _UNUSED int y)
{
	RememberWindowPosition(hwnd, lpCaption.get());
} // end function Move

// HANDLE_WM_SIZE
void IRA_Dialog::OnSize(HWND hwnd, _UNUSED UINT state, _UNUSED int cx,
	_UNUSED int cy)
{
	// doesn't make a lot of sense but I've noticed a lot windows doing this
	RememberWindowSize(hwnd, lpCaption.get());
}

void IRA_Dialog::DestroyControl(HWND hwndCtl) noexcept
{
	DestroyWindow(hwndCtl);
	hwndCtl = nullptr;
}

void IRA_Dialog::Activate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
}

void IRA_Dialog::ActivateApp(HWND hwnd, BOOL fActivate,
	DWORD dwThreadId)
{
}

void IRA_Dialog::CancelMode(HWND hwnd)
{
	EnableWindow(hwnd, FALSE);
}

#pragma region Color Control handlers
void IRA_Dialog::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
}
HBRUSH IRA_Dialog::OnCtlColorBtn(HWND hwnd, HDC hdc, HWND hwndChild,
	int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorDlg(HWND hwnd, HDC hdc, HWND hwndChild,
	int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorEdit(HWND hwnd, HDC hdc, HWND hwndChild,
	int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorListbox(HWND hwnd, HDC hdc, HWND hwndChild,
	int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorMsgbox(HWND hwnd, HDC hdc, HWND hwndChild,
	int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorScrollbar(HWND hwnd, HDC hdc, HWND hwndChild,
	int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorStatic(HWND hwnd, HDC hdc, HWND hwndChild,
	int type)
{
	return nullptr;
}
#pragma endregion


void IRA_Dialog::Enable(HWND hwnd, BOOL fEnable)
{
	EnableWindow(hwnd, fEnable);
}

BOOL IRA_Dialog::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return 0;
}

HFONT IRA_Dialog::GetFont(HWND hwnd)
{
	return GetWindowFont(hwnd);
}

INT IRA_Dialog::GetText(HWND hwnd, int cchTextMax, LPTSTR lpszText)
{
	return GetWindowText(hwnd, lpszText, cchTextMax);
}

INT IRA_Dialog::GetTextLength(HWND hwnd)
{
	return GetWindowTextLength(hwnd) + 1;
}

void IRA_Dialog::KillFocus(_UNUSED HWND hwnd, HWND hwndNewFocus)
{
	// Causes recursion here not sure how to define this for a default
	_CSTD SetFocus(hwndNewFocus);
}

BOOL IRA_Dialog::OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized)
{
	return FALSE;
}

void IRA_Dialog::OnQuit(_UNUSED HWND hwnd, _UNUSED int exitCode)
{
	FORWARD_WM_QUIT(hwnd, exitCode, PostMessage);
}

void IRA_Dialog::SetFocus(HWND hwnd, HWND hwndOldFocus)
{
	_CSTD SetFocus(hwnd);
}

void IRA_Dialog::SetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw)
{
	if(fRedraw) {
		SetWindowFont(hwndCtl, hfont, fRedraw);
	}
}

void IRA_Dialog::SetText(HWND hwnd, LPCTSTR lpszText)
{
	SetWindowText(hwnd, lpszText);
}

// HANDLE_WM_SHOWWINDOW
void IRA_Dialog::OnShowWindow(HWND hwnd, BOOL fShow, UINT status)
{
	// Regular show window seems to corrupt the color
	ShowWindowAsync(hwnd, fShow | status);
}

void IRA_Dialog::OnWindowPosChanged(_UNUSED HWND hwnd,
	_UNUSED  const LPWINDOWPOS lpwpos)
{

}

BOOL IRA_Dialog::OnWindowPosChanging(_UNUSED HWND hwnd, _UNUSED LPWINDOWPOS lpwpos)
{
	return FALSE;
}



#pragma region Button Event Handlers

// HANDLE_WM_CLOSE
void IRA_Dialog::Close(HWND hwnd)
{
	// Note: We aren't calling DefWindowProc or forwarding WM_CLOSE to it
	// because it will call DestroyWindow always.

	if (!bIsModal)
	{
		_CSTD DestroyWindow(hwnd);
		return;
	}
	if(hwnd)
		OnOK(hwnd);
}

void IRA_Dialog::OnOK(HWND hwnd)
{
	// setting to nullptr did not cause problems
	_CSTD EndDialog(hwnd, IDOK);
}

void IRA_Dialog::OnCancel(HWND hwnd)
{
	_CSTD EndDialog(hwnd, IDCANCEL);
}

void IRA_Dialog::Minimize(HWND hwnd)
{
	_CSTD CloseWindow(hwnd);
}
#pragma endregion


} // namespace ra
