#include "RA_Defs.h"

GetParseErrorFunc GetJSONParseErrorStr = GetParseError_En;

std::string DataStreamAsString(const DataStream& stream)
{
	std::ostringstream oss;

	for ( auto& i : stream )
		oss << i;

	// Test
	return oss.str(); // ok it's definitly showing how it should be
}

std::string Narrow(const wchar_t* wstr)
{
	std::ostringstream out;
	std::wstring swstr{ wstr };

	for ( auto& i : swstr )
		out << i;

	return out.str();
}

std::string Narrow(const std::wstring& wstr)
{
	std::ostringstream out;

	for ( auto& i : wstr )
		out << i;

	return out.str();
}

std::wstring Widen(const char* str)
{
	std::wostringstream out;
	std::string sstr{ str };

	for ( auto& i : sstr )
		out << i;

	return out.str();
}

std::wstring Widen(const std::string& str)
{
	std::wostringstream out;

	for ( auto& i : str )
		out << i;

	return out.str();
}

std::wstring Widen(const wchar_t* wstr)
{
	return std::wstring{ wstr };
}

std::wstring Widen(const std::wstring& wstr)
{
	return wstr;
}

std::string Narrow(const char* str)
{
	return std::string{ str };
}

std::string Narrow(const std::string& str)
{
	return str;
}

void RADebugLogNoFormat(const char* data)
{
#ifdef _DEBUG
	OutputDebugString(NativeStr(data).c_str());
#endif // _DEBUG

	// streams are safer, and just as fast, but take more space
	std::ofstream ofile{ RA_LOG_FILENAME, std::ios::app };
	ofile << data << "\r\n"; // does it really need this?
}


BOOL DirectoryExists(const char* sPath)
{
	DWORD dwAttrib = GetFileAttributes(NativeStr(sPath).c_str());
	return(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
