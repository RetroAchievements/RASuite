#include "stdafx.h"
#include "File_handle.h"

#include "RA_Defs.h"

using std::runtime_error;
using gsl::not_null;
using gsl::zstring;
template class File_handle<const char*, char*>;

template<typename CString, typename String>
File_handle<CString, String>::File_handle( CString filename, CString filemode ) :
	fp_{fopen( filename, filemode )}
{
	if ( !fp_ )
	{
		not_null<zstring<BUFSIZ>> err{""};
		WriteErrorMsg( err, filename, filemode );
		throw runtime_error{err};
	} // end if
} // end constructor

template<typename CString, typename String>
File_handle<CString, String>::~File_handle() noexcept
{
	SafeCloseFile( fp_ );
} // end destructor

template<typename CString, typename String>
void File_handle<CString, String>::SafeCloseFile( FILE* fp ) noexcept
{
	Expects( fp );
	fclose( fp );
	Ensures( !fp );
} // end function SafeCloseFile

template<typename CString, typename String>
CString File_handle<CString, String>::WriteErrorMsg( String errmsg, CString filename,
	CString filemode ) noexcept
{
	sprintf( errmsg, "File_handle: could not open %s as %s.", filename, filemode );
	return errmsg;
} // end function WriteErrorMsg

#pragma region Utils
int test_file( const char* filename, const char* mode )
{
	// We need to check if we can read/write to the file
	try
	{
		def_filehandle{filename, mode};
		return ERROR_SUCCESS;
	} // end try
	catch ( const runtime_error& e )
	{
		SetLastError( INVALID_FILE_ATTRIBUTES );
		MessageBox( nullptr, Widen( e.what() ).c_str(), L"Exception", MB_OK );
		return INVALID_FILE_ATTRIBUTES;
	} // end catch
} // end function test_file

void callfh_SafeCloseFile( def_filehandle* fh, FILE* fp )
{
	fh->SafeCloseFile( fp );
	SAFE_DELETE( fh );
} // end function callfh_SafeCloseFile
#pragma endregion
