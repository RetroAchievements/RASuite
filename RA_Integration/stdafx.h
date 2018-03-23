#ifndef STDAFX_H
#define STDAFX_H


#pragma warning(push, 1)

// Windows
#pragma region Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // this has to be on top
#include <ddraw.h>
#include <io.h>
#include <ShlObj.h>
#include <tchar.h>
#include <wincodec.h>
#include <WindowsX.h>
#include <WinHttp.h> // it's always something simple.....

#ifdef WIN32_LEAN_AND_MEAN
#include <CommDlg.h>
#include <MMSystem.h>
#include <ShellAPI.h>
#endif // WIN32_LEAN_AND_MEAN  
#pragma endregion



// Standard C++
#pragma region Standard C++
#include <array> // algorithm, iterator, tuple
#include <codecvt> // locale
#include <fstream>
#include <map>
#include <memory>
#include <mutex> // chrono (time.h), functional, system_error, thread, tuple, 
				 // utility
#include <queue> // deque, algorithm, vector
#pragma endregion

// Standard C
#pragma region Standard C
#pragma endregion


// 3rd Party Libs
#pragma region 3rd Party Libs
#ifdef RA_EXPORTS
#include "rapidjson/include/rapidjson/document.h" // reader.h
#include "rapidjson/include/rapidjson/writer.h" // stringbuffer.h
#include "rapidjson/include/rapidjson/filestream.h"
#include "rapidjson/include/rapidjson/error/en.h"
#endif // RA_EXPORTS

#include "md5.h"  
#include "tinyformat.h"
#pragma endregion




// Rarely Chaning Project Files
#include "RA_Resource.h"

#pragma warning(pop)

#endif // !STDAFX_H
