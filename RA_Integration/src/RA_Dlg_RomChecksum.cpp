#include "RA_Dlg_RomChecksum.h"

#include "RA_Resource.h"
#include "RA_Core.h"


namespace ra {
using namespace std::string_literals;

Dlg_RomChecksum::Dlg_RomChecksum() :
	IRA_Dialog{ IDD_RA_ROMCHECKSUM }
{
}

HWND Dlg_RomChecksum::GetWindow() const noexcept { return GetActiveWindow(); }

BOOL Dlg_RomChecksum::OnInitDialog(HWND hwnd, _UNUSED HWND hwndFocus,
	_UNUSED LPARAM lParam)
{
	hChecksum = GetDlgItem(hwnd, IDC_RA_ROMCHECKSUMTEXT);
	SetText(hChecksum, NativeStr("ROM Checksum: "s + g_sCurrentROMMD5).c_str());
	return FALSE;
}

void Dlg_RomChecksum::OnCommand(HWND hwnd, int id, _UNUSED HWND hwndCtl,
	_UNUSED UINT codeNotify)
{
	switch (id)
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
#pragma warning(suppress : 4702) 
void Dlg_RomChecksum::OnNCDestroy(HWND hwnd)
{
	DestroyControl(hChecksum);
	// I don't understand how this is unreachable...
	IRA_Dialog::OnNCDestroy(hwnd);

}

INT_PTR Dlg_RomChecksum::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	// If this works, this will be awesome
	return IRA_Dialog::DialogProc(hwnd, uMsg, wParam, lParam);
}

} // namespace ra

