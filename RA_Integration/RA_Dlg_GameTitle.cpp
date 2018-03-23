#include "RA_Dlg_GameTitle.h"




#include "RA_Core.h"
#include "RA_Resource.h"
#include "RA_User.h"
#include "RA_Achievement.h"
#include "RA_AchievementSet.h"
#include "RA_httpthread.h"
#include "RA_GameData.h"


INT_PTR Dlg_GameTitle::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	return 0;
}

// this is going to be kind of strange
Dlg_GameTitle::Dlg_GameTitle() :
	IRA_Dialog{ IDD_RA_GAMETITLESEL }
{

}

Dlg_GameTitle::~Dlg_GameTitle() noexcept
{
	OnClose(hKnownGamesCbo);
	OnClose(game_title);
	OnClose(checksum);
}

void Dlg_GameTitle::GetInfoFromModal(std::string & sMD5InOut, std::string & sEstimatedGameTitleInOut, GameID nGameIDOut)
{
	// is this actually possible?
	if ( sMD5InOut.length() == 0 )
		return;

	if ( !RAUsers::LocalUser().IsLoggedIn() )
		return;

	m_sMD5                = sMD5InOut;
	m_sEstimatedGameTitle = sEstimatedGameTitleInOut;
	m_nReturnedGameID     = 0;

	DoModal();

	// Found out it's purpose
	sMD5InOut = m_sMD5;
	sEstimatedGameTitleInOut = m_sEstimatedGameTitle;
	nGameIDOut = m_nReturnedGameID;
}

std::string Dlg_GameTitle::CleanRomName(const std::string& sTryName) noexcept
{
	std::ostringstream sstr;

	//	Scan through, reform sRomNameRef using all logical characters
	for ( auto& i : sTryName )
	{
		if ( i == '\0' )
			break;
		if ( __isascii(i) )
			sstr << i;
	}

	return sstr.str();
}

BOOL Dlg_GameTitle::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	hKnownGamesCbo = GetDlgItem(hDlg, IDC_RA_KNOWNGAMES);
	game_title     = GetDlgItem(hDlg, IDC_RA_GAMETITLE);
	checksum       = GetDlgItem(hDlg, IDC_RA_CHECKSUM);

	std::string sGameTitleTidy = CleanRomName(m_sEstimatedGameTitle);
	SetText(game_title, NativeStr(sGameTitleTidy).c_str());

	//	Load in the checksum
	SetText(checksum, NativeStr(m_sMD5).c_str());

	//	Populate the dropdown
	//	***Do blocking fetch of all game titles.***
	int nSel = ComboBox_AddString(hKnownGamesCbo,
		NativeStr("<New Title>").c_str());
	ComboBox_SetCurSel(hKnownGamesCbo, nSel);

	PostArgs args{ {'c', std::to_string(g_ConsoleID)} };

	Document doc;
	if ( RAWeb::DoBlockingRequest(RequestGamesList, args, doc) )
	{
		const Value& Data = doc["Response"];

		// For all data responses to this request, populate our m_aGameTitles
		// map

		PopulateTitles(Data);
		SetTitles(nSel, sGameTitleTidy);
	}

	return TRUE;
}

void Dlg_GameTitle::SetTitles(int nSel, std::string& sGameTitleTidy) noexcept
{
	// This one can use a ranged for
	for ( auto& i : m_aGameTitles )
	{
		const auto& sTitle{ i.first };
		nSel = ComboBox_AddString(hKnownGamesCbo,
			NativeStr(sTitle).c_str());

		// Attempt to find this game and select it by default: case
		// insensitive comparison
		if ( sGameTitleTidy.compare(sTitle) == 0 )
		{
			ComboBox_SetCurSel(hKnownGamesCbo, nSel);
		}
	}
}

void Dlg_GameTitle::PopulateTitles(const rapidjson::Value& Data) noexcept
{
	for ( auto iter = Data.MemberBegin(); iter != Data.MemberEnd();
		iter++ )
	{
		if ( iter->name.IsNull() || iter->value.IsNull() )
			continue;
		
		//	Keys cannot be anything but strings
		const GameID nGameID{ std::stoul(iter->name.GetString()) };
		const std::string sTitle{ iter->value.GetString() };

		// TODO: Find a way to do this for NES, the breaks with bounds checking
		m_aGameTitles[sTitle] = nGameID;
	}
}

