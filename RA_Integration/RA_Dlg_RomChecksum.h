#ifndef RA_DLG_ROMCHECKSUM_H
#define RA_DLG_ROMCHECKSUM_H
#pragma once

#include "IRA_Dialog.h"

class RA_Dlg_RomChecksum : public IRA_Dialog
{
public:
	RA_Dlg_RomChecksum();
	~RA_Dlg_RomChecksum() noexcept = default;
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) override;
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) override;
	void OnCopyChecksum();

	// Inherited via IRA_Dialog
	virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;
};


#endif // !RA_DLG_ROMCHECKSUM_H