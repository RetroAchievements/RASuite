#ifndef RA_DEFS_H
#define RA_DEFS_H
#pragma once

// pch is too complicated now, this will be a makeshift version
// though it's not precompiled

// This stuff is assuming you're using Visual C++, it's in the prereqs anyway

// Windows
// We don't need all of it
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <ShlObj.h>
#include <tchar.h>
#include <WindowsX.h>

// just in-case
#ifdef WIN32_LEAN_AND_MEAN
#include <ShellAPI.h>
#include <MMSystem.h>
#include <CommDlg.h>
#endif // WIN32_LEAN_AND_MEAN


// Standard C++
#include <array> // algorithm and iterator
#include <fstream>
#include <map> // b-trees
#include <mutex> // chrono (time.h), functional, thread (memory)
#include <queue> // deque and vector
#include <unordered_map> // hashing
// with refactoring we could use this later (i.e. achievement_set,
// condition_set)
//#include <unordered_set> 








// We really should put 3rd party libs outside project directory though
// the C98 version has fixed params and might cause errors on parsing 
#define TINYFORMAT_USE_VARIADIC_TEMPLATES 
#include "tinyformat.h" // cassert, iostream, sstream (string)

#define _RA ::ra::

#ifndef RA_EXPORTS

//	Version Information is integrated into tags

#else

//NB. These must NOT be accessible from the emulator!
//#define RA_INTEGRATION_VERSION	"0.053"


#define RAPIDJSON_HAS_STDSTRING 1


//	RA-Only
#pragma warning(push, 1)
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/filestream.h"
#include "rapidjson/include/rapidjson/ostreamwrapper.h"
#include "rapidjson/include/rapidjson/istreamwrapper.h"
#include "rapidjson/include/rapidjson/error/en.h"

// super strech
#include "rapidjson/include/rapidjson/filereadstream.h"
#include "rapidjson/include/rapidjson/filewritestream.h"
#pragma warning(pop)

using namespace rapidjson;
extern GetParseErrorFunc GetJSONParseErrorStr;

using namespace std::string_literals;
using namespace std::chrono_literals;

namespace ra {} // just so it doesn't complain
using namespace ra;
#endif	//RA_EXPORTS

// if you really want to use null instead of nullptr like .net
#ifndef null
#define null nullptr
#endif // !null

// Maybe an extra check just in-case

#if _HAS_CXX17
#define _DEPRECATED          [[deprecated]]
#define _DEPRECATEDR(reason) [[deprecated(reason)]]
#define _FALLTHROUGH         [[fallthrough]]; // the ";" is needed
#define _NORETURN            [[noreturn]]
#define _UNUSED              [[maybe_unused]]
#endif

#define RA_KEYS_DLL						"RA_Keys.dll"
#define RA_PREFERENCES_FILENAME_PREFIX	"RAPrefs_"
#define RA_UNKNOWN_BADGE_IMAGE_URI		"00000"

#define RA_DIR_OVERLAY					".\\Overlay\\"
#define RA_DIR_BASE						".\\RACache\\"
#define RA_DIR_DATA						RA_DIR_BASE##"Data\\"
#define RA_DIR_BADGE					RA_DIR_BASE##"Badge\\"
#define RA_DIR_USERPIC					RA_DIR_BASE##"UserPic\\"
#define RA_DIR_BOOKMARKS				RA_DIR_BASE##"Bookmarks\\"

#define RA_GAME_HASH_FILENAME			RA_DIR_DATA##"gamehashlibrary.txt"
#define RA_GAME_LIST_FILENAME			RA_DIR_DATA##"gametitles.txt"
#define RA_MY_PROGRESS_FILENAME			RA_DIR_DATA##"myprogress.txt"
#define RA_MY_GAME_LIBRARY_FILENAME		RA_DIR_DATA##"mygamelibrary.txt"

#define RA_OVERLAY_BG_FILENAME			RA_DIR_OVERLAY##"overlayBG.png"
#define RA_NEWS_FILENAME				RA_DIR_DATA##"ra_news.txt"
#define RA_TITLES_FILENAME				RA_DIR_DATA##"gametitles.txt"
#define RA_LOG_FILENAME					RA_DIR_DATA##"RALog.txt"


