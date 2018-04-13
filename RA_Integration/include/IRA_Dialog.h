#ifndef RA_DIALOG_H
#define RA_DIALOG_H
#pragma once

// Might blow up files but w/e
#include "RA_Defs.h"

// Gonna see if it'll work
#include "resource_handles.h"

// A lot of the documentation is derived from here:
// https://msdn.microsoft.com/en-us/library/windows/desktop/ff818516.aspx

// Use <inheritdoc /> for derived classes' overrides



#pragma region Custom Message Crackers
// This is an experiement so-to-speak

// These might exist in newer sdks
// According to Microsoft:
// wParam: type of icon; i.e. ICON_SMALL or ICON_BIG (0,1)  which is an BOOL
// lParam: is a DPI is also an int. SM_CXICON which is an int
//		   windows 10 sdk has easier functions for dpi and might have these 
//         message crackers
// return type is HFONT; Access public
#ifndef HANDLE_WM_GETICON
// Gonna do it the C way for now
// HICON GetIcon(HWND hwnd, BOOL bBigIcon) // DPI doesn't work for this version i guess
// MFC version: HICON GetIcon(BOOL bBigIcon) const;
#define HANDLE_WM_GETICON(hwnd, wParam, lParam, fn) \
	((LRESULT)(fn)((hwnd), (BOOL)(wParam)))
#define FORWARD_WM_GETICON(hwnd, bBigIcon, fn) \
	(HICON)(fn)((hwnd), WM_GETICON, (WPARAM)(BOOL)(bBigIcon), 0L)
#endif // !HANDLE_WM_GETICON

// ICON_BIG

// WM_SETICON doesn't have one in this version either
// wParam: the type of icon, ICON_SMALL or ICON_BIG can be another one; BOOL (0,1,2)
// lParam: the handle to a font (HFONT)
// return type is LRESULT, guess it has too
// MFC version: HICON SetIcon( HICON hIcon, BOOL bBigIcon); Access public
#ifndef HANDLE_WM_SETICON
// HICON SetIcon(HWND hwnd, BOOL bBigIcon, HICON hicon)
#define HANDLE_WM_SETICON(hwnd, wParam, lParam, fn) \
	((LRESULT)(fn)((hwnd), (BOOL)(wParam), (HICON)(lParam)))
#define FORWARD_WM_SETICON(hwnd, bBigIcon, hicon, fn) \
	(HICON)(fn)((hwnd), WM_SETICON, (WPARAM)(BOOL)(bBigIcon), (LPARAM)(HICON)(hicon))
#endif // !HANDLE_WM_SETICON  

// something
#define DO_DEFAULT default: return 0


#pragma endregion

// Alright, at best it seems some of these have to be pure virtual or just not in here
// Like Create, OnInitDialog, OnCommand, OnNCDestroy

namespace ra {

/// <summary>
///   Base class for all RA Dialogs. Not a "true" interface as not all dialogs
///   share this functionality, but needed for the Message Queue.
///
///   For default handlers be very careful, in most cases you must implement a
///   handler for messages you wish to process via override. In very few cases
///   you can use <seealso cref="PostMessage" /> to post to the system's
///   message queue instead of our message queue. A queued message normally is
///   identified by something that affects a window directly such as
///   <c>WM_TIMER</c> or <c>WM_GETFONT</c>. Recursion will happen if you
///   forward the message to the wrong queue. SendMessage is for "non-queued"
///   messages, meaning the system will not handle it.
/// </summary>
/// <remarks>
///   A lot of the comments will refer to an <c>HWND</c> as either an active or
///   a specified window. This is because the handle getting passed around in
///   the queue is always active but controls and parent windows aren't.
/// 
///   Parameters passed here are from Windows, you may change them if desired.
/// </remarks>
class IRA_Dialog
{
	//using dialog_proc = std::function<INT_PTR(CALLBACK*)(HWND, UINT, WPARAM, LPARAM)>;
	friend class IRA_WndClass;
	friend class window_h;
	// this is for saving so to speak

public:
	// sorry it has to be done
	/// <summary>
	///   Initializes a new instance of the <see cref="IRA_Dialog" /> class.
	/// </summary>
	/// <param name="resId">The resource identifier.</param>
	/// <remarks>
	///   The parent HWND in the comments is there just in-case we want to
	///   spawn out a child window from a dialog. A good example would be in
	///   <seealso cref="RA_Dlg_Achievement" />.
	/// </remarks>
	IRA_Dialog(int resId);

