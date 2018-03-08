#pragma once

#include <wtypes.h>
#include "IRA_Dialog.h"
#include "RA_Resource.h"

class RA_Dlg_Login : public IRA_Dialog
{
	// Old functions
	/*static BOOL DoModalLogin();*/
	/*static INT_PTR CALLBACK RA_Dlg_LoginProc( HWND, UINT, WPARAM, LPARAM );*/
public:
	RA_Dlg_Login();
	~RA_Dlg_Login() noexcept = default;

	// Inherited via IRA_Dialog
	virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, 
		LPARAM lParam) override;
	BOOL OnInitDialog(HWND hDlg, HWND hDlgFocus, LPARAM lParam) override;
	void OnCommand(HWND hDlg, int id, HWND hDlgCtl,	UINT codeNotify) override;
	void OnOK(HWND hDlg) override;
};
