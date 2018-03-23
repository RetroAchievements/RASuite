#include "RA_Dlg_RomChecksum.h"

#include "RA_Resource.h"
#include "RA_Core.h"


RA_Dlg_RomChecksum::RA_Dlg_RomChecksum() :
	IRA_Dialog{ IDD_RA_ROMCHECKSUM }
{
}

BOOL RA_Dlg_RomChecksum::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetText(GetDlgItem(hwnd, IDC_RA_ROMCHECKSUMTEXT), 
		NativeStr(std::string("ROM Checksum: ") + g_sCurrentROMMD5).c_str());
	return 0;
}

void RA_Dlg_RomChecksum::OnCommand(HWND hwnd, int id, HWND hwndCtl,
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




void RA_Dlg_RomChecksum::OnCopyChecksum()
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

INT_PTR RA_Dlg_RomChecksum::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, 
	LPARAM lParam)
{
	return 0;
}