void Dlg_GameTitle::OnOK(HWND hDlg)
{

	//	Fetch input data:
	TCHAR sSelectedTitleBuffer[512];
	GetText(hKnownGamesCbo, 512, sSelectedTitleBuffer);
	tstring sSelectedTitle = sSelectedTitleBuffer;

	GameID nGameID = 0;
	if ( sSelectedTitle == "<New Title>")
	{
		//	Add a new title!
		GetText(game_title, 512, sSelectedTitleBuffer);
		sSelectedTitle = sSelectedTitleBuffer;
	}
	else
	{
		//	Existing title
		ASSERT(m_aGameTitles.find(std::string(sSelectedTitle)) !=
			m_aGameTitles.end());
		nGameID = m_aGameTitles[std::string(Narrow(sSelectedTitle))];
	}

	PostArgs args
	{
		{ 'u', RAUSER_NAME },
		{ 't', RAUSER_TOKEN },
		{ 'm', m_sMD5 },
		{ 'i', sSelectedTitle },
		{ 'c', std::to_string(g_ConsoleID) }
	};


	Document doc;
	if ( RAWeb::DoBlockingRequest(RequestSubmitNewTitle, args, doc) &&
		doc.HasMember("Success") && doc["Success"].GetBool() )
	{
		const Value& Response = doc["Response"];

		const auto nGameID{
			static_cast<GameID>(Response["GameID"].GetUint())
		};
		const auto sGameTitle = Response["GameTitle"].GetString();

		//	If we're setting the game title here...
		//	 surely we could set the game ID here too?
		g_pCurrentGameData->SetGameTitle(sGameTitle);
		g_pCurrentGameData->SetGameID(nGameID);

		m_nReturnedGameID = nGameID;

		//	Close this dialog
		IRA_Dialog::OnOK(hDlg);
		return;
	}
	else
	{
		if ( !doc.HasParseError() && doc.HasMember("Error") )
		{
			//Error given
			MessageBox(hDlg,
				NativeStr(std::string("Could not add new title: ") +
					std::string(doc["Error"].GetString())).c_str(),
				TEXT("Errors encountered"),
				MB_OK);
		}
		else
		{
			MessageBox(hDlg,
				TEXT("Cannot contact server!"),
				TEXT("Error in connection"),
				MB_OK);
		}

		return;
	}

}

void Dlg_GameTitle::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch ( id )
	{
	case IDOK:
		OnOK(hDlg);
		break;

	case IDCANCEL:
		OnCancel(hDlg);
		break;

	case IDC_RA_GAMETITLE:
		OnGameTitle(hDlg, hwndCtl, codeNotify);
		break;

	case IDC_RA_KNOWNGAMES:
		OnKnownGames(codeNotify);
	}
}

void Dlg_GameTitle::OnKnownGames(const UINT codeNotify)
{
	switch ( codeNotify )
	{
	case CBN_SELCHANGE:
	{
		//	If the user has selected a value, copy this text to the bottom textbox.
		bUpdatingTextboxTitle = true;

		TCHAR sSelectedTitle[512];
		GetText(hKnownGamesCbo, 512, sSelectedTitle);
		SetText(game_title, sSelectedTitle);

		bUpdatingTextboxTitle = false;
	}
	}
}

void Dlg_GameTitle::OnGameTitle(HWND hDlg, HWND hwndCtl, UINT codeNotify)
{
	switch ( codeNotify )
	{
	case EN_CHANGE:
		if ( !bUpdatingTextboxTitle )	//	Note: use barrier to prevent automatic switching off.
		{
			//	If the user has started to enter a value, set the upper combo to 'new entry'
			hwndCtl = GetDlgItem(hDlg, IDC_RA_KNOWNGAMES);
			ComboBox_SetCurSel(hwndCtl, 0);
		}
	}
}
