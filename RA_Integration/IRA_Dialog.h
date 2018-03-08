#ifndef RA_DIALOG_H
#define RA_DIALOG_H
#pragma once

#include <wtypes.h>
#include "RA_Core.h"

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
	IRA_Dialog(int resId, HWND parent = g_RAMainWnd) :
		ResourceId{ resId },
		hwnd_parent{ parent }
	{
	}
	virtual INT_PTR DoModal(); // Most of the time doesn't need to be changed
	static INT_PTR CALLBACK MsgQueue(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual ~IRA_Dialog();


#pragma region Windows Message Handlers
	virtual BOOL OnInitDialog(HWND hDlg, HWND hDlgFocus, LPARAM lParam) = 0;
	virtual void OnCommand(HWND hDlg, int id, HWND hDlgCtl, UINT codeNotify) = 0;
	void OnClose(HWND hDlg);
	BOOL OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
	void OnNCDestroy(HWND hwnd);
	virtual void OnPaint(HWND hDlg);
	virtual BOOL OnEraseBkgnd(HWND hDlg, HDC hdc);
	virtual void OnChar(HWND hDlg, TCHAR ch, int cRepeat);
	virtual void OnKeyDown(HWND hDlg, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual void OnLButtonUp(HWND hDlg, int x, int y, UINT keyFlags);
	virtual void OnMouseWheel(HWND hDlg, int xPos, int yPos, int zDelta, UINT fwKeys);
	void OnDestroy(HWND hDlg);
	virtual LRESULT OnNotify(HWND hDlg, int idFrom, NMHDR* pnmhdr);

	virtual void OnTimer(HWND hDlg, UINT id);
	virtual void OnGetMinMaxInfo(HWND hDlg, LPMINMAXINFO lpMinMaxInfo);
	virtual void OnDrawItem(HWND hDlg, const DRAWITEMSTRUCT* lpDrawItem);
	virtual void OnMeasureItem(HWND hDlg, MEASUREITEMSTRUCT* lpMeasureItem);
	virtual void OnSize(HWND hDlg, UINT state, int cx, int cy);

#pragma endregion


	/// <summary>
	/// Called when an OK button is pressed.
	/// </summary>
	/// <param name="hDlg">The h dialog.</param>
	virtual void OnOK(HWND hDlg);

	/// <summary>
	/// Called when a cancel button is pressed.
	/// </summary>
	/// <param name="hDlg">The h dialog.</param>
	virtual void OnCancel(HWND hDlg);

	// The HWND is usually the main but could be different
	virtual HWND MakeControl(int ControlId, HWND hDlg = g_RAMainWnd);
protected:
	HWND hwnd_parent{ nullptr };
	HINSTANCE instance{ g_hThisDLLInst };
	int ResourceId{ 0 };
	DLGPROC DlgProc{ MsgQueue };
};

#endif // !RA_Dialog_H