#define RA_HOST_URL						"retroachievements.org"
#define RA_HOST_IMG_URL					"i.retroachievements.org"

#define SIZEOF_ARRAY( ar )	( sizeof( ar ) / sizeof( ar[ 0 ] ) )
#define SAFE_DELETE( x )	{ if( x != nullptr ) { delete x; x = nullptr; } }

using ARGB = DWORD;

//namespace RA
//{

// The function that was here (RAClamp) has a function in the standard
// (std::clamp)

class RARect : public RECT
{
public:
	RARect() {}
	RARect(LONG nX, LONG nY, LONG nW, LONG nH)
	{
		left = nX;
		right = nX + nW;
		top = nY;
		bottom = nY + nH;
	}

public:
	inline int Width() const { return(right - left); }
	inline int Height() const { return(bottom - top); }
};



class RASize
{
public:
	RASize() : m_nWidth(0), m_nHeight(0) {}
	RASize(const RASize& rhs) : m_nWidth(rhs.m_nWidth), m_nHeight(rhs.m_nHeight) {}
	RASize(int nW, int nH) : m_nWidth(nW), m_nHeight(nH) {}

public:
	inline int Width() const { return m_nWidth; }
	inline int Height() const { return m_nHeight; }
	inline void SetWidth(int nW) { m_nWidth = nW; }
	inline void SetHeight(int nH) { m_nHeight = nH; }

private:
	int m_nWidth;
	int m_nHeight;
};

const RASize RA_BADGE_PX(64, 64);
const RASize RA_USERPIC_PX(64, 64);

class ResizeContent
{
public:
	enum AlignType
	{
		NO_ALIGN,
		ALIGN_RIGHT,
		ALIGN_BOTTOM,
		ALIGN_BOTTOM_RIGHT
	};

public:
	HWND hwnd;
	POINT pLT;
	POINT pRB;
	AlignType nAlignType;
	int nDistanceX;
	int nDistanceY;
	bool bResize;

	ResizeContent(HWND parentHwnd, HWND contentHwnd, AlignType newAlignType, bool isResize)
	{
		hwnd = contentHwnd;
		nAlignType = newAlignType;
		bResize = isResize;

		RARect rect;
		GetWindowRect(hwnd, &rect);

		pLT.x = rect.left;	pLT.y = rect.top;
		pRB.x = rect.right; pRB.y = rect.bottom;

		ScreenToClient(parentHwnd, &pLT);
		ScreenToClient(parentHwnd, &pRB);

		GetWindowRect(parentHwnd, &rect);
		nDistanceX = rect.Width() - pLT.x;
		nDistanceY = rect.Height() - pLT.y;

		if (bResize)
		{
			nDistanceX -= (pRB.x - pLT.x);
			nDistanceY -= (pRB.y - pLT.y);
		}
	}

	void Resize(int width, int height)
	{
		int xPos, yPos;

		switch (nAlignType)
		{
		case ResizeContent::ALIGN_RIGHT:
			xPos = width - nDistanceX - (bResize ? pLT.x : 0);
			yPos = bResize ? (pRB.y - pLT.x) : pLT.y;
			break;
		case ResizeContent::ALIGN_BOTTOM:
			xPos = bResize ? (pRB.x - pLT.x) : pLT.x;
			yPos = height - nDistanceY - (bResize ? pLT.y : 0);
			break;
		case ResizeContent::ALIGN_BOTTOM_RIGHT:
			xPos = width - nDistanceX - (bResize ? pLT.x : 0);
			yPos = height - nDistanceY - (bResize ? pLT.y : 0);
			break;
		default:
			xPos = bResize ? (pRB.x - pLT.x) : pLT.x;
			yPos = bResize ? (pRB.y - pLT.x) : pLT.y;
			break;
		}

		if (!bResize)
			SetWindowPos(hwnd, NULL, xPos, yPos, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
		else
			SetWindowPos(hwnd, NULL, 0, 0, xPos, yPos, SWP_NOMOVE | SWP_NOZORDER);
	}
};

enum AchievementSetType
{
	Core,
	Unofficial,
	Local,

