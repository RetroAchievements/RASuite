#ifndef DLG_GAMETITLE_H
#define DLG_GAMETITLE_H
#pragma once

#include "RA_Defs.h"
#include "IRA_Dialog.h"

class Dlg_GameTitle : public IRA_Dialog
{
	using GameTtiles = std::map<std::string, GameID>;
public:
	Dlg_GameTitle();
	~Dlg_GameTitle() noexcept;

	// This calls DoModal internally
	void GetInfoFromModal(std::string& sMD5InOut,
		std::string& sEstimatedGameTitleInOut, GameID nGameIDOut);

	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) override;

	



protected:
	void OnOK(HWND hDlg) override;
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) override;
	void OnKnownGames(const UINT codeNotify);
	void OnGameTitle(HWND hDlg, HWND hwndCtl, UINT codeNotify);
	// Inherited via IRA_Dialog
	virtual INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
		LPARAM lParam) override;

private:
	// This strangely got initilized before construction
	GameID m_nReturnedGameID{ 0 };
	std::string m_sMD5;
	std::string m_sEstimatedGameTitle;

	// don't intialize the HWNDs contrustor, weird I know but Win32 isn't in C++
	// We'd have to make smart pointers for them but that'll take too long
	HWND hKnownGamesCbo{ nullptr }; 
	HWND game_title{ nullptr };
	HWND checksum{ nullptr };
	GameTtiles m_aGameTitles;
	bool bUpdatingTextboxTitle{ false };

	std::string CleanRomName(const std::string& sTryName) noexcept;
	void SetTitles(int nSel, std::string& sGameTitleTidy) noexcept;
	void PopulateTitles(const rapidjson::Value& Data) noexcept;
};

#endif // !DLG_GAMETITLE_H