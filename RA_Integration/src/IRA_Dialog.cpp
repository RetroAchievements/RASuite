#include "IRA_Dialog.h"
#include "RA_Core.h"

// Resources - Places I went to help solve problems
// https://stackoverflow.com/questions/31504685/bind-dialog-procedure-as-method-to-custom-class
// https://docs.microsoft.com/en-us/cpp/mfc/reference/mfc-classes









namespace ra {

IRA_Dialog* IRA_Dialog::ira{ null };

// Note to self: Never call DefWindowProc or DefDlgProc for dialogs
// Note to self: Never Post anything to Windows
// DefWindowProc seems to mess up the focus

IRA_Dialog::IRA_Dialog(int resId) :
	nResourceId{ resId }
{

}

IRA_Dialog::~IRA_Dialog() noexcept
{
	ira = null;
}

IRA_Dialog::IRA_Dialog(IRA_Dialog&& b) noexcept :
nResourceId{ b.nResourceId },
bIsModal{ b.bIsModal },
hFont{ b.hFont },
hIcon{ b.hIcon },
lpCaption{ b.lpCaption }
{
	b.OnNCDestroy(GetActiveWindow());
	b.bIsModal    = false;
	b.nResourceId = 0;
}

IRA_Dialog& IRA_Dialog::operator=(IRA_Dialog&& b) noexcept
{
	nResourceId = b.nResourceId;
	bIsModal    = b.bIsModal;
	hFont       = b.hFont;
	hIcon       = b.hIcon;
	lpCaption   = b.lpCaption;

	b.OnNCDestroy(GetActiveWindow());
	b.bIsModal    = false;
	b.nResourceId = 0;

	return *this;
}

// TODO: Make a class wrapper for RA_Core
INT_PTR IRA_Dialog::DoModal()
{
	bIsModal = true;

	auto result{
		DialogBoxParam(g_hThisDLLInst, MAKEINTRESOURCE(nResourceId), g_RAMainWnd,
		MsgQueue, reinterpret_cast<LPARAM>(this))
	};
	if (!result)
		throw std::system_error{ GetLastErrorCode(), GetLastErrorMsg() };

	return result;
}

bool IRA_Dialog::HasData() noexcept { return ira != null; }




HWND IRA_Dialog::Create()
{
	bIsModal = false;

	auto result{
		CreateDialogParam(g_hThisDLLInst, MAKEINTRESOURCE(nResourceId), g_RAMainWnd,
		MsgQueue, reinterpret_cast<LPARAM>(this))
	};
	if (!result)
		throw std::system_error{ GetLastErrorCode(), GetLastErrorMsg() };

	return result;
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
		// If we used SendMessage, this function would be called until the app crashes
		hIcon = FORWARD_WM_GETICON(hwnd, bBigIcon, PostMessage);
		return hIcon;
	}
	return Static_GetIcon(hwnd, hIcon);
}

HICON IRA_Dialog::SetIcon(HWND hwnd, BOOL bBigIcon, HICON hicon)
{
	if (!hicon)
		return FORWARD_WM_SETICON(hwnd, bBigIcon, hicon, PostMessage);

	return Static_SetIcon(hwnd, hicon);
}




void IRA_Dialog::button_check(HWND hwndCtl) noexcept {
	Button_SetCheck(hwndCtl, TRUE);
}

void IRA_Dialog::button_uncheck(HWND hwndCtl) noexcept {
	Button_SetCheck(hwndCtl, FALSE);
}

BOOL IRA_Dialog::Hide(HWND hwndCtl) noexcept
{
	return ShowWindowAsync(hwndCtl, SW_HIDE);
}

BOOL IRA_Dialog::Show(HWND hwndCtl) noexcept
{
	return ShowWindowAsync(hwndCtl, SW_SHOWNORMAL);
}

inline void IRA_Dialog::delete_font() noexcept {
	DeleteFont(hFont);
}

inline void IRA_Dialog::delete_icon() noexcept
{
	DestroyIcon(hIcon);
}