	// don't set any hwnd to null until wm_ncdestroy
	/// <summary>
	///   Finalizes an instance of the <see cref="IRA_Dialog" /> class.
	/// </summary>
	/// <returns>
	///   Should not return anything, if it does an exception was thrown and
	///   needs to be taken care of.
	/// </returns>
	/// <remarks>
	///   For derived classes, free resources from controls/fonts/captions/etc.
	///   using <see cref="OnNCDestory" />. You can set them to <c>nullptr</c>
	///   if prefered, but shouldn't be necessary. <c>storage</c> must be set
	///   to <c>0</c> here.
	/// </remarks>
	virtual ~IRA_Dialog() noexcept;

	// Alright this is really weird, the wrong WM_INITDIALOG is getting called,
	// maybe move semantics would help
	IRA_Dialog(const IRA_Dialog&) = delete;
	IRA_Dialog& operator=(const IRA_Dialog&) = delete;
	IRA_Dialog(IRA_Dialog&& b) noexcept;
	IRA_Dialog& operator=(IRA_Dialog&& b) noexcept;

	/// <summary>
	/// Displays a modal.
	/// </summary>
	/// <returns></returns>
	virtual INT_PTR DoModal();

	// These have to be pure virtual since this dialog is techinically a window manager
	virtual HWND GetWindow() const noexcept = 0;

	bool HasData() noexcept;


	// maybe it needs to be virtual?
	// It doesn't always have to be implemented but it can't be used here
    // I'm not sure if we need this to be virtual anymore
	_NODISCARD virtual HWND Create();
	// Had to revise it a little
	_NORETURN void PostNcDestroy() noexcept;

	// permissions based off of MFC
#pragma region Public Message Handlers

// queued, non virtual, goes to MsgQueue, with exception to WM_INITDIALOG
#pragma region Posted Messages
// I really doubt we need to change these
// Need to see if Windows will under stand my cracker
// Ok they work, bBigIcon is usually 2 (ICON_SMALL2)
	_NODISCARD HICON GetIcon(HWND hwnd, BOOL bBigIcon);
	_NODISCARD HICON SetIcon(HWND hwnd, BOOL bBigIcon, HICON hicon);
	_NODISCARD HFONT GetFont(HWND hwnd);
	/// <summary>
	///   Posts a WM_QUIT message.
	/// </summary>
	/// <param name="hwnd">The handle to the active window.</param>
	_NORETURN void Destroy(_UNUSED HWND hwnd);
	_NORETURN void Enable(HWND hwnd, BOOL fEnable = TRUE);
	/// <summary>
	/// Sets the focus.
	/// </summary>
	/// <param name="hwnd">The HWND.</param>
	/// <param name="hwndOldFocus">The HWND old focus.</param>
	_NORETURN void SetFocus(HWND hwnd, _UNUSED HWND hwndOldFocus = nullptr);
	_NORETURN void SetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw);
	_NORETURN void OnShowWindow(HWND hwnd, BOOL fShow, UINT status);


	/// <summary>
	///   Sets the focus to a different control in the dialog box.
	/// </summary>
	/// <param name="hwnd">The handle to the active window.</param>
	/// <param name="hwndSetFocus">
	///   If <paramref name="fNext" /> is <c>TRUE</c>, this parameter identifies
	///   the control that receives the focus. If <paramref name="fNext" /> is
	///   <c>FALSE</c>, this parameter indicates whether the next or previous
	///   control with the <c>WS_TABSTOP</c> style receives the focus. If
	///   <paramref name="hwndSetFocus" /> is <c>nullptr</c>, the next control
	///   receives the focus; otherwise, the previous control with the
	///   <c>WS_TABSTOP</c> style receives the focus.
	/// </param>
	/// <param name="fNext">
	///   The low-order word of <c>wParam</c> indicates how the system uses
	///   <paramref name="hwndSetFocus" />. If the low-order word is <c>TRUE
	///   </c>, <paramref name="hwndSetFocus" /> is a handle associated with the
	///   control that receives the focus; otherwise,
	///   <paramref name="hwndSetFocus" /> is a flag that indicates whether the
	///   next or previous control with the <c>WS_TABSTOP</c> style receives the
	///   focus.
	/// </param>
	/// <returns>
	///   Returns the handle to a control depending on <paramref name="fNext" />.
	/// </returns>
	/// <remarks>
	///   <para>
	///     This message performs additional dialog box management operations
	///     beyond those performed by the SetFocus function <c>WM_NEXTDLGCTL</c>
	///     updates the default pushbutton border, sets the default control
	///     identifier, and automatically selects the text of an edit control (if
	///     the target window is an edit control).
	///   </para>
	///   <para>
	///     Do not use the <see cref="SendMessage" /> function to send a <c>
	///     WM_NEXTDLGCTL</c> message if your application will concurrently
	///     process other messages that set the focus. Use the
	///     <see cref="PostMessage" /> function instead.
	///   </para>
	/// </remarks>
	_NODISCARD HWND NextDlgCtl(HWND hwnd, HWND hwndSetFocus, BOOL fNext);
#pragma endregion

