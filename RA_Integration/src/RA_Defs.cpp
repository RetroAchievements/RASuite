#include "RA_Defs.h"

GetParseErrorFunc GetJSONParseErrorStr = GetParseError_En;

std::string DataStreamAsString(const DataStream& stream)
{
	std::ostringstream oss;

    // Ok the string has to come from a json for this to work
	for ( auto& i : stream )
		oss << i;

    // pesky null character
	auto str{ oss.str() };
    if(!str.empty()) {
		str.pop_back();
	}

    // I SEE THE JSON RESPONSE WHY ISN'T IT WORKING?

    // Is everything not from a json supposed to look like shit?
    // Fucking FINNALY! I swear I did this already but forgot how.
	return str; // ok it's definitly showing how it should be
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
	std::wostringstream out;
	std::string str;
	for ( auto& i : wstr )
		str.push_back(out.narrow(i));

	// test var
	auto temp = str;
    // YES! It goddamn worked!
	return str;
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

std::error_code ra::GetLastErrorCode() noexcept {
	auto dwErrVal{ _CSTD GetLastError() };
	std::error_code ec{ static_cast<int>(dwErrVal), std::system_category() };
	return ec;
}

// Returns the last Win32 error, in string format. Returns an empty string if there
// is no error. This is mainly for throwing std::system_error
tstring ra::GetLastErrorMsg()
{
	//Get the error message, if any.
	auto errorID{ _CSTD GetLastError() };
	if (errorID == 0)
		return tstring{}; //No error message has been recorded

	tstring messageBuffer;
	auto size{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, null, errorID,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			messageBuffer.data(), 0, null) };

	tstring message{ messageBuffer.data(), size };



	return message;
}