void IRA_Dialog::delete_caption() noexcept
{
	lpCaption = TEXT("");
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
	if (lpCaption)
		delete_caption();
	if (hwnd)
		DestroyWindow(hwnd); // could use a better name

	ira = null;
}

// HANDLE_WM_PAINT
void IRA_Dialog::OnPaint(_UNUSED HWND hwnd)
{
}

// HANDLE_WM_ERASEBKGND
BOOL IRA_Dialog::EraseBkgnd(_UNUSED HWND hwnd, _UNUSED HDC hdc)
{
	return 0;
}

// HANDLE_WM_CHAR
void IRA_Dialog::OnChar(_UNUSED HWND hwnd, _UNUSED TCHAR ch, _UNUSED int cRepeat)
{
}

// HANDLE_WM_KEYDOWN
void IRA_Dialog::OnKeyDown(_UNUSED HWND hwnd, _UNUSED UINT vk, _UNUSED BOOL fDown,
	_UNUSED int cRepeat, _UNUSED UINT flags)
{
}

// HANDLE_WM_LBUTTONUP
void IRA_Dialog::OnLButtonUp(_UNUSED HWND hwnd, _UNUSED int x, _UNUSED int y,
	_UNUSED UINT keyFlags)
{
}

// HANDLE_WM_MOUSEWHEEL
void IRA_Dialog::OnMouseWheel(_UNUSED HWND hwnd, _UNUSED int xPos, _UNUSED int yPos,
	_UNUSED int zDelta, _UNUSED UINT fwKeys)
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
LRESULT IRA_Dialog::OnNotify(_UNUSED HWND hwnd, _UNUSED int idFrom, _UNUSED NMHDR* pnmhdr)
{
	return 0;
}

// HANDLE_WM_TIMER
void IRA_Dialog::OnTimer(HWND hwnd, UINT id)
{
	// In derived dialogs ether call OnTimer or SendMessage with this macro
	FORWARD_WM_TIMER(hwnd, id, PostMessage);
}

// HANDLE_WM_GETMINMAXINFO
void IRA_Dialog::GetMinMaxInfo(_UNUSED HWND hwnd, _UNUSED LPMINMAXINFO lpMinMaxInfo)
{
}

// HANDLE_WM_DRAWITEM
void IRA_Dialog::DrawItem(_UNUSED HWND hwnd, _UNUSED const DRAWITEMSTRUCT* lpDrawItem)
{
}

// HANDLE_WM_MEASUREITEM
void IRA_Dialog::MeasureItem(_UNUSED HWND hwnd, _UNUSED MEASUREITEMSTRUCT* lpMeasureItem)
{
}

void IRA_Dialog::Move(HWND hwnd, _UNUSED int x, _UNUSED int y)
{
	RememberWindowPosition(hwnd, lpCaption);
} // end function Move

// HANDLE_WM_SIZE
void IRA_Dialog::OnSize(HWND hwnd, _UNUSED UINT state, _UNUSED int cx,
	_UNUSED int cy)
{
	// doesn't make a lot of sense but I've noticed a lot windows doing this
	RememberWindowSize(hwnd, lpCaption);
}

void IRA_Dialog::DestroyControl(HWND hwndCtl) noexcept
{
	DestroyWindow(hwndCtl);
	hwndCtl = nullptr;
}

void IRA_Dialog::Activate(_UNUSED HWND hwnd, _UNUSED  UINT state,
	_UNUSED HWND hwndActDeact, _UNUSED BOOL fMinimized)
{
}

void IRA_Dialog::ActivateApp(_UNUSED HWND hwnd, _UNUSED BOOL fActivate,
	_UNUSED DWORD dwThreadId)
{
}

void IRA_Dialog::CancelMode(HWND hwnd)
{
	EnableWindow(hwnd, FALSE);
}