	NumAchievementSetTypes
};

// These here as aliases for fallbacks, incase there isn't a template
using DataStream         = std::basic_string<BYTE>;
using Data_stringstream  = std::basic_stringstream<BYTE>;
using Data_istringstream = std::basic_istringstream<BYTE>;
using Data_ostringstream = std::basic_istringstream<BYTE>;
using Data_fstream       = std::basic_fstream<BYTE>;
using Data_ifstream      = std::basic_ifstream<BYTE>;
using Data_ofstream      = std::basic_ofstream<BYTE>;
using Data_iostream      = std::basic_iostream<BYTE>;
using Data_istream       = std::basic_ostream<BYTE>;
using Data_ostream       = std::basic_istream<BYTE>;






using ByteAddress   = std::size_t;
using AchievementID = std::size_t;
using LeaderboardID = std::size_t;
using GameID        = std::size_t;

// just call c_str() as needed
std::string DataStreamAsString(const DataStream& stream);

extern void RADebugLogNoFormat(const char* data);


// This function was a disaster, all those lines into two...
template<typename... Args>
void RADebugLog(const char* fmt, const Args&... args)
{
	// it couldn't deduce it
	std::string buf{ tfm::format(fmt, args...) };
	RADebugLogNoFormat(buf.c_str());
}

extern BOOL DirectoryExists(const char* sPath);

constexpr int SERVER_PING_DURATION{ 2 * 60 };
//};
//using namespace RA;

#define RA_LOG RADebugLog

#ifdef _DEBUG
#undef ASSERT
#define ASSERT( x ) assert( x )
#else
#undef ASSERT
#define ASSERT( x ) {}
#endif

#ifndef UNUSED
#define UNUSED( x ) ( x );
#endif

extern std::string Narrow(const wchar_t* wstr);
extern std::string Narrow(const std::wstring& wstr);
extern std::wstring Widen(const char* str);
extern std::wstring Widen(const std::string& str);

//	No-ops to help convert:
//	No-ops to help convert:
extern std::wstring Widen(const wchar_t* wstr);
extern std::wstring Widen(const std::wstring& wstr);
extern std::string Narrow(const char* str);
extern std::string Narrow(const std::string& wstr);




#ifdef UNICODE
#define NativeStr(x) Widen(x)
#define NativeStrType std::wstring
#else
#define NativeStr(x) Narrow(x)
#define NativeStrType std::string
#endif


// Stuff in the namespace is to prevent conflicts
// inside the RA_Integration project it won't matter that much
// but you'll have to use the namespace for emus
namespace ra {
using cstring = const char*;

// This is just an extra step, would need either reorder or put in new file
template<typename CharT, class = _STD enable_if_t<is_char_v<CharT>>>
using string_t = _STD basic_string<CharT>;

// I really hope this doesn't cause linker errors
// Anyway if you put any type besides what's allowed in is_char, you'll get an error
// uncomment the bottom next two lines for an example

//using something = string_t<int>;
//static_assert(is_char_v<something::value_type>);

// Needs work for the alias template
using tstring     = _STD basic_string<TCHAR>;
using ctstring    = const _STD basic_string<TCHAR>;
using cstd_string = const _STD string;
using cstd_wtring = const _STD wstring;

// if you are already in ra or type "using namespace ra" that's good enough
inline namespace int_literals
{

// Don't feel like casting non-stop
// There's also standard literals for strings on clock types

// Use it if you need an unsigned int 
// Not using _s because that's the literal for std::string
// usage: auto a{19_z};
inline constexpr _STD size_t operator""_z(unsigned long long n) noexcept
{
	return static_cast<_STD size_t>(n);
} // end operator""_z

// Use it if you need a signed int
// usage: auto a{9_i};
inline constexpr _STD intptr_t operator""_i(unsigned long long n) noexcept
{
	return static_cast<_STD intptr_t>(n);
} // end operator""_i

// We need one for DWORD, because it doesn't match LPDWORD for some stuff
inline constexpr _CSTD DWORD operator""_dw(unsigned long long n) noexcept
{
	return static_cast<DWORD>(n);
} // end operator""_dw

// streamsize varies as well
inline constexpr _STD streamsize operator""_ss(unsigned long long n) noexcept
{
	return static_cast<_STD streamsize>(n);
} // end operator""_ss


} // inline namespace int_literals




#pragma region Type Validation
/// <summary>
///   A check to tell whether a type is a known character type.
/// </summary>
/// <typeparam name="CharT">A type to be evalualted</typeparam>
/// <typeparam name="">
///   A check, if it does not return void, it is false.
/// </typeparam>
/// <remarks>This struct can be used in static assertions.</remarks>
template<typename CharT, class = _STD void_t<>>
struct is_char : _STD false_type {};

/// <summary>
///   A check to tell whether a type is a known character type. If this
///   function was reached, <typeparamref name="CharT" /> is a known character
///   type.
/// </summary>
/// <typeparam name="CharT">A type to be evalualted</typeparam>
template<typename CharT>
struct is_char<CharT, _STD enable_if_t<_STD is_integral_v<CharT> &&
	(_STD is_same_v<CharT, char> || _STD is_same_v<CharT, wchar_t> ||
		_STD is_same_v<CharT, char16_t> || _STD is_same_v<CharT, char32_t> ||
		_STD is_same_v<CharT, unsigned char>)
	>> :
	_STD true_type {};

// is_char helper function
template<typename CharT>
inline constexpr bool is_char_v = is_char<CharT>::value;

// for the hell of it
template<typename StringType, class = _STD void_t<>>
struct is_string : _STD false_type {};

template<typename StringType>
struct is_string<StringType, _STD enable_if_t<is_char_v<
	typename _STD char_traits<typename StringType::value_type>::char_type>>
> :	_STD true_type{};

template<typename StringType>
inline constexpr bool is_string_v = is_string<StringType>::value;

#pragma endregion


// why is not in the standard? the world will never know...
// crap I forgot the standard moved the aliases to char_traits

// This function was to not need overloads for every character type as long as
// the file stream has a character type it will work valid character types are
// char, wchar_t, unsigned char, char16_t, and char32_t
// see ra::is_char is for more details

/// <summary>Calculates the size of any standard fstream.</summary>
/// <param name="filename">The filename.</param>
/// <typeparam name="CharT">
///   The character type, it should be auto deduced if valid. Otherwise you'll
///   get an intellisense error.
/// </typeparam>
/// <typeparam name="Traits">
///   The character traits of <typeparamref name="CharT" />.
/// </typeparam>
/// <returns>The size of the file stream.</returns>
template<
	typename CharT,
	typename Traits = _STD char_traits<CharT>,
	class = _STD enable_if_t<is_char_v<CharT>>
>
typename Traits::pos_type filesize(
	string_t<CharT>& filename) {
	_STD basic_fstream<CharT> file{
		filename, _STD ios::ate | _STD ios::binary
	};
	return file.tellg();
} // end function filesize


#pragma region Conversions
// NOW I remember, you can use std::to_string instead starting with c++11.
template<typename CharT>
std::basic_string<CharT> TimeToString(time_t the_time) noexcept
{
	std::basic_ostringstream<CharT> oss;
	ss << the_time;
	return ss.str();
}

template<typename Enum, class = std::enable_if_t<std::is_enum_v<Enum>>>
inline constexpr auto etoi(Enum e) -> std::underlying_type_t<Enum>
{
	return static_cast<std::underlying_type_t<Enum>>(e);
}

// alias
template<typename Enum>
inline constexpr auto to_integral = etoi<Enum>;


/// <summary>
///   Converts a standard string with a signed character type to a
///   <c>DataStream</c>.
/// </summary>
/// <param name="str">The string.</param>
/// <typeparam name="CharT">
///   The character type, must be signed or you will get an intellisense error.
/// </typeparam>
/// <returns><paramref name="str" /> as a <c>DataStream</c>.</returns>
/// <remarks>
///   <c>DataStream</c> is just an alias for an unsigned standard string.
/// </remarks>
template<
	typename CharT,
	class = std::enable_if_t<is_char_v<CharT> && std::is_signed_v<CharT>>
>
DataStream stodata_stream(const std::basic_string<CharT>& str) noexcept
{
	Data_ostringstream doss;
	for (auto& i : str)
		doss << i;

	return doss.str();
} // end function stodata_stream



#pragma endregion


// Stuff down here is to allow the use of strongly typed enums while still able to compare
// They do work as intended but there's too many logic errors so it'll  be commented out for now
// The SIZEOF_ARR is NOT a constant expression


// For something to be EqualityComparable, A == B, B == A must be implicitly
// convertible to bool
//#pragma region EqualityComparable
//template<
//    typename EqualityComparable,
//    typename EqualityComparable2,
//    class = std::void_t<>
//>
//struct is_equality_comparable : std::false_type {};
//
//
//template<
//    typename EqualityComparable,
//    typename EqualityComparable2
//>
//struct is_equality_comparable<EqualityComparable, EqualityComparable2,
//    std::enable_if_t<true, 
//    decltype(std::declval<EqualityComparable&>() == std::declval<EqualityComparable2&>())
//    >> : std::true_type {};
//
//template<typename EqualityComparable, typename EqualityComparable2 = EqualityComparable>
//constexpr bool is_equality_comparable_v = is_equality_comparable<EqualityComparable, EqualityComparable2>::value;
//
//template<typename EqualityComparable, typename EqualityComparable2 = EqualityComparable>
//constexpr auto operator==(const EqualityComparable& eq, const EqualityComparable2& eq2) noexcept
//-> decltype(std::declval<EqualityComparable&>() == std::declval<EqualityComparable2&>())
//{
//    return{ eq == eq2 };
//}
//#pragma endregion
//
//#pragma region LessThanComparable 
//template<
//    typename LessThanComparable,
//    typename LessThanComparable2,
//    class = std::void_t<>
//>
//struct is_lessthan_comparable : std::false_type {};
//
//
//template<
//    typename LessThanComparable,
//    typename LessThanComparable2
//>
//struct is_lessthan_comparable<LessThanComparable, LessThanComparable2,
//    std::enable_if_t<true,
//    decltype(std::declval<LessThanComparable&>() < std::declval<LessThanComparable2&>())
//    >> : std::true_type {};
//
//template<typename LessThanComparable, typename LessThanComparable2 = LessThanComparable>
//constexpr bool is_lessthan_comparable_v = is_lessthan_comparable<LessThanComparable, LessThanComparable2>::value;
//
//template<typename LessThanComparable, typename LessThanComparable2 = LessThanComparable>
//constexpr auto operator<(const LessThanComparable& a, const LessThanComparable2& b) noexcept
//-> decltype(std::declval<LessThanComparable&>() == std::declval<LessThanComparable2&>())
//{
//    static_assert(is_lessthan_comparable_v<LessThanComparable, LessThanComparable2>);
//    return{ a < b };
//}
//
//// Now that we have those the rest are handled by the standard
//using namespace std::rel_ops;





#pragma endregion


} // namespace ra


//  // Seems we need more
//template<
//    typename Enum,
//    typename Integral,
//    class = std::enable_if_t<std::is_enum_v<Enum> && std::is_integral_v<Integral>>>
//    constexpr auto operator|(const Enum& a, const Integral& b) noexcept
//    -> decltype(std::declval<Enum&>() | std::declval<Integral&>())
//{
//    auto result{ etoi(a) | b };
//    return result;
//}
//
//
//
//// Seems we need more
//template<
//    typename Enum,
//    typename Integral,
//    class = std::enable_if_t<std::is_enum_v<Enum> && std::is_integral_v<Integral>>>
//    constexpr auto operator|=(const Enum& a, const Integral& b) noexcept
//    -> decltype(std::declval<Enum&>() |= std::declval<Integral&>())
//{
//    auto result{ etoi(a) |= b };
//    return result;
//}


#endif // !RA_DEFS_H
