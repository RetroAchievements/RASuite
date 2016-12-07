#ifndef RA_PCH_H
#define RA_PCH_H

#ifdef _WIN32
#pragma once
#include "targetver.h"
#endif

// RA_PCH.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
// I for one don't like slow building times
// Basic Project includes will be here but not most
// If something isn't here it's most likely because I think it might change

// STL Headers
// Lots of the C library headers converted to C++ counterparts to take advantage of C++11
#include <algorithm> //	std::replace
#include <cassert>   // assert.h
#include <cstdio>    // stdio.h
#include <cstdlib>   // stdlib.h
#include <cstring>   // string.h
#include <ctime>     // time.h
#include <deque>
#include <fstream>
#include <locale>
#include <mutex>
#include <queue>
#include <sstream>
#include <stack>
#include <thread>
#include <vector>

// Windows API Headers
#include <windows.h>
#include <codecvt>
#include <commctrl.h>
#include <commdlg.h>
#include <direct.h>
#include <io.h>
#include <memory.h>
#include <shlobj.h>
#include <wincodec.h>
#include <windowsx.h>
#include <WinHttp.h>
#include <WTypes.h>

// Project Headers
#include "md5.h"
#include "RA_Defs.h"

#endif