	// Nonqueued, virtual, goes to the approriate Window/DialogProc
#pragma region Sent Messages
	/// <summary>Called when <c>WM_INITDIALOG</c> is sent.</summary>
	/// <param name="hwnd">The active window.</param>
	/// <param name="hwndFocus">The window to get focus.</param>
	/// <param name="lParam">
	///   The message information sent from <see cref="DoModal" /> or 
	///   <see cref="Create" />.
	/// </param>
	/// <returns>
	///   <c>TRUE</c> if you want the system to set focus, otherwise
	///   <c>FALSE</c>.
	/// </returns>
	_NODISCARD virtual BOOL OnInitDialog(HWND hwnd, HWND hwndFocus,
		LPARAM lParam) = 0;
	/// <summary>
	///   Destroys a window/control if not a modal dialog, otherwise it just
	///   ends it.
	/// </summary>
	/// <param name="hwnd">The handle to active window/control.</param>
	/// <remarks>
	///   This is not to be confused with <see cref="OnClose" /> when
	///   <c>IDCLOSE</c> is encountered.
	/// </remarks>
	virtual void Close(HWND hwnd); // this really shouldn't be virtual but there's some weird cases
#pragma endregion
















#pragma endregion

protected /*shared*/:
	// queued, non virtual, goes to MsgQueue, with exception to WM_INITDIALOG
#pragma region Posted Messages
	_NORETURN void Activate(_UNUSED HWND hwnd, _UNUSED UINT state,
		_UNUSED HWND hwndActDeact, _UNUSED BOOL fMinimized);
	_NORETURN void ActivateApp(_UNUSED HWND hwnd, _UNUSED BOOL fActivate,
		_UNUSED DWORD dwThreadId);
	_NORETURN void CancelMode(HWND hwnd);
	// not a message handler but noticed some weird shit
	// It is techinically Posted
	_NORETURN void DestroyControl(HWND hwndCtl) noexcept;

#pragma region ColorCtl
	_NODISCARD HBRUSH OnCtlColorBtn(_UNUSED HWND hwnd, _UNUSED HDC hdc,
		_UNUSED HWND hwndChild, _UNUSED int type);
	_NODISCARD HBRUSH OnCtlColorDlg(_UNUSED HWND hwnd, _UNUSED HDC hdc,
		_UNUSED HWND hwndChild, _UNUSED int type);
	_NODISCARD HBRUSH OnCtlColorEdit(_UNUSED HWND hwnd, _UNUSED HDC hdc,
		_UNUSED HWND hwndChild, _UNUSED int type);
	_NODISCARD HBRUSH OnCtlColorListbox(_UNUSED HWND hwnd, _UNUSED HDC hdc,
		_UNUSED HWND hwndChild, _UNUSED int type);
	_NODISCARD HBRUSH OnCtlColorMsgbox(_UNUSED HWND hwnd, _UNUSED HDC hdc,
		_UNUSED HWND hwndChild, _UNUSED int type);
	_NODISCARD HBRUSH OnCtlColorScrollbar(_UNUSED HWND hwnd, _UNUSED HDC hdc,
		_UNUSED HWND hwndChild, _UNUSED int type);
	_NODISCARD HBRUSH OnCtlColorStatic(_UNUSED HWND hwnd, _UNUSED HDC hdc,
		_UNUSED HWND hwndChild, _UNUSED int type);
#pragma endregion

#pragma endregion





#pragma region Protected Message Handlers

	_NORETURN virtual void OnChar(_UNUSED HWND hwnd, _UNUSED TCHAR ch,
		_UNUSED int cRepeat);



