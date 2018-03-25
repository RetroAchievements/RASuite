#ifndef RA_DIALOG_H
#define RA_DIALOG_H
#pragma once

#include <wtypes.h>

// A lot of the documentation is derived from here:
// https://msdn.microsoft.com/en-us/library/windows/desktop/ff818516.aspx

// Use <inheritdoc /> for derived classes' overrides

namespace ra {

	/// <summary>
	///   Base class for all RA Dialogs. Not a "true" interface as not all dialogs
	///   share this functionality, but needed for the Message Queue.
	/// </summary>
	/// <remarks>
	///   A lot of the comments will refer to an <c>HWND</c> as either an active or
	///   a specified window. This is because the handle getting passed around in
	///   the queue is always active but controls and parent windows aren't.
	/// </remarks>
	class IRA_Dialog
	{
	public:
		/// <summary>
		///   Initializes a new instance of the <see cref="IRA_Dialog" /> class.
		/// </summary>
		/// <param name="resId">The resource identifier.</param>
		/// <remarks>
		///   The parent HWND in the comments is there just in-case we want to
		///   spawn out a child window from a dialog. A good example would be in
		///   <seealso cref="RA_Dlg_Achievement" />.
		/// </remarks>
		IRA_Dialog(int resId/*, HWND parent = nullptr*/);

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

		/// <summary>
		/// Displays a modal.
		/// </summary>
		/// <returns></returns>
		virtual INT_PTR DoModal();


		/// <summary>
		/// Creates a handle for a modeless dialog.
		/// </summary>
		/// <returns>The handle for a modelss dialog.</returns>
		/// 
		virtual HWND Create();



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
		static INT_PTR CALLBACK MsgQueue(HWND hwnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);


		/// <summary>
		///   The main dialog procedure. In most cases, it will return 0. Process
		///   any user-defined message (non-system) if needed.
		/// </summary>
		/// <param name="hDlg">The handle for the top-level dialog.</param>
		/// <param name="uMsg">The message to processed.</param>
		/// <param name="wParam">Additional message information.</param>
		/// <param name="lParam">Additional message information.</param>
		/// <returns>
		///   Non-zero if a message was processed, otherwise <c>0</c>.
		/// </returns>
		/// <remarks>This function must be pure virtual.</remarks>
		virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
			LPARAM lParam) = 0;


		// permissions based off of MFC
#pragma region Public Message Handlers
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
		virtual BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) = 0;
		virtual HFONT GetFont(HWND hwnd);


		/// <summary>
		///   Destroys a window/control if not a modal dialog, otherwise it just
		///   ends it.
		/// </summary>
		/// <param name="hwnd">The handle to active window/control.</param>
		/// <remarks>
		///   This is not to be confused with <see cref="OnClose" /> when
		///   <c>IDCLOSE</c> is encountered.
		/// </remarks>
		void Close(HWND hwnd);


		/// <summary>
		///   Posts a WM_QUIT message.
		/// </summary>
		/// <param name="hwnd">The handle to the active window.</param>
		void Destroy(HWND hwnd);
		void Enable(HWND hwnd, BOOL fEnable = TRUE);


		/// <summary>
		/// Sets the focus.
		/// </summary>
		/// <param name="hwnd">The HWND.</param>
		/// <param name="hwndOldFocus">The HWND old focus.</param>
		virtual void SetFocus(HWND hwnd, HWND hwndOldFocus = nullptr);
		void SetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw);
		void ShowWindow(HWND hwnd, BOOL fShow, UINT status);
#pragma endregion

	protected:
#pragma region Protected Message Handlers
		virtual void OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
		virtual void OnActivateApp(HWND hwnd, BOOL fActivate, DWORD dwThreadId);
		void OnCancelMode(HWND hwnd);
		virtual void OnChar(HWND hwnd, TCHAR ch, int cRepeat);



		/// <summary>
		/// Called after <c>WM_COMMAND</c> is sent.
		/// </summary>
		/// <param name="hwnd">The active window.</param>
		/// <param name="id">A control ID.</param>
		/// <param name="hwndCtl">A handle to a control.</param>
		/// <param name="codeNotify">A notification code.</param>
		virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) = 0;
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
		INT GetText(HWND hwnd, int cchTextMax, LPTSTR lpszText);
		INT GetTextLength(HWND hwnd);


		/// <summary>
		///   Called when <c>WM_KEYDOWN</c> is posted.
		/// </summary>
		/// <param name="hwnd">The handle to the active window.</param>
		/// <param name="vk">A non-system virtual key-code.</param>
		/// <param name="fDown">Key is down, this is always <c>TRUE</c>.</param>
		/// <param name="cRepeat">The repeat count.</param>
		/// <param name="flags">
		///   The scan and context codes. The extended-key, previous key-state, and
		///   transition-state flags.
		/// </param>
		virtual void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);


		/// <summary>
		///   Kills the focus for <paramref name="hwnd" /> and sets the focus
		///   for <paramref name="hwndNewFocus" />.
		/// </summary>
		/// <param name="hwnd">The HWND.</param>
		/// <param name="hwndNewFocus">The HWND new focus.</param>
		void KillFocus(HWND hwnd, HWND hwndNewFocus);
		virtual void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
		virtual void OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem);
		virtual void OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys);
		virtual BOOL OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized);


		/// <summary>
		///   Called when <c>WM_NCCREATE</c> is posted. The only instance this may
		///   need to be used in <seealso cref="MemoryViewControl" />.
		/// </summary>
		/// <param name="hwnd">The handle for a new window to be created.</param>
		/// <param name="lpCreateStruct">
		///   Contains inital parameters to construct a new window.
		/// </param>
		/// <returns></returns>
		BOOL OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);


		/// <summary>
		///   Called when <c>WM_NCDESTROY</c> is posted. Use this function to
		///   destroy any controls and non-shared resource.
		/// </summary>
		/// <param name="hwnd">
		///   A handle to a window, usually a control window. Usually this
		///   specific parameter will not be used.
		/// </param>
		virtual void OnNCDestroy(HWND hwnd);
		virtual LRESULT OnNotify(HWND hwnd, int idFrom, NMHDR* pnmhdr);
		virtual void OnPaint(HWND hwnd);
		void OnQuit(HWND hwnd, int exitCode);


		/// <summary>
		///   Sets the text for the specified window, usually a caption or control.
		/// </summary>
		/// <param name="hwnd">The handle to the specified window.</param>
		/// <param name="lpszText">The text to be set.</param>
		void SetText(HWND hwnd, LPCTSTR lpszText);


		/// <summary>
		/// Called when <c>WM_SIZE</c> is posted.
		/// </summary>
		/// <param name="hwnd">The HWND.</param>
		/// <param name="state">The state.</param>
		/// <param name="cx">The cx.</param>
		/// <param name="cy">The cy.</param>
		virtual void OnSize(HWND hwnd, UINT state, int cx, int cy);
		virtual void OnTimer(HWND hwnd, UINT id);
		virtual void OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos);
		virtual BOOL OnWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos);
#pragma endregion

		/// <summary>
		/// Raises the OK event. Should be called when an OK button is pressed.
		/// </summary>
		/// <param name="hwnd">The handled to the active window.</param>
		virtual void OnOK(HWND hwnd);


		/// <summary>
		/// Raises the Cancel event. Should be called when a cancel button is pressed.
		/// </summary>
		/// <param name="hwnd">The handled to the active window.</param>
		virtual void OnCancel(HWND hwnd);


		/// <summary>
		/// Raises the Close event. Should be called when pressing the "x" button.
		/// </summary>
		/// <param name="hwnd">The handle to the specificed window.</param>
		void OnClose(HWND hwnd);

		/// <summary>Called when a window needs to minimized.</summary>
		/// <param name="hwnd">The handle to the specificed window.</param>
		/// <remarks>
		///   Despite the function being called named <see cref="CloseWindow" />,
		///   it actually minimizes it.
		/// </remarks>
		void Minimize(HWND hwnd);

		constexpr void SetCaption(LPCTSTR lpCaption) noexcept {
			lpCaption_ = lpCaption;
		}

		constexpr void DeleteCaption() noexcept { lpCaption_ = TEXT(""); }
		/// <summary>
		///   The Resource Id for the dialog or control.
		/// </summary>
		int nResourceId_{ 0 };
		bool bIsModal_{ false };
		HFONT hFont{ nullptr };
	private:
		/*HFONT hfont_{ nullptr };*/
		LPCTSTR lpCaption_{ TEXT("") };
		static LRESULT lStorage_;
	};

} // namespace ra
#endif // !RA_Dialog_H