#pragma region Color Control handlers
HBRUSH IRA_Dialog::OnCtlColorBtn(_UNUSED HWND hwnd, _UNUSED HDC hdc,
	_UNUSED HWND hwndChild, _UNUSED int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorDlg(_UNUSED HWND hwnd, _UNUSED HDC hdc,
	_UNUSED HWND hwndChild, _UNUSED int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorEdit(_UNUSED HWND hwnd, _UNUSED HDC hdc,
	_UNUSED HWND hwndChild, _UNUSED int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorListbox(_UNUSED HWND hwnd, _UNUSED HDC hdc,
	_UNUSED HWND hwndChild, _UNUSED int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorMsgbox(_UNUSED HWND hwnd, _UNUSED HDC hdc,
	_UNUSED HWND hwndChild, _UNUSED int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorScrollbar(_UNUSED HWND hwnd, _UNUSED HDC hdc,
	_UNUSED HWND hwndChild, _UNUSED int type)
{
	return nullptr;
}

HBRUSH IRA_Dialog::OnCtlColorStatic(_UNUSED HWND hwnd, _UNUSED HDC hdc,
	_UNUSED HWND hwndChild, _UNUSED int type)
{
	return nullptr;
}
#pragma endregion


void IRA_Dialog::Enable(HWND hwnd, BOOL fEnable)
{
	EnableWindow(hwnd, fEnable);
}

HWND IRA_Dialog::NextDlgCtl(HWND hwnd, HWND hwndSetFocus, BOOL fNext)
{
	return FORWARD_WM_NEXTDLGCTL(hwnd, hwndSetFocus, fNext, PostMessage);
}

// HANDLE_WM_INITDIALOG
BOOL IRA_Dialog::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowLongPtr(hwnd, DWLP_USER, lParam);
	auto my_ptr{ reinterpret_cast<IRA_Dialog*>(GetWindowLongPtr(hwnd, DWLP_USER)) };
	if (my_ptr)
		return my_ptr->OnInitDialog(hwnd, hwndFocus, lParam);
	return FALSE;
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
	SetFocus(hwndNewFocus);
}

BOOL IRA_Dialog::OnNCActivate(_UNUSED HWND hwnd, _UNUSED BOOL fActive,
	_UNUSED HWND hwndActDeact, _UNUSED BOOL fMinimized)
{
	return FALSE;
}

void IRA_Dialog::OnQuit(_UNUSED HWND hwnd, _UNUSED int exitCode)
{
	FORWARD_WM_QUIT(hwnd, exitCode, PostMessage);
}

void IRA_Dialog::SetFocus(HWND hwnd, _UNUSED HWND hwndOldFocus)
{
	_CSTD SetFocus(hwnd);
}

void IRA_Dialog::SetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw)
{
	// This messed up the colors and did some whacky recursion without the if
	// Damn it dude
	if(fRedraw)
		SetWindowFont(hwndCtl, hfont, fRedraw);
}

void IRA_Dialog::SetText(HWND hwnd, LPCTSTR lpszText)
{

	if (lpszText != null) {
		SetWindowText(hwnd, lpszText);
	}
}

// HANDLE_WM_SHOWWINDOW
void IRA_Dialog::OnShowWindow(HWND hwnd, BOOL fShow, UINT status)
{
	// Regular show window seems to corrupt the color
	// Modeless dialogs will call this after Show()
	ShowWindowAsync(hwnd, static_cast<int>(fShow | status));
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
		// Don't nullify here, nullify in WM_NCDESTROY!
		DestroyWindow(hwnd);
        OnNCDestroy(hwnd);
		return;
	}
	if (hwnd)
	{
		OnOK(hwnd);
	}
}

void IRA_Dialog::OnOK(HWND hwnd)
{
	// setting to nullptr DID cause problems, probably windows does it for us
	// Do the nullifying in OnNcDestroy
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


// Crossing my damn fingers....
// If it worked for other functions it should work here
INT_PTR IRA_Dialog::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // I always need to see what the message is to debug
	current_msg = uMsg;

    // Lol I thought I did something wrong but the internet keeps on disconnecting.........

	// mainly an experiment, stuff that is never overriden should be processed here
	// Holy shit it actually came here!
	// My bad OnInitDialog DOES have to be here
	switch (uMsg)
	{
		// Some stuff will not work here
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(hwnd, WM_ACTIVATE, Activate);
		HANDLE_MSG(hwnd, WM_ACTIVATEAPP, ActivateApp);
		HANDLE_MSG(hwnd, WM_CANCELMODE, CancelMode);
		HANDLE_MSG(hwnd, WM_CLOSE, Close);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand); // I'm a dumb fuck
		HANDLE_MSG(hwnd, WM_CTLCOLORBTN, OnCtlColorBtn);
		HANDLE_MSG(hwnd, WM_CTLCOLORDLG, OnCtlColorDlg);
		HANDLE_MSG(hwnd, WM_CTLCOLOREDIT, OnCtlColorEdit);
		HANDLE_MSG(hwnd, WM_CTLCOLORLISTBOX, OnCtlColorListbox);
		HANDLE_MSG(hwnd, WM_CTLCOLORMSGBOX, OnCtlColorMsgbox);
		HANDLE_MSG(hwnd, WM_CTLCOLORSCROLLBAR, OnCtlColorScrollbar);
		HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC, OnCtlColorStatic);
		HANDLE_MSG(hwnd, WM_DESTROY, Destroy);
		HANDLE_MSG(hwnd, WM_DRAWITEM, DrawItem);
		HANDLE_MSG(hwnd, WM_ENABLE, Enable);
		HANDLE_MSG(hwnd, WM_ERASEBKGND, EraseBkgnd);
		HANDLE_MSG(hwnd, WM_GETFONT, GetFont);
		HANDLE_MSG(hwnd, WM_GETICON, GetIcon); // custom
		HANDLE_MSG(hwnd, WM_GETMINMAXINFO, GetMinMaxInfo);
		HANDLE_MSG(hwnd, WM_GETTEXT, GetText);
		HANDLE_MSG(hwnd, WM_GETTEXTLENGTH, GetTextLength);
		HANDLE_MSG(hwnd, WM_KILLFOCUS, KillFocus);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, OnLButtonUp);
		HANDLE_MSG(hwnd, WM_MEASUREITEM, MeasureItem);
		HANDLE_MSG(hwnd, WM_MOUSEWHEEL, OnMouseWheel);
		HANDLE_MSG(hwnd, WM_MOVE, Move);
		HANDLE_MSG(hwnd, WM_NCACTIVATE, OnNCActivate);
		HANDLE_MSG(hwnd, WM_NCCREATE, OnNCCreate);
		HANDLE_MSG(hwnd, WM_NCDESTROY, OnNCDestroy);
		HANDLE_MSG(hwnd, WM_NCPAINT, OnNCPaint);
		HANDLE_MSG(hwnd, WM_NEXTDLGCTL, NextDlgCtl);
		HANDLE_MSG(hwnd, WM_NOTIFY, OnNotify);
		HANDLE_MSG(hwnd, WM_QUIT, OnQuit);
		HANDLE_MSG(hwnd, WM_SETFOCUS, SetFocus);
		HANDLE_MSG(hwnd, WM_SETFONT, SetFont);
		HANDLE_MSG(hwnd, WM_SETICON, SetIcon); // custom
		HANDLE_MSG(hwnd, WM_SETTEXT, SetText);
		HANDLE_MSG(hwnd, WM_SHOWWINDOW, OnShowWindow);
		HANDLE_MSG(hwnd, WM_SIZE, OnSize);
		HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
		HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED, OnWindowPosChanged);
		HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING, OnWindowPosChanging);
		DO_DEFAULT;
	}
}


