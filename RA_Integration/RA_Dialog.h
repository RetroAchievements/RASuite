#pragma once
class RA_Dialog
{
public:
	RA_Dialog();
	virtual ~RA_Dialog();

	// Creates a modal dialog
	static INT_PTR DoModal();

	// Creates a modeless dialog using MAKEINTRESOURCE
	virtual HWND Create();

	// Creates a modeless dialog using just the template ID in the resource file
	virtual HWND Create2();
};
