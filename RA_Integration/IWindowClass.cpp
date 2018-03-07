#include "common.h"
#include "IWindowClass.h"

#include "IRA_Dialog.h"
#include "RA_Core.h"

IWindowClass::IWindowClass(HINSTANCE hInst, LPCTSTR className)
{
	hInstance = hInst;
	if ( !hInstance )
		GetModuleHandle("RA_Integration.dll");

	// all messages for windows belonging to this Window Class will get sent to
	// MsgQueue
	lpfnWndProc   = reinterpret_cast<WNDPROC>(IRA_Dialog::MsgQueue);
	lpszClassName = className;
	
	// Basic stuff, can be changed later
	lpszMenuName  = 0;
	cbSize        = sizeof(WNDCLASSEX);
	cbClsExtra    = 0;
	cbWndExtra    = 0;
	style         = 0;
	hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
	hCursor       = LoadCursor(NULL, IDC_ARROW);
	hbrBackground = GetStockBrush(COLOR_BTNFACE);

	Register();
}

IWindowClass::~IWindowClass()
{
	FreeLibrary(hInstance);
}

bool IWindowClass::Register()
{
	if ( RegisterClassEx(this) ) 
		return true;
	else
		return false;
}

LPCTSTR IWindowClass::GetClassname() const
{
	return lpszClassName;
}
