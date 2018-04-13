
#include "IRA_WndClass.h"

#include "RA_Core.h"
#include "IRA_Dialog.h"


namespace ra {
	IRA_WndClass::IRA_WndClass(LPCTSTR className) noexcept
	{
		cbSize        = sizeof(WNDCLASSEX);
		style         = 0;
		lpfnWndProc   = reinterpret_cast<WNDPROC>(IRA_Dialog::MsgQueue);
		cbClsExtra    = 0;
		cbWndExtra    = 0;

		// FreeLibrary seems to do access violations so I'll just use the global
		hInstance     = g_hThisDLLInst;
		hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
		hCursor       = LoadCursor(nullptr, IDC_ARROW);
		hbrBackground = GetStockBrush(COLOR_BTNFACE);
		lpszMenuName  = nullptr;
		lpszClassName = className;
		hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);

		Register();
	} // end constructor

	IRA_WndClass::~IRA_WndClass() noexcept
	{
		if ( hbrBackground )
			DeleteBrush(hbrBackground);
	} // end destructor

	bool IRA_WndClass::Register() noexcept
	{
		if ( RegisterClassEx(this) )
			return true;
		else
			return false;
	} // end function Register
} // namespace ra