// OK, to make this work for everything only WM_INITDIALOG can be here everything else
// needs to be in DialogProc
INT_PTR IRA_Dialog::MsgQueue(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/*
	0    - WM_NULL,
	85   - WM_NOTIFYFORMAT,
	295  - WM_CHANGEUISTATE,
	296  - WM_UPDATEUISTATE,
	297  - WM_QUERYUISTATE,
	641  - WM_IME_SETCONTEXT,
	642  - WM_IME_NOTIFY,
	792  - WM_PRINTCLIENT,
	799  - WM_DWMNCRENDERINGCHANGED,
	*/

	// We need to filter out bad messages and make our own message loop
	using MessageFilter = std::array<int, 1000>;
	constexpr MessageFilter no_crackers{
		WM_NULL, WM_NOTIFYFORMAT, WM_CHANGEUISTATE, WM_UPDATEUISTATE, WM_QUERYUISTATE,
		WM_IME_SETCONTEXT, WM_IME_NOTIFY, WM_PRINTCLIENT, WM_DWMNCRENDERINGCHANGED,
	};

	// might sloww it down though
	for (auto& access_violations : no_crackers)
	{
		if (uMsg == access_violations)
			return 0;
	}

	/*
	6   - WM_ACTIVATE
	7   - WM_SETFOCUS (dwp)
	8   - WM_KILLFOCUS
	15  - WM_PAINT
	20  - WM_ERASEBKGND
	24  - WM_SHOWWINDOW (dwp)
	28  - WM_ACTIVATEAPP
	40  - WM_NEXTDLGCTL
	48  - WM_SETFONT
	49  - WM_GETFONT
	70  - WM_WINDOWPOSCHANGING (dwp)
	71  - WM_WINDOWPOSCHANGED (dwp)
	78  - WM_NOTIFY
	127 - WM_GETICON
	128 - WM_SETICON
	133 - WM_NCPAINT
	134 - WM_NCACTIVATE (dwp)
	272 - WM_INITDIALOG
	273 - WM_COMMAND
	307 - WM_CTLCOLOREDIT
	308 - WM_CTLCOLORLISTBOX
	309 - WM_CTLCOLORBTN - buttons appear
	310 - WM_CTLCOLORDLG - Modal Dialogs appear
	312 - WM_CTLCOLORSTATIC
	*/

	constexpr MessageFilter our_messages{
		WM_ACTIVATE, WM_SETFOCUS, WM_KILLFOCUS, WM_PAINT, WM_ERASEBKGND, WM_SHOWWINDOW,
		WM_ACTIVATEAPP, WM_NEXTDLGCTL, WM_SETFONT, WM_GETFONT, WM_WINDOWPOSCHANGING,
		WM_WINDOWPOSCHANGED, WM_NOTIFY, WM_GETICON, WM_NCPAINT, WM_NCACTIVATE,
		WM_INITDIALOG, WM_COMMAND, WM_CTLCOLOREDIT, WM_CTLCOLORLISTBOX, WM_CTLCOLORBTN,
		WM_CTLCOLORDLG,	WM_CTLCOLORSTATIC, WM_GETICON
	};

	// can't figure it out otherwise
	
	if (uMsg == WM_INITDIALOG)
	{
		
		// why is it null!
		// whats the difference between gamelib and achievement set?
		// it's null for gamelib but not null for achievementset?

		 ira = _RA OnInitDialog(hwnd, reinterpret_cast<HWND>(wParam), lParam);
	}

	if (ira) {
		return ira->DialogProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;







}

// Lets see if a global callback will work
_Use_decl_annotations_
IRA_Dialog* CALLBACK OnInitDialog(_In_ HWND hwnd, _In_ HWND hwndFocus, _In_ LPARAM lParam)
{
	SetWindowLongPtr(hwnd, DWLP_USER, lParam);
	IRA_Dialog::ira = reinterpret_cast<IRA_Dialog*>(GetWindowLongPtr(hwnd, DWLP_USER));
	if (IRA_Dialog::ira)
		IRA_Dialog::ira->OnInitDialog(hwnd, hwndFocus, lParam);

	return IRA_Dialog::ira;
}

} // namespace ra
