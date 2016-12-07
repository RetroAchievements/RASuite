#pragma once

#include <wtypes.h>

class RA_Dlg_RomChecksum
{
public:
	static bool DoModalDialog();

public:
	static INT_PTR CALLBACK RA_Dlg_RomChecksumProc( HWND, UINT, WPARAM, LPARAM );
};
