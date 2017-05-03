#include "stdafx.h"
#include "RA_Defs.h"

#include "File_handle.h"

GetParseErrorFunc GetJSONParseErrorStr = GetParseError_En;

void RADebugLogNoFormat( const char* data )
{
	OutputDebugString( Widen( data ).c_str() );

	//SetCurrentDirectory( g_sHomeDir.c_str() );//?

	// There's an explanation for this in _WriteFileToBuffer
	test_file( RA_LOG_FILENAME, "a" );
	auto pf{fopen( RA_LOG_FILENAME, "r" )};

	fwrite( static_cast<const void*>(data), sizeof( char ), strlen( data ), pf );

	def_filehandle* fh{nullptr};
	fh->SafeCloseFile( pf );
	SAFE_DELETE( fh );
} // end function RADebugLogNoFormat

// NB: auto ensures intended types are used, otherwise casting is needed
void RADebugLog( const char* format, ... )
{
	char buf[4096];
	auto p{buf};

	va_list args;
	va_start( args, format );

	// NB: since primitives are self-referential, they have their own constructors.

	// buf-3 is room for CR/LF/NUL
	auto max_count{sizeof buf - size_t{3}};
	auto n{_vsnprintf_s( p, size_t{4096}, max_count, format, args )};
	va_end( args );

	p += (n < 0) ? sizeof buf - 3 : n;

	while ( (p > buf) && (isspace( p[-1] )) )
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p   = '\0';

	OutputDebugString( Widen( buf ).c_str() );

	//SetCurrentDirectory( g_sHomeDir.c_str() );//?
	test_file( RA_LOG_FILENAME, "a" );
	auto pf{fopen( RA_LOG_FILENAME, "r" )};

	fwrite( static_cast<const void*>(buf), sizeof( char ), strlen( buf ), pf );

	def_filehandle* fh{nullptr};
	fh->SafeCloseFile( pf );
	SAFE_DELETE( fh );
} // end function RADebugLog

BOOL DirectoryExists( const char* sPath )
{
	DWORD dwAttrib = GetFileAttributes( Widen( sPath ).c_str() );
	return(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// NB: std::string by definition is a byte stream/byte string, you
//     don't need to do this cast, it'll lead to undefined behavior.
//     That's what iterators are for...

// NB: constexpr is more reliable for compile time checks than static_assert but can't be used
//     since std::string isn't a literal type.
//static_assert(sizeof( BYTE* ) == sizeof( char* ), "dangerous cast ahead");
char* DataStreamAsString( DataStream& stream )
{
	std::string str{stream.begin(), stream.end()};
	return str._Myptr();
} // end function DataStreamAsString

// TODO: Discuss whether we actually need these, if we're only using
//       UTF-8 we don't need these since std::string and std::wstring
//       are UTF-8. We could just use 2 functions instead of four. Is
//       there any really reason why the converters are static?

std::string Narrow( const wchar_t* wstr )
{
	static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t > converter;
	return converter.to_bytes( wstr );
}

std::string Narrow( const std::wstring& wstr )
{
	static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t > converter;
	return converter.to_bytes( wstr );
}

std::wstring Widen( const char* str )
{
	static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t > converter;
	return converter.from_bytes( str );
}

std::wstring Widen( const std::string& str )
{
	static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t > converter;
	return converter.from_bytes( str );
}
