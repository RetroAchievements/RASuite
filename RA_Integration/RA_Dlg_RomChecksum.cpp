#include "common.h"
#include "RA_Dlg_RomChecksum.h"


#include "RA_Core.h"

RA_Dlg_RomChecksum::RA_Dlg_RomChecksum() :
	IRA_Dialog{ IDD_RA_ROMCHECKSUM },
	ChecksumCtl{ MakeControl(IDC_RA_ROMCHECKSUMTEXT) }
{
}

RA_Dlg_RomChecksum::~RA_Dlg_RomChecksum() noexcept
{
	OnDestroy(ChecksumCtl);
}

INT_PTR RA_Dlg_RomChecksum::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	switch ( uMsg )
	{
		HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
		HANDLE_MSG(hDlg, WM_CLOSE, OnClose);
	}
	return 0;
}

BOOL RA_Dlg_RomChecksum::OnInitDialog(HWND hDlg, HWND hDlgFocus, LPARAM lParam)
{
	SetWindowText(ChecksumCtl, NativeStr(std::string("ROM Checksum: ") +
		g_sCurrentROMMD5).c_str());
	return 0;
}

void RA_Dlg_RomChecksum::OnCommand(HWND hDlg, int id, HWND hDlgCtl, 
	UINT codeNotify)
{
	switch ( id )
	{
	case IDOK:
		OnOK(hDlg);
		break;

	case IDC_RA_COPYCHECKSUMCLIPBOARD:
		OnCopyChecksumClipboard();
	}
}

void RA_Dlg_RomChecksum::OnCopyChecksumClipboard()
{
	//	Allocate memory to be managed by the clipboard
	auto hMem{ GlobalAlloc(GMEM_MOVEABLE, g_sCurrentROMMD5.length() + 1) };
	memcpy(GlobalLock(hMem), g_sCurrentROMMD5.c_str(), 
		g_sCurrentROMMD5.length() + 1);
	GlobalUnlock(hMem);

	OpenClipboard(nullptr);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();

	//MessageBeep( 0xffffffff );
}
