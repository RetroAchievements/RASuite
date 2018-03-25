#include "RA_Dlg_RomChecksum.h"

#include "RA_Resource.h"
#include "RA_Core.h"


namespace ra {
	using namespace std::string_literals;

	Dlg_RomChecksum::Dlg_RomChecksum() :
		IRA_Dialog{ IDD_RA_ROMCHECKSUM }
	{
	}

	BOOL Dlg_RomChecksum::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
	{
		hChecksum = GetDlgItem(hwnd, IDC_RA_ROMCHECKSUMTEXT);
		SetText(hChecksum, NativeStr("ROM Checksum: "s + g_sCurrentROMMD5).c_str());
		return FALSE;
	}

	void Dlg_RomChecksum::OnCommand(HWND hwnd, int id, HWND hwndCtl,
		UINT codeNotify)
	{
		switch ( id )
		{
		case IDOK:
			OnOK(hwnd);
			break;
		case IDC_RA_COPYCHECKSUMCLIPBOARD:
			OnCopyChecksum();
		}
	}




	void Dlg_RomChecksum::OnCopyChecksum()
	{
		//	Allocate memory to be managed by the clipboard
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, g_sCurrentROMMD5.length() + 1);
		memcpy(GlobalLock(hMem), g_sCurrentROMMD5.c_str(), g_sCurrentROMMD5.length() + 1);
		GlobalUnlock(hMem);

		OpenClipboard(nullptr);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();

		//MessageBeep( 0xffffffff );
	}

	void Dlg_RomChecksum::OnNCDestroy(HWND hwnd)
	{
		Close(hChecksum);
		IRA_Dialog::OnNCDestroy(hwnd);
	}

	INT_PTR Dlg_RomChecksum::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam)
	{
		return 0;
	}

} // namespace ra

