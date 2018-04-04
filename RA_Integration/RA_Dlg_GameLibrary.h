#ifndef DLG_GAMELIBRARY_H
#define DLG_GAMELIBRARY_H
#pragma once

#include "RA_Defs.h"
#include "IRA_Dialog.h"

namespace ra {

struct GameEntry
{
	std::string m_sTitle;
	std::string m_sFilename;
	GameID m_nGameID{ 0_z };
};

class Dlg_GameLibrary : public IRA_Dialog
{
	// readability
	using FileQueue        = std::deque<std::string>;
	using GameEntries      = std::vector<GameEntry>;
    // OK I got it, this is supposed to be a hash table but you're using a tree
    // plus with how big all these files are you might as well make all of the hash tables
    // To speed up the average lookup time
	using GameHashLib      = std::unordered_map<std::string, GameID>;
	using GameTitleLib     = std::unordered_map<GameID, std::string>;
	using ProgressLib      = std::unordered_map<GameID, std::string>;

    // going to make these two regualar ones and see if that changes anything
	using ResultMap        = std::map<std::string, std::string>;
	using VisibleResultMap = std::map<std::string, std::string>;
public:
	Dlg_GameLibrary();

	/// <inheritdoc />
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus,
		LPARAM lParam) override;
	void AddTitle(GameEntry&& game_entry);
	void ClearTitles();

	void LoadAll();
	void SaveAll();

	void KillThread();
	HWND GetWindow() const  { return m_hGameLibWnd; }
	void SetWindow() {m_hGameLibWnd = Create();}
	BOOL Show() { return ShowWindow(m_hGameLibWnd, SW_SHOW); }
	// Just in case we want to further subclass this dialog
protected:
	/// <inheritdoc />
	void OnNCDestroy(HWND hwnd) override;

	/// <inheritdoc />
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) override;

	/// <inheritdoc />
	void OnTimer(HWND hwnd, UINT id) override;

	/// <inheritdoc />
	LRESULT OnNotify(HWND hwnd, int idFrom, NMHDR* pnmhdr) override;

	/// <inheritdoc />
	void OnOK(HWND hwnd) override;

	// The use of this message here makes no sense to me
	/// <inheritdoc />
	void OnPaint([[maybe_unused]] HWND hwnd) override;


	// Remarks: There's no default handler for WM_USER since it's a private
	//          message for private window classes, You'll have to define it
	//          here and call it in DialogProc since it will never be
	//          intercepted.
	/// <inheritdoc />
	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;

private:
    // path NEEDS to passed by reference or it will be erased!
	bool ListFiles(std::string path, std::string mask,
		_Out_ FileQueue& rFileList);
	// These three functions are almost identical
	void ParseGameHashLibraryFromFile(_Out_ GameHashLib& gamehash_libary);
	void ParseGameTitlesFromFile(_Out_ GameTitleLib& gametitle_libary);
	void ParseMyProgressFromFile(_Out_ ProgressLib& progress_library);
	void SetupColumns(HWND hList);

	// This function was too complex and broken up
	void ReloadGameListData();

	// can't reduce complexity (McCabe of 23), if you can go ahead
	// It's still relatively fast based on what's needed
	// sRomDir should be an "in" parameter but there's that narrow used
	void ReloadByConsoleId(_Out_ bool& bOK, _Out_ tstring& sRomDir);

	// Not sure about the naming but function is far less complex now
	void ScanAndAddRomsRecursive(const std::string& sBaseDir);
	void TryToParseFile(WIN32_FIND_DATA& ffd,
		const std::string& sFilename, const std::string& sBaseDir,
		DataStream& sROMRawData);
	void PrepareToAddGame(const HANDLE& hROMReader,
		DataStream& sROMRawData, std::string& sAbsFileDir);
	void AddGameFromHashLibrary(const std::string& sHashOut,
		std::string& sAbsFileDir);


	bool LaunchSelected();
	void RefreshList();
	void ThreadedScanProc();
	void OnLbxGamelist();
	void OnPickRomDir();
	void OnRescan();



	// do we really need a double ended queue? wouldn't a stack be more
	// appropriate? LIFO?
	FileQueue FilesToScan;
	ResultMap Results;			        //	filepath,md5 (parsed/persisted)
	VisibleResultMap VisibleResults;	//	filepath,md5 (added to renderable)
	size_t nNumParsed{ 0_z };


	bool ThreadProcessingAllowed{ true };
	bool ThreadProcessingActive{ false };

	GameHashLib m_GameHashLibrary;
	GameTitleLib m_GameTitlesLibrary;
	ProgressLib m_ProgressLibrary;
	GameEntries m_vGameEntries;

	HWND m_hList{ nullptr };
	HWND m_hRomDir{ nullptr };
	HWND m_hGlibName{ nullptr };
	HWND m_hScannerFoundInfo{ nullptr };
	HWND m_hGameLibWnd{ nullptr };
};
// OK it is modeless, has to be global
// Call PostNcDestroy near the end

} // namespace ra

extern _RA Dlg_GameLibrary g_GameLibrary;

#endif // !DLG_GAMELIBRARY_H
