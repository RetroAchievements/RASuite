#ifndef RA_CORE_H
#define RA_CORE_H
#pragma once

#include "RA_Defs.h"
#include "RA_Interface.h"




#ifdef RA_EXPORTS
#define API __declspec(dllexport)
#else
#define API
#endif

// since c functions cannot throw, we should put noexcept to optimize the runtime

#ifdef __cplusplus
extern "C" {
#endif

    //	Fetch the version number of this integration version.
    API const char* CCONV _RA_IntegrationVersion() noexcept;

    //	Initialize all data related to RA Engine. Call as early as possible.
    API int CCONV _RA_InitI(HWND hMainHWND, /*enum EmulatorID*/int nConsoleID, const char* sClientVersion) noexcept;

    //	Call for a tidy exit at end of app.
    API int CCONV _RA_Shutdown() noexcept;

    //	Allocates and configures a popup menu, to be called, embedded and managed by the app.
    API HMENU CCONV _RA_CreatePopupMenu() noexcept;

    //	Check all achievement sets for changes, and displays a dlg box to warn lost changes.
    API bool CCONV _RA_ConfirmLoadNewRom(bool bQuittingApp) noexcept;

    //	On or immediately after a new ROM is loaded, including if the ROM is reset.
    API int CCONV _RA_OnLoadNewRom(const BYTE* pROM, unsigned int nROMSize) noexcept;

    //	On or immediately after a new ROM is loaded, for each memory bank found
    //	NB:
    //pReader is typedef unsigned char (_RAMByteReadFn)( size_t nOffset );
    //pWriter is typedef void (_RAMByteWriteFn)( unsigned int nOffs, unsigned int nVal );
    API void CCONV _RA_InstallMemoryBank(int nBankID, void* pReader, void* pWriter, int nBankSize) noexcept;

    //	Call before installing any memory banks
    API void CCONV _RA_ClearMemoryBanks() noexcept;

    //	Immediately after loading a new state.
    API void CCONV _RA_OnLoadState(const char* sFileName) noexcept;

    //	Immediately after saving a new state.
    API void CCONV _RA_OnSaveState(const char* sFileName) noexcept;


    //	Perform one test for all achievements in the current set. Call this once per frame/cycle.
    API void CCONV _RA_DoAchievementsFrame() noexcept;


    //	Use in special cases where the emulator contains more than one console ID.
    API void CCONV _RA_SetConsoleID(unsigned int nConsoleID) noexcept;

    //	Display a dialog helping the user get the latest RA client version
    API BOOL CCONV _RA_OfferNewRAUpdate(const char* sNewVer) noexcept;

    //	Deal with any HTTP results that come along. Call per-cycle from main thread.
    API int CCONV _RA_HandleHTTPResults() noexcept;

    //	Execute a blocking check to see if this client is out of date.
    API void CCONV _RA_CheckForUpdate() noexcept;

    

    //	Update the title of the app
    API void CCONV _RA_UpdateAppTitle(const char* sMessage = nullptr) noexcept;

    //	Load preferences from ra_prefs.cfg
    API void CCONV _RA_LoadPreferences() noexcept;

    //	Save preferences to ra_prefs.cfg
    API void CCONV _RA_SavePreferences() noexcept;

    //	Display or unhide an RA dialog.
    API void CCONV _RA_InvokeDialog(LPARAM nID) noexcept;

    //	Call this when the pause state changes, to update RA with the new state.
    API void CCONV _RA_SetPaused(bool bIsPaused) noexcept;

    //	Returns the currently active user
    API const char* CCONV _RA_Username() noexcept;

    //	Attempt to login, or present login dialog.
    API void CCONV _RA_AttemptLogin(bool bBlocking) noexcept;

    //	Return whether or not the hardcore mode is active.
    API int CCONV _RA_HardcoreModeIsActive() noexcept;

    //	Return whether a HTTPGetRequest already exists
    
    _DEPRECATED API int  CCONV _RA_HTTPGetRequestExists(const char* sPageName) noexcept;

    // The calling convention was explictly put it to prevent mishaps
    //	Install user-side functions that can be called from the DLL
    API void CCONV _RA_InstallSharedFunctions(bool(CCONV* fpIsActive)(void), void(CCONV* fpCauseUnpause)(void), void(CCONV* fpRebuildMenu)(void), void(CCONV* fpEstimateTitle)(char*), void(CCONV* fpResetEmulation)(void), void(CCONV* fpLoadROM)(const char*)) noexcept;
    API void CCONV _RA_InstallSharedFunctionsExt(bool(CCONV* fpIsActive)(void), void(CCONV* fpCauseUnpause)(void), void(CCONV* fpCausePause)(void), void(CCONV* fpRebuildMenu)(void), void(CCONV* fpEstimateTitle)(char*), void(CCONV* fpResetEmulation)(void), void(CCONV* fpLoadROM)(const char*)) noexcept;

#ifdef __cplusplus
}
#endif


