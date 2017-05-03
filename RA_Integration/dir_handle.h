#pragma once
#ifndef DIR_HANDLE_H
#define DIR_HANDLE_H

/// <summary>
///   Handles directory operations that are not currently handled.
/// </summary>
/// <remarks>
///   This class addresses the issue of ignoring the specification of
///   _mkdir.
/// </remarks>
class Dir_handle
{
public:

	/// <summary>
	/// Checks the return value _mkdir
	/// </summary>
	/// <param name="dirname">Path for a new directory.</param>
	/// <returns>0 on success, and -1 on failure.</returns>
	int check_mkdir( const char* dirname );
};

extern Dir_handle* dir_handle;

// C Wrapper for RA_Core
extern "C" {
	int calldirh_check_mkdir( Dir_handle* dh, const char* dirname );
} // end C-Linkage

#endif // !DIR_HANDLE_H
