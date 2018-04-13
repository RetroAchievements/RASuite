#ifndef IRA_WINDOWCLASS_H
#define IRA_WINDOWCLASS_H
#pragma once

#include <WTypes.h>


namespace ra {
	/// <summary>
	///   Extremely simple window class, derive to make specialized versions
	/// </summary>
	/// <seealso cref="WNDCLASSEX" />
	class IRA_WndClass : protected WNDCLASSEX
	{
		friend class window_h;
	public:
		IRA_WndClass(LPCTSTR className) noexcept;
		virtual ~IRA_WndClass() noexcept;


		virtual bool Register() noexcept;
		virtual LPCTSTR className() const noexcept { return lpszClassName; }
	};
} // namespace ra

#endif // !IRA_WINDOWCLASS_H

