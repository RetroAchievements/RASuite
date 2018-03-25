#ifndef RA_DLG_LOGIN_H
#define RA_DLG_LOGIN_H
#pragma once


#include "IRA_Dialog.h"
namespace ra {
class Dlg_Login : public IRA_Dialog
{

public:
	Dlg_Login();

	/// <inheritdoc />
	virtual INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;

	/// <inheritdoc />
	BOOL OnInitDialog(HWND hwnd, HWND hDlgFocus, LPARAM lParam) override;

	/// <inheritdoc />
	void OnCommand(HWND hwnd, int id, HWND hDlgCtl, UINT codeNotify) override;

	/// <inheritdoc />
	void OnOK(HWND hwnd) override;

	/// <inheritdoc />
	void OnNCDestroy(HWND hwnd) override;

private:
	// Do not initilize these in the constructor, it won't work
	HWND hUsername{ nullptr };
	HWND hPassword{ nullptr };
	HWND hSavePassword{ nullptr };
};
} // end namespace ra

#endif // !RA_DLG_LOGIN_H