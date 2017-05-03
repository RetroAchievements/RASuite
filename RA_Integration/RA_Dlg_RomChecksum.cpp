#include "stdafx.h"
#include "RA_Dlg_RomChecksum.h"

#include "RA_Resource.h"
#include "RA_Core.h"

//static
BOOL RA_Dlg_RomChecksum::DoModalDialog()
{
	return DialogBox( g_hThisDLLInst, MAKEINTRESOURCE( IDD_RA_ROMCHECKSUM ), g_RAMainWnd, RA_Dlg_RomChecksum::RA_Dlg_RomChecksumProc );
}

INT_PTR CALLBACK RA_Dlg_RomChecksum::RA_Dlg_RomChecksumProc( HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( nMsg )
	{
	case WM_INITDIALOG:
		SetDlgItemText( hDlg, IDC_RA_ROMCHECKSUMTEXT, Widen( std::string( "ROM Checksum: " ) + g_sCurrentROMMD5 ).c_str() );
		return FALSE;

	case WM_COMMAND:
		switch ( wParam )
		{
		case IDOK:
			EndDialog( hDlg, TRUE );
			return TRUE;

		case IDC_RA_COPYCHECKSUMCLIPBOARD:
		{
			//	Allocate memory to be managed by the clipboard
			// A bit dangerous, it might make the length exceed capacity
			auto hMem{GlobalAlloc( GMEM_MOVEABLE, g_sCurrentROMMD5.length() + 1 )};

			// N.B. Code Analysis

			// warning C28183: 'hMem' could be '0', and is a copy of the value
			// found in 'GlobalAlloc()31': this odes not adhere to the
			// specification for the function 'GlobalLock'.

			// Layman's terms: hMem is not checked whether or not it's a null pointer
			Expects( hMem );
			auto glock{GlobalLock( hMem )};
			Ensures( glock );

			// I'm probably gonna stop explaining, just search MSDN for why I'm doing this
			memcpy( glock, g_sCurrentROMMD5.c_str(), g_sCurrentROMMD5.length() + 1 );
			GlobalUnlock( hMem );

			OpenClipboard( nullptr );
			EmptyClipboard();
			SetClipboardData( CF_TEXT, hMem );
			CloseClipboard();

			//MessageBeep( 0xffffffff );
		}
		return TRUE;

		default:
			return FALSE;
		}

	case WM_CLOSE:
		EndDialog( hDlg, TRUE );
		return TRUE;

	default:
		return FALSE;
	}
}
