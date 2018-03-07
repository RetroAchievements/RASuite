#ifndef RA_DIALOG_H
#define RA_DIALOG_H
#pragma once




/// <summary>
///		Base class for all RA Dialogs.
///		Not a "true" interface as not all dialogs share this functionality, but 
///		needed for the Message Queue.
/// </summary>
/// <remarks>
///		Functions that are always different are pure virtual, 
///		functions that are always the same are not virtual, 
///		functions that are ususally the same but can be different are just 
///		virtual.
/// </remarks>
class IRA_Dialog
{
public:
	IRA_Dialog(int resId, HWND parent = nullptr);

	/// <summary>
	/// A modal pops up
	/// </summary>
	/// <returns>Zero or non-zero</returns>
	/// <remarks>
	///		Almost always the same but made virtual since some dialogs have 
	///		extra functionality for this function.
	/// </remarks>
	virtual INT_PTR DoModal( );

	// Like DoModal but for modeless
	// From what we could tell it's always the same
	virtual HWND Create(); // CreateDialog

	static INT_PTR CALLBACK MsgQueue(HWND hDlg, UINT uMsg, WPARAM wParam, 
		LPARAM lParam);
	virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, 
		LPARAM lParam) = 0;
	virtual ~IRA_Dialog();


#pragma region Windows Message Handlers
	virtual BOOL OnInitDialog(HWND hDlg, HWND hDlgFocus, LPARAM lParam) = 0;
	virtual void OnCommand(HWND hDlg, int id, HWND hDlgCtl, 
		UINT codeNotify) = 0;

	/// <summary>
	/// Closes a dialog.
	/// NB: If the dialog is not a modal use DestroyWindow
	/// </summary>
	/// <param name="hDlg"></param>
	virtual void OnClose(HWND hDlg);

	/// <summary>
	/// Called when <c>WM_NCCREATE</c> is posted.
	/// </summary>
	/// <param name="hwnd">
	/// The handle to a window, for us it's invisible.
	/// </param>
	/// <param name="lpCreateStruct">
	/// The initial parameters used to initilize the window, maps to
	/// <c>lParam</c> in DialogProc.
	/// </param>
	/// <returns></returns>
	BOOL OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);

	void OnNCDestroy(HWND hwnd);
	virtual void OnPaint(HWND hDlg);
	virtual BOOL OnEraseBkgnd(HWND hDlg, HDC hdc);
	virtual void OnChar(HWND hDlg, TCHAR ch, int cRepeat);

	// Doesn't do anything here unless there's a repetivitve pattern
	// Mainly a template
	// HANDLE_WM_KEYDOWN
	/// <summary>
	/// Posted to the window with the keyboard focus when a nonsystem key is
	/// pressed. A nonsystem key is a key that is pressed when the ALT key is
	/// not pressed.
	/// </summary>
	/// <seealso cref="HANDLE_WM_KEYDOWN" />
	/// <param name="hDlg">Handle to the dialog</param>
	/// <param name="vk">
	/// Virtual KeyCode, maps to <c>wParam</c> of the <see cref="DialogProc" />.
	/// </param>
	/// <param name="fDown">Always <c>TRUE</c>.</param>
	/// <param name="cRepeat">
	/// Amount of times a keycode should repeat. Maps to the <c>LOWORD</c>
	/// portion of <c>lParam</c> in <see cref="DialogProc" />.
	/// </param>
	/// <param name="flags">
	/// Can include previous key-state or transition flags. Maps to the
	/// <c>HIWORD</c> portion of <c>lParam</c> in <see cref="DialogProc" />.
	/// </param>
	virtual void OnKeyDown(HWND hDlg, UINT vk, BOOL fDown, int cRepeat, 
		UINT flags);
	virtual void OnLButtonUp(HWND hDlg, int x, int y, UINT keyFlags);
	virtual void OnMouseWheel(HWND hDlg, int xPos, int yPos, int zDelta, 
		UINT fwKeys);
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
	virtual HWND MakeControl(int ControlId, HWND hDlg = nullptr);
protected:
	// These vars are here to prevent mishaps
	HWND hDlg{ nullptr };
	HWND hwnd_parent{ nullptr };
	HINSTANCE instance{ nullptr };
	int ResourceId{ 0 };
	std::string ResourceName{ "" };
	DLGPROC DlgProc{ nullptr };
};

#endif // !RA_Dialog_H


