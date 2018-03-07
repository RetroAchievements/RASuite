#ifndef RA_DLG_LOGIN_H
#define RA_DLG_LOGIN_H
#pragma once


#include "IRA_Dialog.h"

class RA_Dlg_Login : public IRA_Dialog
{
public:
	RA_Dlg_Login();
	~RA_Dlg_Login() noexcept;

	// Inherited via IRA_Dialog
	virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;
	virtual BOOL OnInitDialog(HWND hDlg, HWND hDlgFocus,
		LPARAM lParam) override;
	virtual void OnCommand(HWND hDlg, int id, HWND hDlgCtl,
		UINT codeNotify) override;
	void OnOK(HWND hDlg) override;
private:
	HWND UsernameCtl{ nullptr };
	HWND PassCtl{ nullptr };
};
#endif // !RA_DLG_LOGIN_H

