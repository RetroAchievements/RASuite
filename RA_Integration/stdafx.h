#pragma once
#ifndef STDAFX_H
#define STDAFX_H

// The purpose of this file is to dramatically speedup the build process

/*
	Standard libraries and third party headers go here.

	To prevent confusion, we will put what headers are included in each one if they contained
	a header that was previously included.

	"Export" headers like Windows.h will list what is used with macro below so it's not
	included twice.

	Assume a file w/o ".h" appended is C++ Standard Template Library header
*/

// 3rd Party Headers

/*
	Has all of GSL, and assert.h
	Useful for narrowing conversions, bounds, and for expressing pre/post conditions.
*/
#include <gsl.h>
#include "md5.h"

//	RA-Only
#ifdef RA_EXPORTS
/*
	Included:
		— reader.h
		— rapidjson.h
		— cassert->assert.h
*/
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/filestream.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/error/en.h"
#endif // RA_EXPORTS

#define WIN32_LEAN_AND_MEAN // This strips out uncommon headers but we need to add some now

/*
	Includes sdkddkver.h so we don't need a targetver.h file

	Included:
		— ctype.h
		— except.h
		— guiddef.h
		— stdarg.h
		— string.h
		— winbase.h
		— windef.h
		— winerror.h
		— wingdi.h
		— winnt.h
		— winuser.h

	Re-include because of WIN32_LEAN_AND_MEAN:
		— mmsystem.h
		— shellapi.h
*/
#include <Windows.h>
#include <CommCtrl.h>
#include <CommDlg.h>

/*
	Included:
		— stdlib.h
		— wtypes.h
		— rpcndr.h
*/
#include <ddraw.h>
#include <direct.h>
#include <io.h>		//	_access()
#include <memory.h>
#include <MMSystem.h>
#include <ShellAPI.h>
#include <ShlObj.h>
#include <tchar.h>
#include <wincodec.h>
#include <WindowsX.h> // Useful message crackers for Window/Dialog procedures
#include <winhttp.h>

// Standard Template Library

/*
	Included:
		— algorithm
		— iterator
		— tuple
*/
#include <array>

/*
	Included:
		— locale
		— cwchar
*/
#include <codecvt>

/*
	Included:
		— istream
*/
#include <fstream>

/*
	Included:
		— tuple
		— xtree
*/
#include <map>

/*
	Included:
		— chrono
			— time.h
		— functional
		— thread
*/
#include <mutex>

/*
	Included:
		— algorithm
		— deque
		— vector
*/
#include <queue>

/*
	Included:
		— string
*/
#include <sstream>

/*
	Included:
		— deque
*/
#include <stack>

// Standard C Library
// All of the ones we need are already included

#endif // !STDAFX_H
