#ifndef DLG_RICHPRESENCE_H
#define DLG_RICHPRESENCE_H
#pragma once

#include "IRA_Dialog.h"


namespace ra {
class Dlg_RichPresence : public IRA_Dialog
{
public:
	Dlg_RichPresence();
	~Dlg_RichPresence() noexcept = default;

	// Alright this is really weird, the wrong WM_INITDIALOG is getting called,
	// maybe move semantics would help
	Dlg_RichPresence(const Dlg_RichPresence&) = delete;
	Dlg_RichPresence& operator=(const Dlg_RichPresence&) = delete;
	Dlg_RichPresence(Dlg_RichPresence&& b) noexcept;
	Dlg_RichPresence& operator=(Dlg_RichPresence&& b) noexcept;


	void StartMonitoring();
	HWND GetWindow() const noexcept { return m_hRpWnd; }
	void SetWindow() { m_hRpWnd = Create(); }
	BOOL Show() { return ShowWindow(m_hRpWnd, SW_SHOW); }
protected:
	/// <inheritdoc />
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) override;

	// OnCommand doesn't do anything in this dialog since there's no buttons
	/// <inheritdoc />
	void OnCommand(HWND hwnd, int id, _UNUSED HWND hwndCtl,
		_UNUSED UINT codeNotify) override;

	/// <inheritdoc />
	void OnNCDestroy(HWND hwnd) override;

	/// <inheritdoc />
	void OnTimer(_UNUSED HWND hwnd, _UNUSED UINT id) override;




	void Close(HWND hwnd) override;

	/// <inheritdoc />
	INT_PTR CALLBACK DialogProc(_UNUSED HWND hDlg, _UNUSED UINT uMsg,
		_UNUSED WPARAM wParam, _UNUSED LPARAM lParam) override;
private:
	void StartTimer();
	void StopTimer();

	bool m_bTimerActive{ false };
	HWND m_hRichPresenceText{ nullptr };
	HWND m_hRpWnd{ nullptr };

	// Inherited via IRA_Dialog
	
};
} // namespace ra


extern _RA Dlg_RichPresence g_RichPresenceDialog;




#endif // !DLG_RICHPRESENCE_H
