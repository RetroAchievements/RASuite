#ifndef COMMON_H
#define COMMON_H

// Stuff in here barely changes

// We don't care about warnings for these

#pragma warning(push, 1)
#pragma warning(disable : 4365 4571 4623 4625 4626 4701 4774 4820 4917 5026 \
						  5027 5039)
// Windows
// Commented out stuff here is included in Windows.h
#include <Windows.h>
#include <commctrl.h>
// #include <commdlg.h>
#include <ddraw.h>
#include <direct.h>
#include <io.h> //	_access()
#include <shlobj.h>
#include <tchar.h>
#include <wincodec.h>
#include <WindowsX.h>
#include <winhttp.h>
#include <WTypes.h>

// Standard
#include <algorithm>	//	std::replace
#include <codecvt>
#include <deque>
#include <fstream>
#include <locale>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <vector>




// Standard C
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <time.h>






// Other Libs
#include "md5.h"

// Rarely changed project files
#include "RA_Defs.h"
#include "RA_Resource.h"

#pragma warning(pop)

#endif // !COMMON_H
