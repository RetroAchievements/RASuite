#include "common.h"
#include "RA_Defs.h"

GetParseErrorFunc GetJSONParseErrorStr = GetParseError_En;
using namespace std;

// static_assert(sizeof(BYTE*) == sizeof(char*), "dangerous cast ahead");
char* DataStreamAsString(DataStream& stream)
{
	// could do this
	ostringstream oss;
	for ( auto& i : stream )
	{
		oss << i;
	}
	return oss.str().data();
}

std::string Narrow(const wchar_t* wstr)
{
	// could do this
	ostringstream oss;
	wstring wstd_str{ wstr };
	for ( auto& i : wstd_str )
	{
		oss << i;
	}
	return oss.str();
}

std::string Narrow(const std::wstring& wstr)
{
	// could do this
	ostringstream oss;
	for ( auto& i : wstr )
	{
		oss << i;
	}
	return oss.str();
}

std::wstring Widen(const char* str)
{
	// could do this
	wostringstream oss;
	string std_str{ str };
	for ( auto& i : std_str )
	{
		oss << i;
	}
	return oss.str();
}

std::wstring Widen(const std::string& str)
{
	// could do this
	wostringstream oss;
	for ( auto& i : str )
	{
		oss << i;
	}
	return oss.str();
}

std::wstring Widen(const wchar_t* wstr)
{
	return std::wstring(wstr);
}

std::wstring Widen(const std::wstring& wstr)
{
	return wstr;
}

std::string Narrow(const char* str)
{
	return std::string(str);
}

std::string Narrow(const std::string& str)
{
	return str;
}

void RADebugLogNoFormat(const char* data)
{
	OutputDebugString(NativeStr(data).c_str());

	//SetCurrentDirectory( g_sHomeDir.c_str() );//?
	FILE* pf = NULL;
	if (fopen_s(&pf, RA_LOG_FILENAME, "a") == 0)
	{
		fwrite(data, sizeof(char), strlen(data), pf);
		fclose(pf);
	}
}

void RADebugLog(const char* format, ...)
{
	char buf[4096];
	char* p = buf;

	va_list args;
	va_start(args, format);
	int n = _vsnprintf_s(p, 4096, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while ((p > buf) && (isspace(p[-1])))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';

	OutputDebugString(NativeStr(buf).c_str());

	//SetCurrentDirectory( g_sHomeDir.c_str() );//?
	FILE* pf = NULL;
	if (fopen_s(&pf, RA_LOG_FILENAME, "a") == 0)
	{
		fwrite(buf, sizeof(char), strlen(buf), pf);
		fclose(pf);
	}
}

BOOL DirectoryExists(const char* sPath)
{
	DWORD dwAttrib = GetFileAttributes(NativeStr(sPath).c_str());
	return(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