//	Non-exposed:
extern std::string g_sKnownRAVersion;
extern std::string g_sHomeDir;
extern std::string g_sROMDirLocation;
extern std::string g_sCurrentROMMD5;

extern HINSTANCE g_hRAKeysDLL;
extern HMODULE g_hThisDLLInst;
extern HWND g_RAMainWnd;
extern EmulatorID g_EmulatorID;
extern ConsoleID g_ConsoleID;
extern const char* g_sGetLatestClientPage;
extern const char* g_sClientVersion;
extern const char* g_sClientName;
extern bool g_bRAMTamperedWith;
extern bool g_bHardcoreModeActive;
extern bool g_bLeaderboardsActive;
extern bool g_bLBDisplayNotification;
extern bool g_bLBDisplayCounter;
extern bool g_bLBDisplayScoreboard;
extern unsigned int g_nNumHTTPThreads;

//	Read a file to a malloc'd buffer. Returns NULL on error. Owner MUST free() buffer if not NULL.
// won't be null now
extern char* _MallocAndBulkReadFileToBuffer(const char* sFilename, std::streamoff nFileSizeOut);

//	Read file until reaching the end of the file, or the specified char.
extern BOOL _ReadTil(const char nChar, char buffer[], unsigned int nSize, DWORD* pCharsRead, FILE* pFile);

//	Read a string til the end of the string, or nChar. bTerminate==TRUE replaces that char with \0.
extern char* _ReadStringTil(char nChar, char*& pOffsetInOut, BOOL bTerminate);

//	Write out the buffer to a file
extern void _WriteBufferToFile(const std::string& sFileName, const DataStream& rawData);
extern void _WriteBufferToFile(const std::string& sFileName, const Document& doc);
extern void _WriteBufferToFile(const std::string& sFileName, const std::string& sString);
extern void _WriteBufferToFile(const char* sFile, const BYTE* sBuffer, int nBytes);

//	Fetch various interim txt/data files
extern void _FetchGameHashLibraryFromWeb();
extern void _FetchGameTitlesFromWeb();
extern void _FetchMyProgressFromWeb();

extern bool _FileExists(const std::string& sFileName);

extern std::string _TimeStampToString(time_t nTime);

extern std::string GetFolderFromDialog();

extern BOOL RemoveFileIfExists(const std::string& sFilePath);

BOOL CanCausePause();

void RestoreWindowPosition(HWND hDlg, const char* sDlgKey, bool bToRight, bool bToBottom);
void RememberWindowPosition(HWND hDlg, const char* sDlgKey);
void RememberWindowSize(HWND hDlg, const char* sDlgKey);



namespace ra {


// Stuff down here was to address repetition
_EXTERN_C
int CALLBACK no_rom_loaded() noexcept;
_END_EXTERN_C


// leave HWND blank to get the active window or specfiy it
// It's a template function, for simplicity it has to be in the header,
// You could do other stuff, but it's tedious.
// Type validation is in place to prevent this function being used incorrectly
template<
	typename StringType, 
	class = _STD enable_if_t<_RA is_string_v<StringType>>
>
int CALLBACK show_error(const StringType& msg, HWND hwnd = null) noexcept
{
	if (!hwnd)
		hwnd = GetActiveWindow();
	return MessageBox(hwnd, TEXT(msg.c_str()), TEXT("Error!"), MB_OK);
} // end function show_error

_STD string CCONV prefs_filename() noexcept;
} // namespace ra
#endif // !RA_CORE_H
