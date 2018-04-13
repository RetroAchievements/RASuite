#ifndef RA_DLG_ROMCHECKSUM_H
#define RA_DLG_ROMCHECKSUM_H
#pragma once

#include "IRA_Dialog.h"

namespace ra {
	class Dlg_RomChecksum : public IRA_Dialog
	{
	public:
		Dlg_RomChecksum();
		~Dlg_RomChecksum() noexcept = default;
		Dlg_RomChecksum(const Dlg_RomChecksum&) = delete;
		Dlg_RomChecksum& operator=(const Dlg_RomChecksum&) = delete;
		Dlg_RomChecksum(Dlg_RomChecksum&&) = delete;
		Dlg_RomChecksum& operator=(Dlg_RomChecksum&&) = delete;

		// The functions below keep on failing if I define it here
		HWND GetWindow() const noexcept override;

		HWND Create() override {
			throw std::runtime_error{ "This isn't a modeless dialog!" };
			return null;
		}

		BOOL OnInitDialog(HWND hwnd, _UNUSED HWND hwndFocus, _UNUSED LPARAM lParam) override;
		void OnCommand(HWND hwnd, int id, _UNUSED HWND hwndCtl, _UNUSED UINT codeNotify) override;
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
