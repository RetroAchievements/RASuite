#ifndef RA_DLG_ROMCHECKSUM_H
#define RA_DLG_ROMCHECKSUM_H
#pragma once

#include <wtypes.h>
#include "IRA_Dialog.h"

class RA_Dlg_RomChecksum : public IRA_Dialog
{

public:
	RA_Dlg_RomChecksum();
	~RA_Dlg_RomChecksum() noexcept;

	// Inherited via IRA_Dialog
	virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;
	virtual BOOL OnInitDialog(HWND hDlg, HWND hDlgFocus,
		LPARAM lParam) override;
	virtual void OnCommand(HWND hDlg, int id, HWND hDlgCtl,
		UINT codeNotify) override;

	
private:
	HWND ChecksumCtl{ nullptr };

	// Event specific to this dialog
	void OnCopyChecksumClipboard();
};
#endif // !RA_Dlg_RomChecksum_h

