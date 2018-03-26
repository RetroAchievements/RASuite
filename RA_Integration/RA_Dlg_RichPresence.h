#ifndef DLG_RICHPRESENCE_H
#define DLG_RICHPRESENCE_H
#pragma once

#include "IRA_Dialog.h"


namespace ra {
class Dlg_RichPresence : public IRA_Dialog
{
public:
	Dlg_RichPresence();
	void StartMonitoring();

protected:
	/// <inheritdoc />
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) override;

	// OnCommand doesn't do anything in this dialog since there's no buttons
	/// <inheritdoc />
	void OnCommand(HWND hwnd, int id, HWND hwndCtl,	UINT codeNotify) override;

	/// <inheritdoc />
	void OnNCDestroy(HWND hwnd) override;

	/// <inheritdoc />
	void OnTimer(HWND hwnd, UINT id) override;

	/// <inheritdoc />
	void OnSize(HWND hwnd, UINT state, int cx, int cy) override;

	/// <inheritdoc />
	void Move(HWND hwnd, int x, int y) override;


	void Close(HWND hwnd) override;

	/// <inheritdoc />
	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;
private:
	void StartTimer();
	void StopTimer();

	bool m_bTimerActive{ false };
	HWND m_hRichPresenceText{ nullptr };

};
} // namespace ra


extern _RA Dlg_RichPresence g_RichPresenceDialog;




#endif // !DLG_RICHPRESENCE_H