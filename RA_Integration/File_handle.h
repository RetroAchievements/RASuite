#pragma once
#ifndef FILE_HANDLE_H
#define FILE_HANDLE_H

/// <summary>
///   A handler for file pointers.
/// </summary>
/// <typeparam name="CString">
///   A type of immutable string, defaults to <c>const char*</c>.
/// </typeparam>
/// <typeparam name="String">
///   A type of mutable string, defaults to <c>char*</c>.
/// </typeparam>
/// <remarks>
///   Copying, moving, and default construction are deleted because
///   they would invalidate the purpose of an error handle.
/// </remarks>
template<typename CString, typename String>
class File_handle
{
public:

	/// <summary>
	///   Initializes a new instance of the <see cref="File_handle" />
	///   class.
	/// </summary>
	/// <param name="filename">The filename.</param>
	/// <param name="filemode">
	///   The mode of operation applied to <see cref="fp" />.
	/// </param>
	/// <exception cref="runtime_error">
	///   Thrown if <paramref name="filename" /> can't be opened for
	///   <paramref name="filemode" />.
	/// </exception>
	/// <remarks>
	///   Constructor is non-transforming to disambiguate it from a
	///   <typeparamref name="CString" />.
	/// </remarks>
	explicit File_handle( CString filename, CString filemode );

	/// <summary>
	///   Finalizes an instance of the <see cref="File_handle" /> class.
	/// </summary>
	/// <remarks>
	///   The destructor does not throw exceptions, however, if the post or
	///   preconditions are violated, the program will terminate.
	/// </remarks>
	~File_handle() noexcept;

	/// <summary>
	///   Closes a <c>FILE*</c> safely.
	/// </summary>
	/// <remarks>
	///   This will be used in a C Wrapper function so we don't need an
	///   instance of <see cref="File_handle" />.
	/// </remarks>
	void SafeCloseFile( FILE* fp ) noexcept;

	/// <summary>
	///   Writes an error message when the exception is thrown.
	/// </summary>
	/// <param name="errmsg">The exception message.</param>
	/// <param name="filename">The filename.</param>
	/// <param name="filemode">The filemode.</param>
	/// <returns>The exception message.</returns>
	CString WriteErrorMsg( String errmsg, CString filename, CString filemode ) noexcept;

	File_handle() noexcept = delete;
	File_handle( const File_handle& ) = delete;
	File_handle& operator=( const File_handle& ) = delete;
	File_handle( File_handle&& ) = delete;
	File_handle& operator=( File_handle&& ) = delete;

private:
	FILE* fp_{nullptr};
};

extern template class File_handle<const char*, char*>;
using def_filehandle = File_handle<const char*, char*>;

#pragma region Utils

/// <summary>
///   Tests whether or not the file mode is acceptable, a runtime_error
///   exception will be caught, the exception will be reported to the
///   user in a message box, and a system error will be returned.
/// </summary>
/// <param name="filename">
///   Refer to <see cref="File_handle" />.
/// </param>
/// <param name="mode">Refer to <see cref="File_handle" />.</param>
/// <returns>A system error code.</returns>
int test_file( const char* filename, const char* mode );

extern "C" {

	/// <summary>
	///   Calls <see cref="File_handle{String}:SafeCloseFile" />.
	/// </summary>
	/// <param name="fh">
	///   A pointer to a <see cref="File_handle{String}" /> instance.
	/// </param>
	/// <remarks>
	///   This is needed since destructors don't exist in C. Also it
	///   can't be marked as noexcept.
	/// </remarks>
	void callfh_SafeCloseFile( def_filehandle* fh, FILE* fp );
} // end C Linkage
#pragma endregion

#endif // !FILE_HANDLE_H
