#include "stdafx.h"
#include "dir_handle.h"

int Dir_handle::check_mkdir( const char* dirname )
{
	if ( _mkdir( dirname ) )
		return 0;
	return -1;
} // end function check_mkdir

int calldirh_check_mkdir( Dir_handle* dh, const char* dirname )
{
	return dh->check_mkdir( dirname );
} // end function func
