#ifndef RA_DIALOG_H
#define RA_DIALOG_H
#pragma once

#include <wtypes.h>


/// <summary>
/// Base class for all RA Dialogs.
/// Not a "true" interface as not all dialogs share this functionality, but needed
/// for the Message Queue.
/// </summary>
/// <remarks>
///		Functions that are always different are pure virtual, 
///		functions that are always the same are not virtual, 
///		functions that are ususally the same but can be different are just virtual.
/// </remarks>
class IRA_Dialog
{
public:
	// I can't figure out another way to do this
	// basically just for those "four" dialogs in RA_Core you have to put false
	IRA_Dialog(int resId, bool isModal = true/*, HWND parent = nullptr*/);
	virtual INT_PTR DoModal(); // Most of the time doesn't need to be changed
	// for the modeless, always the same for us so no overloads
	// Unlike MFC this is not the handle to a parent window
	virtual HWND Create();
	static INT_PTR CALLBACK MsgQueue(HWND hwnd, UINT uMsg, WPARAM wParam, 
		LPARAM lParam);

	// Despite doing nothing this is necessary
	virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) = 0;
	virtual ~IRA_Dialog();

	// permissions based off of MFC
#pragma region Public Message Handlers
	virtual BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	virtual HFONT OnGetFont(HWND hwnd);
	void OnClose(HWND hwnd);
	void OnDestroy(HWND hwnd);
	virtual void OnEnable(HWND hwnd, BOOL fEnable = 1);
	virtual void OnSetFocus(HWND hwnd, HWND hwndOldFocus);
	virtual void OnSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw);
	virtual void OnShowWindow(HWND hwnd, BOOL fShow, UINT status);
#pragma endregion

protected:
#pragma region Protected Message Handlers
	virtual void OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
	virtual void OnActivateApp(HWND hwnd, BOOL fActivate, DWORD dwThreadId);
	virtual void OnCancelMode(HWND hwnd);
	virtual void OnChar(HWND hwnd, TCHAR ch, int cRepeat);
	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	virtual HBRUSH OnCtlColorBtn(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	virtual HBRUSH OnCtlColorDlg(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	virtual HBRUSH OnCtlColorEdit(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	virtual HBRUSH OnCtlColorListbox(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	virtual HBRUSH OnCtlColorMsgbox(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	virtual HBRUSH OnCtlColorScrollbar(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	virtual HBRUSH OnCtlColorStatic(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	virtual void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
	virtual BOOL OnEraseBkgnd(HWND hwnd, HDC hdc);
	virtual void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
	virtual INT GetText(HWND hwnd, int cchTextMax, LPTSTR lpszText);
	virtual INT OnGetTextLength(HWND hwnd);
	virtual void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual void OnKillFocus(HWND hwnd, HWND hwndNewFocus);
	virtual void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	virtual void OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem);
	virtual void OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys);
	virtual BOOL OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized);
	BOOL OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
	void OnNCDestroy(HWND hwnd);
	virtual LRESULT OnNotify(HWND hwnd, int idFrom, NMHDR* pnmhdr);
	virtual void OnPaint(HWND hwnd);
	virtual void OnQuit(HWND hwnd, int exitCode);
	virtual void SetText(HWND hwnd, LPCTSTR lpszText);
	virtual void OnSize(HWND hwnd, UINT state, int cx, int cy);
	virtual void OnTimer(HWND hwnd, UINT id);
	virtual void OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos);
	virtual BOOL OnWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos);
#pragma endregion

	virtual void OnOK(HWND hwnd);
	virtual void OnCancel(HWND hwnd);


	int ResourceId{ 0 };
	bool is_modal{ true };
private:
	static long storage; // this must never be accessed outside this class
};

// Function alias, MSFT functions are too long
#define GetWnd GetActiveWindow()

#endif // !RA_Dialog_H


