#ifndef RA_DLG_ROMCHECKSUM_H
#define RA_DLG_ROMCHECKSUM_H
#pragma once

#include "IRA_Dialog.h"

namespace ra {
	class Dlg_RomChecksum : public IRA_Dialog
	{
	public:
		Dlg_RomChecksum();

		BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) override;
		void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) override;
		void OnCopyChecksum();

		void OnNCDestroy(HWND hwnd) override;

		// Inherited via IRA_Dialog
		virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
			LPARAM lParam) override;

	private:
		HWND hChecksum{ nullptr };
	};
} // namespace ra


#endif // !RA_DLG_ROMCHECKSUM_H