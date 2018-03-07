#ifndef IWINDOWCLASS_H
#define IWINDOWCLASS_H
#pragma once


class IWindowClass : protected WNDCLASSEX
{
public:
	IWindowClass(HINSTANCE hInst, LPCTSTR className);
	virtual ~IWindowClass() noexcept;
	bool Register();
	LPCTSTR GetClassname() const;
};

#endif // !IWINDOWCLASS_H