	/// <summary>
	/// Called after <c>WM_COMMAND</c> is sent.
	/// </summary>
	/// <param name="hwnd">The active window.</param>
	/// <param name="id">A control ID.</param>
	/// <param name="hwndCtl">A handle to a control.</param>
	/// <param name="codeNotify">A notification code.</param>
	_NORETURN virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT
		codeNotify) = 0;


	virtual void DrawItem(_UNUSED HWND hwnd,
		_UNUSED const DRAWITEMSTRUCT* lpDrawItem);
	_NODISCARD BOOL EraseBkgnd(_UNUSED HWND hwnd, _UNUSED HDC hdc);
	virtual void GetMinMaxInfo(_UNUSED HWND hwnd,
		_UNUSED LPMINMAXINFO lpMinMaxInfo);
	INT GetText(HWND hwnd, int cchTextMax, LPTSTR lpszText);
	_NODISCARD virtual INT GetTextLength(HWND hwnd);


	/// <summary>Called when <c>WM_KEYDOWN</c> is posted.</summary>
	/// <param name="hwnd">The handle to the active window.</param>
	/// <param name="vk">A non-system virtual key-code.</param>
	/// <param name="fDown">Key is down, this is always <c>TRUE</c>.</param>
	/// <param name="cRepeat">The repeat count.</param>
	/// <param name="flags">
	///   The scan and context codes. The extended-key, previous key-state, and
	///   transition-state flags.
	/// </param>
	/// <remarks>
	///   <paramref name="fDown" /> is marked as unused only because it is not
	///   used in-code, but the system always passes a value of <c>TRUE</c> for
	///   it.
	/// </remarks>
	_NORETURN virtual void OnKeyDown(_UNUSED HWND hwnd, _UNUSED UINT vk,
		_UNUSED BOOL fDown, _UNUSED int cRepeat, _UNUSED UINT flags);


	/// <summary>
	///   Kills the focus for <paramref name="hwnd" /> and sets the focus
	///   for <paramref name="hwndNewFocus" />.
	/// </summary>
	/// <param name="hwnd">The HWND.</param>
	/// <param name="hwndNewFocus">The HWND new focus.</param>
	_NORETURN void KillFocus(_UNUSED HWND hwnd, HWND hwndNewFocus);
	_NORETURN virtual void OnLButtonUp(_UNUSED HWND hwnd, _UNUSED int x,
		_UNUSED int y, _UNUSED UINT keyFlags);
	_NORETURN virtual void MeasureItem(_UNUSED HWND hwnd,
		_UNUSED MEASUREITEMSTRUCT* lpMeasureItem);
	_NORETURN void Move(HWND hwnd, _UNUSED int x, _UNUSED int y);
	_NORETURN virtual void OnMouseWheel(_UNUSED HWND hwnd, _UNUSED int xPos,
		_UNUSED int yPos, _UNUSED int zDelta, _UNUSED UINT fwKeys);
	_NODISCARD virtual BOOL OnNCActivate(_UNUSED HWND hwnd, _UNUSED BOOL fActive,
		_UNUSED HWND hwndActDeact, _UNUSED BOOL fMinimized);


	/// <summary>
	///   Called when <c>WM_NCCREATE</c> is posted. The only instance this may
	///   need to be used in <seealso cref="MemoryViewControl" />.
	/// </summary>
	/// <param name="hwnd">The handle for a new window to be created.</param>
	/// <param name="lpCreateStruct">
	///   Contains inital parameters to construct a new window.
	/// </param>
	/// <returns></returns>
	_NODISCARD virtual BOOL OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);


	/// <summary>
	///   Called when <c>WM_NCDESTROY</c> is posted. Use this function to
	///   destroy any controls and non-shared resource.
	/// </summary>
	/// <param name="hwnd">
	///   A handle to a window, usually a control window. Usually this
	///   specific parameter will not be used.
	/// </param>
	_NORETURN virtual void OnNCDestroy(HWND hwnd) = 0;

	// newly discovered message
	_NORETURN void OnNCPaint(HWND hwnd, HRGN hrgn);
	_NODISCARD virtual LRESULT OnNotify(_UNUSED HWND hwnd, _UNUSED int idFrom,
		_UNUSED NMHDR* pnmhdr);
	_NORETURN virtual void OnPaint(_UNUSED HWND hwnd);
	_NORETURN void OnQuit(_UNUSED HWND hwnd, _UNUSED int exitCode);


	/// <summary>
	///   Sets the text for the specified window, usually a caption or control.
	/// </summary>
	/// <param name="hwnd">The handle to the specified window.</param>
	/// <param name="lpszText">The text to be set.</param>
	_NORETURN void SetText(HWND hwnd, LPCTSTR lpszText);


	/// <summary>Called when <c>WM_SIZE</c> is posted.</summary>
	/// <param name="hwnd">The HWND.</param>
	/// <param name="state">The state.</param>
	/// <param name="cx">The cx.</param>
	/// <param name="cy">The cy.</param>
	/// <remarks>
	///   The function called here will not use the parameters passed by
	///   Windows, override if you wish to do something special with resizing.
	/// </remarks>
	_NORETURN void OnSize(HWND hwnd, _UNUSED UINT state, _UNUSED int cx,
		_UNUSED int cy);
	// This is a queued message, meaning you used PostMessage
	_NORETURN virtual void OnTimer(HWND hwnd, UINT id);
	_NORETURN void OnWindowPosChanged(_UNUSED HWND hwnd,
		_UNUSED const LPWINDOWPOS lpwpos);
	_NODISCARD BOOL OnWindowPosChanging(_UNUSED HWND hwnd,
		_UNUSED LPWINDOWPOS lpwpos);
