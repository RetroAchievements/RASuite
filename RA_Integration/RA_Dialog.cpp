#include "stdafx.h"
#include "RA_Dialog.h"

RA_Dialog::RA_Dialog()
{
}

RA_Dialog::~RA_Dialog()
{
}

// Creates a modal dialog
INT_PTR RA_Dialog::DoModal()
{
	return INT_PTR();
}

// Creates a modeless dialog using MAKEINTRESOURCE
HWND RA_Dialog::Create()
{
	return HWND();
}

// Creates a modeless dialog using just the template ID in the resource file
HWND RA_Dialog::Create2()
{
	return HWND();
}
