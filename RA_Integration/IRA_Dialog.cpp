#include "IRA_Dialog.h"
#include "RA_Core.h"

namespace ra {
	/// <summary>
	/// The storage{CC2D43FA-BBC4-448A-9D0B-7B57ADF2655C}
	/// </summary>
	long IRA_Dialog::lStorage_{ 0 };

	// Note to self: Never call DefWindowProc or DefDlgProc for dialogs
	// DefWindowProc seems to mess up the focus

	IRA_Dialog::IRA_Dialog(int resId) :
		nResourceId_{ resId }
	{
		lStorage_ = reinterpret_cast<LRESULT>(this);
	}

	IRA_Dialog::~IRA_Dialog() noexcept
	{
		lStorage_ = 0;
	}

	INT_PTR IRA_Dialog::DoModal()
	{
		bIsModal_ = true;
		return DialogBox(g_hThisDLLInst, MAKEINTRESOURCE(nResourceId_), g_RAMainWnd, MsgQueue);
	}

	HWND IRA_Dialog::Create()
	{
		bIsModal_ = false;
		return CreateDialog(g_hThisDLLInst, MAKEINTRESOURCE(nResourceId_), g_RAMainWnd, MsgQueue);
	}

	INT_PTR IRA_Dialog::MsgQueue(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		IRA_Dialog* ira = reinterpret_cast<IRA_Dialog*>(lStorage_);

		switch ( uMsg )
		{
			HANDLE_MSG(hwnd, WM_ACTIVATE, ira->OnActivate);
			HANDLE_MSG(hwnd, WM_ACTIVATEAPP, ira->OnActivateApp);
			HANDLE_MSG(hwnd, WM_CANCELMODE, ira->OnCancelMode);
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
			HANDLE_MSG(hwnd, WM_DRAWITEM, ira->OnDrawItem);
			HANDLE_MSG(hwnd, WM_ENABLE, ira->Enable);
			HANDLE_MSG(hwnd, WM_ERASEBKGND, ira->OnEraseBkgnd);
			HANDLE_MSG(hwnd, WM_GETFONT, ira->GetFont);
			HANDLE_MSG(hwnd, WM_GETMINMAXINFO, ira->OnGetMinMaxInfo);
			HANDLE_MSG(hwnd, WM_GETTEXT, ira->GetText);
			HANDLE_MSG(hwnd, WM_GETTEXTLENGTH, ira->GetTextLength);
			HANDLE_MSG(hwnd, WM_INITDIALOG, ira->OnInitDialog);
			HANDLE_MSG(hwnd, WM_KEYDOWN, ira->OnKeyDown);
			HANDLE_MSG(hwnd, WM_KILLFOCUS, ira->KillFocus);
			HANDLE_MSG(hwnd, WM_LBUTTONUP, ira->OnLButtonUp);
			HANDLE_MSG(hwnd, WM_MEASUREITEM, ira->OnMeasureItem);
			HANDLE_MSG(hwnd, WM_MOUSEWHEEL, ira->OnMouseWheel);
			HANDLE_MSG(hwnd, WM_NCACTIVATE, ira->OnNCActivate);
			HANDLE_MSG(hwnd, WM_NCCREATE, ira->OnNCCreate);
			HANDLE_MSG(hwnd, WM_NCDESTROY, ira->OnNCDestroy);
			HANDLE_MSG(hwnd, WM_NOTIFY, ira->OnNotify);
			HANDLE_MSG(hwnd, WM_PAINT, ira->OnPaint);
			HANDLE_MSG(hwnd, WM_QUIT, ira->OnQuit);
			HANDLE_MSG(hwnd, WM_SETFOCUS, ira->SetFocus);
			HANDLE_MSG(hwnd, WM_SETFONT, ira->SetFont);
			HANDLE_MSG(hwnd, WM_SETTEXT, ira->SetText);
			HANDLE_MSG(hwnd, WM_SHOWWINDOW, ira->ShowWindow);
			HANDLE_MSG(hwnd, WM_SIZE, ira->OnSize);
			HANDLE_MSG(hwnd, WM_TIMER, ira->OnTimer);
			HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED, ira->OnWindowPosChanged);
			HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING, ira->OnWindowPosChanging);
		default:
			return ira->DialogProc(hwnd, uMsg, wParam, lParam);
		}
	}




	// Right-click and go to definition on each function if you want to know where
	// it came from (not the '// ')


	// HANDLE_WM_NCCREATE
	BOOL IRA_Dialog::OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
	{
		return 0;
	}

	// NB. Don't use DefDlgProc, it's useless and always throws expections

	// HANDLE_WM_NCDESTROY
	void IRA_Dialog::OnNCDestroy(HWND hwnd)
	{
		if ( lpCaption_ )
			DeleteCaption();
		if ( hwnd )
			Close(hwnd);
	}

	// HANDLE_WM_PAINT
	void IRA_Dialog::OnPaint(HWND hwnd)
	{
	}

	// HANDLE_WM_ERASEBKGND
	BOOL IRA_Dialog::OnEraseBkgnd(HWND hwnd, HDC hdc)
	{
		return 0;
	}

	// HANDLE_WM_CHAR
	void IRA_Dialog::OnChar(HWND hwnd, TCHAR ch, int cRepeat)
	{
	}

	// HANDLE_WM_KEYDOWN
	void IRA_Dialog::OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat,
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
	void IRA_Dialog::Destroy(HWND hwnd)
	{
		PostQuitMessage(0);
	}

	// HANDLE_WM_NOTIFY
	LRESULT IRA_Dialog::OnNotify(HWND hwnd, int idFrom, NMHDR* pnmhdr)
	{
		return 0;
	}

	// HANDLE_WM_TIMER
	void IRA_Dialog::OnTimer(HWND hwnd, UINT id)
	{
	}

	// HANDLE_WM_GETMINMAXINFO
	void IRA_Dialog::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
	{
	}

	// HANDLE_WM_DRAWITEM
	void IRA_Dialog::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
	{
	}

	// HANDLE_WM_MEASUREITEM
	void IRA_Dialog::OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
	{
	}

	// HANDLE_WM_SIZE
	void IRA_Dialog::OnSize(HWND hwnd, UINT state, int cx, int cy)
	{
	}

	void IRA_Dialog::OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
	{
	}

	void IRA_Dialog::OnActivateApp(HWND hwnd, BOOL fActivate, DWORD dwThreadId)
	{
	}

	void IRA_Dialog::OnCancelMode(HWND hwnd)
	{
		EnableWindow(hwnd, FALSE);
	}

	HBRUSH IRA_Dialog::OnCtlColorBtn(HWND hwnd, HDC hdc, HWND hwndChild, int type)
	{
		return nullptr;
	}

	HBRUSH IRA_Dialog::OnCtlColorDlg(HWND hwnd, HDC hdc, HWND hwndChild, int type)
	{
		return nullptr;
	}

	HBRUSH IRA_Dialog::OnCtlColorEdit(HWND hwnd, HDC hdc, HWND hwndChild, int type)
	{
		return nullptr;
	}

	HBRUSH IRA_Dialog::OnCtlColorListbox(HWND hwnd, HDC hdc, HWND hwndChild, int type)
	{
		return nullptr;
	}

	HBRUSH IRA_Dialog::OnCtlColorMsgbox(HWND hwnd, HDC hdc, HWND hwndChild, int type)
	{
		return nullptr;
	}

	HBRUSH IRA_Dialog::OnCtlColorScrollbar(HWND hwnd, HDC hdc, HWND hwndChild, int type)
	{
		return nullptr;
	}

	HBRUSH IRA_Dialog::OnCtlColorStatic(HWND hwnd, HDC hdc, HWND hwndChild, 
		int type)
	{
		return nullptr;
	}

	void IRA_Dialog::Enable(HWND hwnd, BOOL fEnable)
	{
		EnableWindow(hwnd, fEnable);
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
		return GetWindowTextLength(hwnd);
	}

	void IRA_Dialog::KillFocus(HWND hwnd, HWND hwndNewFocus)
	{
		// Causes recursion here not sure how to define this for a default
		SetFocus(hwndNewFocus);
	}

	BOOL IRA_Dialog::OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized)
	{
		return FALSE;
	}

	void IRA_Dialog::OnQuit(HWND hwnd, int exitCode)
	{

	}

	void IRA_Dialog::SetFocus(HWND hwnd, HWND hwndOldFocus)
	{
		_CSTD SetFocus(hwnd);
	}

	void IRA_Dialog::SetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw)
	{
		hFont = hfont;
		SetWindowFont(hwndCtl, hFont, fRedraw);
	}

	void IRA_Dialog::SetText(HWND hwnd, LPCTSTR lpszText)
	{
		SetWindowText(hwnd, lpszText);
	}

	void IRA_Dialog::ShowWindow(HWND hwnd, BOOL fShow, UINT status)
	{
		ShowWindowAsync(hwnd, fShow | status);
	}

	void IRA_Dialog::OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos)
	{
		
	}

	BOOL IRA_Dialog::OnWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos)
	{
		return FALSE;
	}



#pragma region Button Event Handlers

	// HANDLE_WM_CLOSE
	void IRA_Dialog::Close(HWND hwnd)
	{
		// Note: We aren't calling DefWindowProc or forwarding WM_CLOSE to it
		// because it will call DestroyWindow always.

		if ( bIsModal_ )
			OnOK(hwnd);
		else
			DestroyWindow(hwnd);
	}

	void IRA_Dialog::OnOK(HWND hwnd)
	{
		EndDialog(hwnd, IDOK);
	}

	void IRA_Dialog::OnCancel(HWND hwnd)
	{
		EndDialog(hwnd, IDCANCEL);
	}
	void IRA_Dialog::OnClose(HWND hwnd)
	{
		EndDialog(hwnd, IDCLOSE);
	}
	void IRA_Dialog::Minimize(HWND hwnd)
	{
		_CSTD CloseWindow(hwnd);
	}
#pragma endregion


} // namespace ra