#pragma endregion

#pragma region Button stuff
	/// <summary>
/// Raises the OK event. Should be called when an OK button is pressed.
/// </summary>
/// <param name="hwnd">The handled to the active window.</param>
	_NORETURN virtual void OnOK(HWND hwnd);


	/// <summary>
	/// Raises the Cancel event. Should be called when a cancel button is pressed.
	/// </summary>
	/// <param name="hwnd">The handled to the active window.</param>
	_NORETURN virtual void OnCancel(HWND hwnd);

	/// <summary>Called when a window needs to minimized.</summary>
	/// <param name="hwnd">The handle to the specificed window.</param>
	/// <remarks>
	///   Despite the function being called named <see cref="CloseWindow" />,
	///   it actually minimizes it.
	/// </remarks>
	_NORETURN void Minimize(HWND hwnd);
#pragma endregion


	_NORETURN inline constexpr void SetCaption(LPCTSTR lpcCaption) noexcept {
		lpCaption = lpcCaption;
	}

#pragma region MessageQueue and DialogProc
	/// <summary>
	///   Messages related to the RA dialogs get posted/sent here. Overrides for
	///   message handlers will get called first.
	/// </summary>
	/// <param name="hwnd">
	///   The handle supplied by Windows for a new window.
	/// </param>
	/// <param name="uMsg">The message to processed.</param>
	/// <param name="wParam">Additional message information.</param>
	/// <param name="lParam">Additional message information.</param>
	/// <returns>
	///   Non-zero if a message was processed, otherwise <c>0</c>.
	/// </returns>
	/// <remarks>
	///   Posted messages will be sent to the system's message queue first, sent
	///   messages will be sent here.
	/// </remarks>
	_NODISCARD static INT_PTR CALLBACK MsgQueue(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam);


	/// <summary>
	///   Everything except WM_INITDIALOG needs to go to the derived DialogProc. Every Dialog must have an override for OnInitDialog. You don't have to put WM_INITDIALOG in the DialogProc.
	/// </summary>
	/// <param name="hDlg">The handle for the top-level dialog.</param>
	/// <param name="uMsg">The message to processed.</param>
	/// <param name="wParam">Additional message information.</param>
	/// <param name="lParam">Additional message information.</param>
	/// <returns>
	///   Non-zero if a message was processed, otherwise <c>0</c>.
	/// </returns>
	/// <remarks>This function must be pure virtual. Every single derivation requires WM_INITDIALOG, WM_COMMAND, and WM_NCDESTROY to be handled.</remarks>
	_NODISCARD virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam) = 0;
#pragma endregion

	_NORETURN void button_check(HWND hwndCtl) noexcept;
	_NORETURN void button_uncheck(HWND hwndCtl) noexcept;
	BOOL Hide(HWND hwndCtl) noexcept;
	BOOL Show(HWND hwndCtl) noexcept;

	// making the default constructor private may have been the cause
	IRA_Dialog() noexcept = default;

	/// <summary>
	///   The Resource Id for the dialog or control.
	/// </summary>
	int nResourceId{ 0 };
	bool bIsModal{ false };
	HFONT hFont{ nullptr };
	HICON hIcon{ nullptr };
	// The handle version didn't work auto deletion
	LPCTSTR lpCaption{ TEXT("") };

    


	// having hwnd as a data member is a pain in the ass, we'll just pass in hwnds
private /*internal*/:
	_NORETURN void delete_font() noexcept;
	_NORETURN void delete_icon() noexcept;
	_NORETURN void delete_caption() noexcept;

	using dlg_owner = std::unique_ptr<IRA_Dialog>;
	// for the love of god
	

    // Windows were getting created twice
	friend IRA_Dialog* CALLBACK OnInitDialog(_In_ HWND hwnd, _In_ HWND hwndFocus,
		_In_ LPARAM lParam);


    // purly for debug reasons
	UINT current_msg{ 0_z };
	static IRA_Dialog* ira;
};

// Lets see if a global callback will work
IRA_Dialog* CALLBACK OnInitDialog(_In_ HWND hwnd, _In_ HWND hwndFocus, _In_ LPARAM lParam);


} // namespace ra

#ifndef _RA
#define _RA ::ra::
#endif // !_RA



#endif // !RA_Dialog_H


