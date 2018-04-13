
#include "RA_Core.h"
#include "RA_Achievement.h"
#include "RA_AchievementSet.h"
#include "RA_AchievementOverlay.h"
#include "RA_BuildVer.h"
#include "RA_CodeNotes.h"
#include "RA_GameData.h"
#include "RA_httpthread.h"
#include "RA_ImageFactory.h"
#include "RA_Interface.h"
#include "RA_Leaderboard.h"
#include "RA_md5factory.h"
#include "RA_MemManager.h"
#include "RA_PopupWindows.h"
#include "RA_RichPresence.h"
#include "RA_User.h"

#include "RA_Dlg_AchEditor.h"
#include "RA_Dlg_Achievement.h"
#include "RA_Dlg_AchievementsReporter.h"
#include "RA_Dlg_GameLibrary.h"
#include "RA_Dlg_GameTitle.h"
#include "RA_Dlg_Login.h"
#include "RA_Dlg_Memory.h"
#include "RA_Dlg_RichPresence.h"
#include "RA_Dlg_RomChecksum.h"
#include "RA_Dlg_MemBookmark.h"


// Not sure if I did something but the know version isn't appearing
std::string g_sKnownRAVersion{ "0.000" };
std::string g_sHomeDir;
std::string g_sROMDirLocation;
std::string g_sCurrentROMMD5;	//	Internal


#if _DEBUG
constexpr cstring lib_name{ "RA_Integration_d.dll" };
#else
constexpr cstring lib_name{ "RA_Integration.dll" };
#endif // _DEBUG


// I want to see if we can load this immediately
// Let me see if I don't call load library imediatley
// It seems loading this at compile time causes it to request for latest client page
// If loaded at runtime, it requests news
HMODULE g_hThisDLLInst{ nullptr };

HINSTANCE g_hRAKeysDLL = nullptr;

// Where is this initilized?
HWND g_RAMainWnd = nullptr;
EmulatorID g_EmulatorID = EmulatorID::UnknownEmulator;	//	Uniquely identifies the emulator
ConsoleID g_ConsoleID = ConsoleID::UnknownConsoleID;	//	Currently active Console ID
const char* g_sGetLatestClientPage = nullptr;
const char* g_sClientVersion = nullptr;
const char* g_sClientName = nullptr;
const char* g_sClientDownloadURL = nullptr;
const char* g_sClientEXEName = nullptr;
bool g_bRAMTamperedWith = false;
bool g_bHardcoreModeActive = true;
bool g_bLeaderboardsActive = true;
bool g_bLBDisplayNotification = true;
bool g_bLBDisplayCounter = true;
bool g_bLBDisplayScoreboard = true;
unsigned int g_nNumHTTPThreads = 15;

typedef struct WindowPosition {
	int nLeft;
	int nTop;
	int nWidth;
	int nHeight;
	bool bLoaded;

	static const int nUnset = -99999;
} WindowPosition;

using WindowPositionMap = std::map<std::string, WindowPosition>;
WindowPositionMap g_mWindowPositions;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, _UNUSED LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		g_hThisDLLInst = hModule;
	return TRUE;
}

API const char* CCONV _RA_IntegrationVersion() noexcept
{
	return RA_INTEGRATION_VERSION;
}

// Figured out where g_RAMainWnd came from
// TODO: Make RA_Core into a MFC like class and export the member functions like originally
API BOOL CCONV _RA_InitI(HWND hMainHWND, /*enum EmulatorID*/int nEmulatorID, const char* sClientVer) noexcept
{
	//	Ensure all required directories are created:
	if (DirectoryExists(RA_DIR_BASE) == FALSE)
		_mkdir(RA_DIR_BASE);
	if (DirectoryExists(RA_DIR_BADGE) == FALSE)
		_mkdir(RA_DIR_BADGE);
	if (DirectoryExists(RA_DIR_DATA) == FALSE)
		_mkdir(RA_DIR_DATA);
	if (DirectoryExists(RA_DIR_USERPIC) == FALSE)
		_mkdir(RA_DIR_USERPIC);
	if (DirectoryExists(RA_DIR_OVERLAY) == FALSE)	//	It should already, really...
		_mkdir(RA_DIR_OVERLAY);
	if (DirectoryExists(RA_DIR_BOOKMARKS) == FALSE)
		_mkdir(RA_DIR_BOOKMARKS);


	g_EmulatorID = static_cast<EmulatorID>(nEmulatorID);
	g_RAMainWnd = hMainHWND;
	//g_hThisDLLInst

	RA_LOG(__FUNCTION__ " Init called! ID: %d, ClientVer: %s\n", nEmulatorID, sClientVer);

	switch (g_EmulatorID)
	{
	case EmulatorID::UnknownEmulator:
		_FALLTHROUGH;
	case RA_Gens:
		g_ConsoleID = MegaDrive;
		g_sGetLatestClientPage = "LatestRAGensVersion.html";
		g_sClientVersion = sClientVer;
		g_sClientName = "RAGens_REWiND";
		g_sClientDownloadURL = "RAGens.zip";
		g_sClientEXEName = "RAGens.exe";
		break;
	case RA_Project64:
		g_ConsoleID = N64;
		g_sGetLatestClientPage = "LatestRAP64Version.html";
		g_sClientVersion = sClientVer;
		g_sClientName = "RAP64";
		g_sClientDownloadURL = "RAP64.zip";
		g_sClientEXEName = "RAP64.exe";
		break;
	case RA_Snes9x:
		g_ConsoleID = SNES;
		g_sGetLatestClientPage = "LatestRASnesVersion.html";
		g_sClientVersion = sClientVer;
		g_sClientName = "RASnes9X";
		g_sClientDownloadURL = "RASnes9X.zip";
		g_sClientEXEName = "RASnes9X.exe";
		break;
	case RA_VisualboyAdvance:
		g_ConsoleID = GB;
		g_sGetLatestClientPage = "LatestRAVBAVersion.html";
		g_sClientVersion = sClientVer;
		g_sClientName = "RAVisualBoyAdvance";
		g_sClientDownloadURL = "RAVBA.zip";
		g_sClientEXEName = "RAVisualBoyAdvance.exe";
		break;
	case RA_Nester:
	case RA_FCEUX:
		g_ConsoleID = NES;
		g_sGetLatestClientPage = "LatestRANESVersion.html";
		g_sClientVersion = sClientVer;
		g_sClientName = "RANes";
		g_sClientDownloadURL = "RANes.zip";
		g_sClientEXEName = "RANes.exe";
		break;
	case RA_PCE:
		g_ConsoleID = PCEngine;
		g_sGetLatestClientPage = "LatestRAPCEVersion.html";
		g_sClientVersion = sClientVer;
		g_sClientName = "RAPCE";
		g_sClientDownloadURL = "RAPCE.zip";
		g_sClientEXEName = "RAPCE.exe";
		break;
	case RA_Libretro:
		g_ConsoleID = Atari2600;
		g_sGetLatestClientPage = "LatestRALibretroVersion.html";
		g_sClientVersion = sClientVer;
		g_sClientName = "RALibretro";
		g_sClientDownloadURL = "RALibretro.zip";
		g_sClientEXEName = "RALibretro.exe";
		break;
	default:
		break;
	}

	if (g_sClientName != nullptr)
	{
		RA_LOG("(found as: %s)\n", g_sClientName);
	}

	TCHAR buffer[2048];
	GetCurrentDirectory(2048, buffer);
	g_sHomeDir = Narrow(buffer);
	g_sHomeDir.append("\\");

	RA_LOG(__FUNCTION__ " - storing \"%s\" as home dir\n", g_sHomeDir.c_str());

	g_sROMDirLocation[0] = '\0';

	_RA_LoadPreferences();

	RAWeb::RA_InitializeHTTPThreads();

	//////////////////////////////////////////////////////////////////////////
	//	Dialogs:
	g_MemoryDialog.Init();

	//////////////////////////////////////////////////////////////////////////
	//	Initialize All AchievementSets
	g_pCoreAchievements = new AchievementSet(Core);
	g_pUnofficialAchievements = new AchievementSet(Unofficial);
	g_pLocalAchievements = new AchievementSet(Local);
	g_pActiveAchievements = g_pCoreAchievements;

	//////////////////////////////////////////////////////////////////////////
	//	Image rendering: Setup image factory and overlay
	InitializeUserImageFactory(g_hThisDLLInst);
	g_AchievementOverlay.Initialize(g_hThisDLLInst);

	//////////////////////////////////////////////////////////////////////////
	//	Setup min required directories:
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());

	//////////////////////////////////////////////////////////////////////////
	//	Update news:
	PostArgs args;
	args['c'] = std::to_string(6);
	RAWeb::CreateThreadedHTTPRequest(RequestNews, args);

	//////////////////////////////////////////////////////////////////////////
	//	Attempt to fetch latest client version:
	args.clear();
	args['c'] = std::to_string(g_ConsoleID);
	RAWeb::CreateThreadedHTTPRequest(RequestLatestClientPage, args);	//	g_sGetLatestClientPage

	//	TBD:
	//if( RAUsers::LocalUser().Username().length() > 0 )
	//{
	//	args.clear();
	//	args[ 'u' ] = RAUsers::LocalUser().Username();
	//	args[ 't' ] = RAUsers::LocalUser().Token();
	//	RAWeb::CreateThreadedHTTPRequest( RequestScore, args );
	//}

	return TRUE;
}

API int CCONV _RA_Shutdown() noexcept
{
    
	_RA_SavePreferences();

	SAFE_DELETE(g_pCoreAchievements);
	SAFE_DELETE(g_pUnofficialAchievements);
	SAFE_DELETE(g_pLocalAchievements);

	RAWeb::RA_KillHTTPThreads();

    _RA PostNcDestroy(&g_AchievementsDialog);

	if (g_AchievementEditorDialog.GetHWND() != nullptr)
	{
		DestroyWindow(g_AchievementEditorDialog.GetHWND());
		g_AchievementEditorDialog.InstallHWND(nullptr);
	}

	if (g_MemoryDialog.GetHWND() != nullptr)
	{
		DestroyWindow(g_MemoryDialog.GetHWND());
		g_MemoryDialog.InstallHWND(nullptr);
	}

    _RA PostNcDestroy(&g_GameLibrary);
	g_GameLibrary.KillThread();


	if (SetActiveWindow(g_RichPresenceDialog.GetWindow()); GetActiveWindow())
        g_RichPresenceDialog.PostNcDestroy();

	if (g_MemBookmarkDialog.GetHWND() != nullptr)
	{
		DestroyWindow(g_MemBookmarkDialog.GetHWND());
		g_MemBookmarkDialog.InstallHWND(nullptr);
	}

	CoUninitialize();

	return 0;
}



API bool CCONV _RA_ConfirmLoadNewRom(bool bQuittingApp) noexcept
{
	//	Returns true if we can go ahead and load the new rom.
	auto nResult{ IDYES };

	auto sCurrentAction{ bQuittingApp ? "quit now"s : "load a new ROM"s };
	auto sNextAction{ bQuittingApp ? "Are you sure?"s : "Continue load?"s };

	if (g_pCoreAchievements->HasUnsavedChanges())
	{

		auto buffer = tfm::format(
			"You have unsaved changes in the Core Achievements set.\n"
			"If you %s you will lose these changes.\n"
			"%s", sCurrentAction, sNextAction);
		nResult = show_warning(buffer, g_RAMainWnd);
	}
	if (g_pUnofficialAchievements->HasUnsavedChanges())
	{
		auto buffer = tfm::format(
			"You have unsaved changes in the Unofficial Achievements set.\n"
			"If you %s you will lose these changes.\n"
			"%s", sCurrentAction, sNextAction);

		nResult = show_warning(buffer, g_RAMainWnd);
	}
	if (g_pLocalAchievements->HasUnsavedChanges())
	{
		auto buffer = tfm::format(
			"You have unsaved changes in the Local Achievements set.\n"
			"If you %s you will lose these changes.\n"
			"%s", sCurrentAction, sNextAction);

		nResult = show_warning(buffer, g_RAMainWnd);
	}

	return(nResult == IDYES);
}

API void CCONV _RA_SetConsoleID(unsigned int nConsoleID) noexcept
{
	g_ConsoleID = static_cast<ConsoleID>(nConsoleID);
}

API int CCONV _RA_HardcoreModeIsActive() noexcept
{
	return g_bHardcoreModeActive;
}

API int CCONV _RA_HTTPGetRequestExists(_UNUSED const char* sPageName) noexcept
{
	//return RAWeb::HTTPRequestExists( sPageName );	//	Deprecated
	return 0;
}

API int CCONV _RA_OnLoadNewRom(const BYTE* pROM, unsigned int nROMSize) noexcept
{
	static std::string sMD5null = RAGenerateMD5(nullptr, 0);

	g_sCurrentROMMD5 = RAGenerateMD5(pROM, nROMSize);
	RA_LOG("Loading new ROM... MD5 is %s\n", (g_sCurrentROMMD5 == sMD5null) ? "Null" : g_sCurrentROMMD5.c_str());

	ASSERT(g_MemManager.NumMemoryBanks() > 0);

	//	Go ahead and load: RA_ConfirmLoadNewRom has allowed it.
	//	TBD: local DB of MD5 to GameIDs here
	GameID nGameID = 0;
	if (pROM != nullptr)
	{
		//	Fetch the gameID from the DB here:
		PostArgs args;
		args['u'] = RAUsers::LocalUser().Username();
		args['t'] = RAUsers::LocalUser().Token();
		args['m'] = g_sCurrentROMMD5;

		Document doc;
		if (RAWeb::DoBlockingRequest(RequestGameID, args, doc))
		{
			nGameID = static_cast<GameID>(doc["GameID"].GetUint());
			if (nGameID == 0)	//	Unknown
			{
				RA_LOG("Could not recognise game with MD5 %s\n", g_sCurrentROMMD5.c_str());
				std::string buffer;
				buffer.reserve(64_z);

				RA_GetEstimatedGameTitle(buffer.data());
				buffer = buffer.data();
				std::string sEstimatedGameTitle(buffer);
				Dlg_GameTitle my_game_title;
				my_game_title.GetInfoFromModal(g_sCurrentROMMD5, sEstimatedGameTitle, nGameID);
			}
			else
			{
				RA_LOG("Successfully looked up game with ID %d\n", nGameID);
			}
		}
		else
		{
			//	Some other fatal error... panic?
			ASSERT(!"Unknown error from requestgameid.php");
			auto str{ tfm::format("Error from %s!\n", RA_HOST_URL) };
			MessageBox(g_RAMainWnd, NativeStr(str).c_str(), TEXT("Error returned!"), MB_OK);
		}
	}

	//g_PopupWindows.Clear(); //TBD

	g_bRAMTamperedWith = false;
	g_LeaderboardManager.Clear();
	g_PopupWindows.LeaderboardPopups().Reset();

	if (nGameID != 0)
	{
		if (RAUsers::LocalUser().IsLoggedIn())
		{
			//	Delete Core and Unofficial Achievements so they are redownloaded every time:
			g_pCoreAchievements->Clear();
			g_pUnofficialAchievements->Clear();
			g_pLocalAchievements->Clear();

			g_pCoreAchievements->DeletePatchFile(nGameID);
			g_pUnofficialAchievements->DeletePatchFile(nGameID);

			AchievementSet::FetchFromWebBlocking(nGameID);

			g_pCoreAchievements->LoadFromFile(nGameID);
			g_pUnofficialAchievements->LoadFromFile(nGameID);
			g_pLocalAchievements->LoadFromFile(nGameID);

			RAUsers::LocalUser().PostActivity(PlayerStartedPlaying);
		}
	}
	else
	{
		g_pCoreAchievements->Clear();
		g_pUnofficialAchievements->Clear();
		g_pLocalAchievements->Clear();
	}

	if (!g_bHardcoreModeActive && g_bLeaderboardsActive)
	{
		g_PopupWindows.AchievementPopups().AddMessage(
			MessagePopup("Playing in Softcore Mode", "Leaderboard submissions will be canceled.", PopupInfo));
	}
	else if (!g_bHardcoreModeActive)
	{
		g_PopupWindows.AchievementPopups().AddMessage(
			MessagePopup("Playing in Softcore Mode", "", PopupInfo));
	}

	g_AchievementsDialog.OnLoad_NewRom(nGameID);
	g_AchievementEditorDialog.OnLoad_NewRom();
	g_MemoryDialog.OnLoad_NewRom();
	g_AchievementOverlay.OnLoad_NewRom();
	g_MemBookmarkDialog.OnLoad_NewRom();

	return 0;
}

API void CCONV _RA_InstallMemoryBank(int nBankID, void* pReader, void* pWriter, int nBankSize) noexcept
{
	g_MemManager.AddMemoryBank(static_cast<size_t>(nBankID), (_RAMByteReadFn*)pReader, (_RAMByteWriteFn*)pWriter, static_cast<size_t>(nBankSize));
}

API void CCONV _RA_ClearMemoryBanks() noexcept
{
	g_MemManager.ClearMemoryBanks();
}


//void FetchBinaryFromWeb( const char* sFilename )
//{
//	const unsigned int nBufferSize = (3*1024*1024);	//	3mb enough?
//
//	char* buffer = new char[nBufferSize];	
//	if( buffer != null )
//	{
//		char sAddr[1024];
//		sprintf_s( sAddr, 1024, "/files/%s", sFilename );
//		char sOutput[1024];
//		sprintf_s( sOutput, 1024, "%s%s.new", g_sHomeDir, sFilename );
//
//		DWORD nBytesRead = 0;
//		if( RAWeb::DoBlockingHttpGet( sAddr, buffer, nBufferSize, &nBytesRead ) )
//			_WriteBufferToFile( sOutput, buffer, nBytesRead );
//
//		delete[] ( buffer );
//		buffer = null;
//	}
//}

API BOOL CCONV _RA_OfferNewRAUpdate(const char* sNewVer) noexcept
{
	auto buffer{ tfm::format("Update available!\n"
		"A new version of %s is available for download at %s.\n\n"
		"Would you like to update?\n\n"
		"Current version:%s\n"
		"New version:%s\n",
		g_sClientName,
		RA_HOST_URL,
		g_sClientVersion,
		sNewVer)
	};

	//	Update last known version:
	//strcpy_s( g_sKnownRAVersion, 50, sNewVer );

	if (MessageBox(g_RAMainWnd, NativeStr(buffer).c_str(), TEXT("Update available!"), MB_YESNO) == IDYES)
	{
		//SetCurrentDirectory( g_sHomeDir );
		//FetchBinaryFromWeb( g_sClientEXEName );
		//
		//char sBatchUpdater[2048];
		//sprintf_s( sBatchUpdater, 2048,
		//	"@echo off\r\n"
		//	"taskkill /IM %s\r\n"
		//	"del /f %s\r\n"
		//	"rename %s.new %s%s.exe\r\n"
		//	"start %s%s.exe\r\n"
		//	"del \"%%~f0\"\r\n",
		//	g_sClientEXEName,
		//	g_sClientEXEName,
		//	g_sClientEXEName,
		//	g_sClientName,
		//	sNewVer,
		//	g_sClientName,
		//	sNewVer );

		//_WriteBufferToFile( "BatchUpdater.bat", sBatchUpdater, strlen( sBatchUpdater ) );

		//ShellExecute( null,
		//	"open",
		//	"BatchUpdater.bat",
		//	null,
		//	null,
		//	SW_SHOWNORMAL ); 

		ShellExecute(null,
			TEXT("open"),
			TEXT("http://www.retroachievements.org/download.php"),
			null,
			null,
			SW_SHOWNORMAL);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

API int CCONV _RA_HandleHTTPResults() noexcept
{
	WaitForSingleObject(RAWeb::Mutex(), INFINITE);

	RequestObject* pObj = RAWeb::PopNextHttpResult();
	while (pObj != nullptr)
	{
		if (pObj->GetResponse().size() > 0)
		{
			Document doc;
			BOOL bJSONParsedOK = FALSE;

			if (pObj->GetRequestType() == RequestBadge)
			{
				//	Ignore...
			}
			else
			{
				bJSONParsedOK = pObj->ParseResponseToJSON(doc);
			}

			switch (pObj->GetRequestType())
			{
			case RequestType::RequestSubmitTicket:
			case RequestType::RequestSubmitNewTitle:
			case RequestType::RequestSubmitCodeNote:
			case RequestType::RequestSubmitAchievementData:
			case RequestType::RequestRichPresence:
			case RequestType::RequestPostActivity:
			case RequestType::RequestPing:
			case RequestType::RequestPatch:
			case RequestType::RequestHashLibrary:
			case RequestType::RequestGamesList:
			case RequestType::RequestGameID:
			case RequestType::RequestFriendList:
			case RequestType::RequestAllProgress:
			case RequestType::NumRequestTypes:
			case RequestType::StopThread:
				_FALLTHROUGH;
			case RequestLogin:
				RAUsers::LocalUser().HandleSilentLoginResponse(doc);
				break;

			case RequestBadge:
			{
				SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
				const std::string& sBadgeURI = pObj->GetData();
				_WriteBufferToFile(RA_DIR_BADGE + sBadgeURI + ".png", pObj->GetResponse());

				/* This block seems unnecessary. --GD
				for( size_t i = 0; i < g_pActiveAchievements->NumAchievements(); ++i )
				{
					Achievement& ach = g_pActiveAchievements->GetAchievement( i );
					if( ach.BadgeImageURI().compare( 0, 5, sBadgeURI, 0, 5 ) == 0 )
					{
						//	Re-set this badge image
						//	NB. This is already a non-modifying op
						ach.SetBadgeImage( sBadgeURI );
					}
				}*/

				g_AchievementEditorDialog.UpdateSelectedBadgeImage();	//	Is this necessary if it's no longer selected?
			}
			break;

			case RequestBadgeIter:
				g_AchievementEditorDialog.GetBadgeNames().OnNewBadgeNames(doc);
				break;

			case RequestUserPic:
				RAUsers::OnUserPicDownloaded(*pObj);
				break;

			case RequestScore:
			{
				ASSERT(doc["Success"].GetBool());
				if (doc["Success"].GetBool() && doc.HasMember("User") && doc.HasMember("Score"))
				{
					const std::string& sUser = doc["User"].GetString();
					unsigned int nScore = doc["Score"].GetUint();
					RA_LOG("%s's score: %d", sUser.c_str(), nScore);

					if (sUser.compare(RAUsers::LocalUser().Username()) == 0)
					{
						RAUsers::LocalUser().SetScore(nScore);
					}
					else
					{
						//	Find friend? Update this information?
						RAUsers::GetUser(sUser)->SetScore(nScore);
					}
				}
				else
				{
					ASSERT(!"RequestScore bad response!?");
					RA_LOG("RequestScore bad response!?");
				}
			}
			break;

			case RequestLatestClientPage:
			{
				if (doc.HasMember("LatestVersion"))
				{
					
                    // this was done incorrectly so it was getting skipped
					if (std::string sReply{ doc["LatestVersion"].GetString() };
					sReply.substr(0, 2) == "0.")
					{
                        // I'm just curious what's the difference
						// auto temp = sReply.c_str() + 2;
                        // From What I could tell, it removed the "."

                        // It does erase it , we'll just use rvalues then
						auto temp_reply{ sReply };
						temp_reply.erase(1,1);

						std::string known_ver{ g_sKnownRAVersion };

                        if(!g_sKnownRAVersion.empty())
							known_ver.erase(1, 1);

						std::string temp{ g_sClientVersion };
						auto client_ver{ temp.erase(1,1) };

                        // if it actually erases the string we're gonna use temp vars

						auto nValServer{ std::stol(temp_reply) };
						auto nValKnown{std::stol(known_ver)};
						auto nValCurrent{ std::stol(temp) };

						if (nValKnown < nValServer && nValCurrent < nValServer)
						{
							//	Update available:
							_RA_OfferNewRAUpdate(sReply.c_str());

							//	Update the last version I've heard of:
							g_sKnownRAVersion = sReply;
						}
						else
						{
							RA_LOG("Latest Client already up to date: server 0.%d, current 0.%d\n", nValServer, nValCurrent);
						}
					}
				}
				else
				{
					ASSERT(!"RequestLatestClientPage responded, but 'LatestVersion' cannot be found!");
					RA_LOG("RequestLatestClientPage responded, but 'LatestVersion' cannot be found?");
				}
			}
			break;

			case RequestSubmitAwardAchievement:
			{
				//	Response to an achievement being awarded:
				AchievementID nAchID = static_cast<AchievementID>(doc["AchievementID"].GetUint());
				const Achievement* pAch = g_pCoreAchievements->Find(nAchID);
				if (pAch == nullptr)
					pAch = g_pUnofficialAchievements->Find(nAchID);
				if (pAch != nullptr)
				{
					if (!doc.HasMember("Error"))
					{
						g_PopupWindows.AchievementPopups().AddMessage(
							MessagePopup("Achievement Unlocked",
								pAch->Title() + " (" + std::to_string(pAch->Points()) + ")",
								PopupMessageType::PopupAchievementUnlocked,
								pAch->BadgeImage()));
						g_AchievementsDialog.OnGet_Achievement(*pAch);

						RAUsers::LocalUser().SetScore(doc["Score"].GetUint());
					}
					else
					{
						g_PopupWindows.AchievementPopups().AddMessage(
							MessagePopup("Achievement Unlocked (Error)",
								pAch->Title() + " (" + std::to_string(pAch->Points()) + ")",
								PopupMessageType::PopupAchievementError,
								pAch->BadgeImage()));
						g_AchievementsDialog.OnGet_Achievement(*pAch);

						g_PopupWindows.AchievementPopups().AddMessage(
							MessagePopup("Error submitting achievement:",
								doc["Error"].GetString())); //?

			  //MessageBox( HWnd, buffer, "Error!", MB_OK|MB_ICONWARNING );
					}
				}
				else
				{
					ASSERT(!"RequestSubmitAwardAchievement responded, but cannot find achievement ID!");
					RA_LOG("RequestSubmitAwardAchievement responded, but cannot find achievement with ID %d", nAchID);
				}
			}
			break;

			case RequestNews:
				SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
				_WriteBufferToFile(RA_NEWS_FILENAME, doc);
				g_AchievementOverlay.InstallNewsArticlesFromFile();
				break;

			case RequestAchievementInfo:
				g_AchExamine.OnReceiveData(doc);
				break;

			case RequestCodeNotes:
				CodeNotes::OnCodeNotesResponse(doc);
				break;

			case RequestSubmitLeaderboardEntry:
				RA_LeaderboardManager::OnSubmitEntry(doc);
				break;

			case RequestLeaderboardInfo:
				g_LBExamine.OnReceiveData(doc);
				break;

			case RequestUnlocks:
				AchievementSet::OnRequestUnlocks(doc);
				//sprintf_s( sMessage, 512, " You have %d of %d achievements unlocked. ", nNumUnlocked, m_nNumAchievements );
				break;

			}
		}

		SAFE_DELETE(pObj);
		pObj = RAWeb::PopNextHttpResult();
	}

	ReleaseMutex(RAWeb::Mutex());
	return 0;
}

//	Following this function, an app should call AppendMenu to associate this submenu.
API HMENU CCONV _RA_CreatePopupMenu() noexcept
{
	HMENU hRA = CreatePopupMenu();
	HMENU hRA_LB = CreatePopupMenu();
	if (RAUsers::LocalUser().IsLoggedIn())
	{
		AppendMenu(hRA, MF_STRING, IDM_RA_FILES_LOGOUT, TEXT("Log&out"));
		AppendMenu(hRA, MF_SEPARATOR, 0_z, null);
		AppendMenu(hRA, MF_STRING, IDM_RA_OPENUSERPAGE, TEXT("Open my &User Page"));

		UINT nGameFlags = MF_STRING;
		//if( g_pActiveAchievements->GameID() == 0 )	//	Disabled til I can get this right: Snes9x doesn't call this?
		//	nGameFlags |= (MF_GRAYED|MF_DISABLED);


		AppendMenu(hRA, nGameFlags, IDM_RA_OPENGAMEPAGE, TEXT("Open this &Game's Page"));
		AppendMenu(hRA, MF_SEPARATOR, 0_z, null);
		AppendMenu(hRA, static_cast<UINT>(g_bHardcoreModeActive ? MF_CHECKED : MF_UNCHECKED), IDM_RA_HARDCORE_MODE, TEXT("&Hardcore Mode"));
		AppendMenu(hRA, MF_SEPARATOR, 0_z, null);

		AppendMenu(hRA, MF_POPUP, (UINT_PTR)hRA_LB, "Leaderboards");
		AppendMenu(hRA_LB, static_cast<UINT>(g_bLeaderboardsActive ? MF_CHECKED : MF_UNCHECKED ), IDM_RA_TOGGLELEADERBOARDS, TEXT("Enable &Leaderboards"));
		AppendMenu(hRA_LB, MF_SEPARATOR, 0_z, null);
		AppendMenu(hRA_LB, static_cast<UINT>(g_bLBDisplayNotification ? MF_CHECKED : MF_UNCHECKED), IDM_RA_TOGGLE_LB_NOTIFICATIONS, TEXT("Display Challenge Notification"));
		AppendMenu(hRA_LB, static_cast<UINT>(g_bLBDisplayCounter ? MF_CHECKED : MF_UNCHECKED), IDM_RA_TOGGLE_LB_COUNTER, TEXT("Display Time/Score Counter"));
		AppendMenu(hRA_LB, static_cast<UINT>(g_bLBDisplayScoreboard ? MF_CHECKED : MF_UNCHECKED), IDM_RA_TOGGLE_LB_SCOREBOARD, TEXT("Display Rank Scoreboard"));

		AppendMenu(hRA, MF_SEPARATOR, 0_z, null);
		AppendMenu(hRA, MF_STRING, IDM_RA_FILES_ACHIEVEMENTS, TEXT("Achievement &Sets"));
		AppendMenu(hRA, MF_STRING, IDM_RA_FILES_ACHIEVEMENTEDITOR, TEXT("Achievement &Editor"));
		AppendMenu(hRA, MF_STRING, IDM_RA_FILES_MEMORYFINDER, TEXT("&Memory Inspector"));
		AppendMenu(hRA, MF_STRING, IDM_RA_PARSERICHPRESENCE, TEXT("Rich &Presence Monitor"));
		AppendMenu(hRA, MF_SEPARATOR, 0_z, null);
		AppendMenu(hRA, MF_STRING, IDM_RA_REPORTBROKENACHIEVEMENTS, TEXT("&Report Broken Achievements"));
		AppendMenu(hRA, MF_STRING, IDM_RA_GETROMCHECKSUM, TEXT("Get ROM &Checksum"));
		AppendMenu(hRA, MF_STRING, IDM_RA_SCANFORGAMES, TEXT("Scan &for games"));
	}
	else
	{
		AppendMenu(hRA, MF_STRING, IDM_RA_FILES_LOGIN, TEXT("&Login to RA"));
	}

	AppendMenu(hRA, MF_SEPARATOR, 0_z, null);
	AppendMenu(hRA, MF_STRING, IDM_RA_FILES_CHECKFORUPDATE, TEXT("&Check for Emulator Update"));

	return hRA;
}

API void CCONV _RA_UpdateAppTitle(const char* sMessage) noexcept
{
	std::stringstream sstr;
	sstr << std::string(g_sClientName) << " - " << std::string(g_sClientVersion);

	if (sMessage != nullptr)
		sstr << " - " << sMessage;

	if (RAUsers::LocalUser().IsLoggedIn())
		sstr << " - " << RAUsers::LocalUser().Username();

	if (_stricmp(RA_HOST_URL, "localhost") == 0)
		sstr << " *AT LOCALHOST*";

    // What's after this, can't follow the execution...
	SetWindowText(g_RAMainWnd, NativeStr(sstr.str()).c_str());
}

//	##BLOCKING##
API void CCONV _RA_CheckForUpdate() noexcept
{
	PostArgs args;
	args['c'] = std::to_string(g_ConsoleID);

	DataStream Response;
	if (RAWeb::DoBlockingRequest(RequestLatestClientPage, args, Response))
	{
		std::string sReply = DataStreamAsString(Response);
		if (sReply.length() > 2 && sReply.at(0) == '0' && sReply.at(1) == '.')
		{
			//	Ignore g_sKnownRAVersion: check against g_sRAVersion
			unsigned long nLocalVersion = std::strtoul(g_sClientVersion + 2, nullptr, 10);
			unsigned long nServerVersion = std::strtoul(sReply.c_str() + 2, nullptr, 10);

			if (nLocalVersion < nServerVersion)
			{
				_RA_OfferNewRAUpdate(sReply.c_str());
			}
			else
			{
				//	Up to date
				auto buffer{
					tfm::format("You have the latest version of %s: 0.%02d",
					g_sClientEXEName, nServerVersion)
				};
				MessageBox(g_RAMainWnd, NativeStr(buffer).c_str(),
					TEXT("Up to date"), MB_OK);
			}
		}
		else
		{
			auto msg{ tfm::format("Error in download from %s...\n"
				"Please check your connection settings or RA forums!", RA_HOST_URL) };
			//	Error in download
			show_error(msg, g_RAMainWnd);
		}
	}
	else
	{
		auto msg{
			tfm::format("Could not connect to %s..\n"
			"Please check your connection settings or RA forums!", RA_HOST_URL)
		};
		show_error(msg, g_RAMainWnd);
	}
}

API void CCONV _RA_LoadPreferences() noexcept
{
	RA_LOG(__FUNCTION__ " - loading preferences...\n");

	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());

	std::ifstream ifile{ prefs_filename() };


	//	Test for first-time use:
	//RA_LOG( __FUNCTION__ " - no preferences found: showing first-time message!\n" );
	//
	//char sWelcomeMessage[4096];

	//sprintf_s( sWelcomeMessage, 4096, 
	//	"Welcome! It looks like this is your first time using RetroAchievements.\n\n"
	//	"Quick Start: Press ESCAPE or 'Back' on your Xbox 360 controller to view the achievement overlay.\n\n" );

	//switch( g_EmulatorID )
	//{
	//case RA_Gens:
	//	strcat_s( sWelcomeMessage, 4096,
	//		"Default Keyboard Controls: Use cursor keys, A-S-D are A, B, C, and Return for Start.\n\n" );
	//	break;
	//case RA_VisualboyAdvance:
	//	strcat_s( sWelcomeMessage, 4096,
	//		"Default Keyboard Controls: Use cursor keys, Z-X are A and B, A-S are L and R, use Return for Start and Backspace for Select.\n\n" );
	//	break;
	//case RA_Snes9x:
	//	strcat_s( sWelcomeMessage, 4096,
	//		"Default Keyboard Controls: Use cursor keys, D-C-S-X are A, B, X, Y, Z-V are L and R, use Return for Start and Space for Select.\n\n" );
	//	break;
	//case RA_FCEUX:
	//	strcat_s( sWelcomeMessage, 4096,
	//		"Default Keyboard Controls: Use cursor keys, D-F are B and A, use Return for Start and S for Select.\n\n" );
	//	break;
	//case RA_PCE:
	//	strcat_s( sWelcomeMessage, 4096,
	//		"Default Keyboard Controls: Use cursor keys, A-S-D for A, B, C, and Return for Start\n\n" );
	//	break;
	//}

	//strcat_s( sWelcomeMessage, 4096, "These defaults can be changed under [Option]->[Joypads].\n\n"
	//	"If you have any questions, comments or feedback, please visit forum.RetroAchievements.org for more information.\n\n" );

	//MessageBox( g_RAMainWnd, 
	//	sWelcomeMessage,
	//	"Welcome to RetroAchievements!", MB_OK );

	//	TBD: setup some decent default variables:
	//_RA_SavePreferences(); this kept on erasing the file


	Document doc;
	// It doesn't like the rvalue
	IStreamWrapper isw{ ifile };
	doc.ParseStream(isw);

	if (doc.HasParseError())
	{
		//MessageBox( nullptr, std::to_string( doc.GetParseError() ).c_str(), "ERROR!", MB_OK );
		_RA_SavePreferences();
	}
	else
	{
		if (doc.HasMember("Username"))
			RAUsers::LocalUser().SetUsername(doc["Username"].GetString());
		if (doc.HasMember("Token"))
			RAUsers::LocalUser().SetToken(doc["Token"].GetString());
		if (doc.HasMember("Hardcore Active"))
			g_bHardcoreModeActive = doc["Hardcore Active"].GetBool();

		if (doc.HasMember("Leaderboards Active"))
			g_bLeaderboardsActive = doc["Leaderboards Active"].GetBool();
		if (doc.HasMember("Leaderboard Notification Display"))
			g_bLBDisplayNotification = doc["Leaderboard Notification Display"].GetBool();
		if (doc.HasMember("Leaderboard Counter Display"))
			g_bLBDisplayCounter = doc["Leaderboard Counter Display"].GetBool();
		if (doc.HasMember("Leaderboard Scoreboard Display"))
			g_bLBDisplayScoreboard = doc["Leaderboard Scoreboard Display"].GetBool();

		if (doc.HasMember("Num Background Threads"))
			g_nNumHTTPThreads = doc["Num Background Threads"].GetUint();
		if (doc.HasMember("ROM Directory"))
			g_sROMDirLocation = doc["ROM Directory"].GetString();

		if (doc.HasMember("Window Positions")) {
			const Value& positions = doc["Window Positions"];
			if (positions.IsObject()) {
				for (Value::ConstMemberIterator iter = positions.MemberBegin(); iter != positions.MemberEnd(); ++iter) {
					WindowPosition& pos = g_mWindowPositions[iter->name.GetString()];
					pos.nLeft = pos.nTop = pos.nWidth = pos.nHeight = WindowPosition::nUnset;
					pos.bLoaded = false;

					if (iter->value.HasMember("X"))
						pos.nLeft = iter->value["X"].GetInt();
					if (iter->value.HasMember("Y"))
						pos.nTop = iter->value["Y"].GetInt();
					if (iter->value.HasMember("Width"))
						pos.nWidth = iter->value["Width"].GetInt();
					if (iter->value.HasMember("Height"))
						pos.nHeight = iter->value["Height"].GetInt();
				}
			}
		}
	}


	//TBD:
	//g_GameLibrary.LoadAll();
}

API void CCONV _RA_SavePreferences()  noexcept
{
	RA_LOG(__FUNCTION__ " - saving preferences...\n");

	if (g_sClientName == nullptr)
	{
		RA_LOG(__FUNCTION__ " - aborting save, we don't even know who we are...\n");
		return;
	}

	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	std::ofstream ofile{ prefs_filename() };

	OStreamWrapper osw{ ofile };
	Writer<OStreamWrapper> writer{ osw };

	Document doc;
	doc.SetObject();

	auto& a = doc.GetAllocator();
	doc.AddMember("Username", StringRef(RAUsers::LocalUser().Username().c_str()), a);
	doc.AddMember("Token", StringRef(RAUsers::LocalUser().Token().c_str()), a);
	doc.AddMember("Hardcore Active", g_bHardcoreModeActive, a);
	doc.AddMember("Leaderboards Active", g_bLeaderboardsActive, a);
	doc.AddMember("Leaderboard Notification Display", g_bLBDisplayNotification, a);
	doc.AddMember("Leaderboard Counter Display", g_bLBDisplayCounter, a);
	doc.AddMember("Leaderboard Scoreboard Display", g_bLBDisplayScoreboard, a);
	doc.AddMember("Num Background Threads", g_nNumHTTPThreads, a);
	doc.AddMember("ROM Directory", StringRef(g_sROMDirLocation.c_str()), a);

	Value positions(kObjectType);
	for (auto iter = g_mWindowPositions.begin(); iter != g_mWindowPositions.end(); ++iter) {
		Value rect(kObjectType);
		if (iter->second.nLeft != WindowPosition::nUnset)
			rect.AddMember("X", iter->second.nLeft, a);
		if (iter->second.nTop != WindowPosition::nUnset)
			rect.AddMember("Y", iter->second.nTop, a);
		if (iter->second.nWidth != WindowPosition::nUnset)
			rect.AddMember("Width", iter->second.nWidth, a);
		if (iter->second.nHeight != WindowPosition::nUnset)
			rect.AddMember("Height", iter->second.nHeight, a);

		if (rect.MemberCount() > 0)
			positions.AddMember(StringRef(iter->first.c_str()), rect, a);
	}

	if (positions.MemberCount() > 0)
		doc.AddMember("Window Positions", positions.Move(), a);

	doc.Accept(writer);	//	Save


//TBD:
//g_GameLibrary.SaveAll();
}

void _FetchGameHashLibraryFromWeb() noexcept
{
	PostArgs args;
	args['c'] = std::to_string(g_ConsoleID);
	args['u'] = RAUsers::LocalUser().Username();
	args['t'] = RAUsers::LocalUser().Token();
	DataStream Response;
	if (RAWeb::DoBlockingRequest(RequestHashLibrary, args, Response))
		_WriteBufferToFile(RA_GAME_HASH_FILENAME, Response);
}

void _FetchGameTitlesFromWeb() noexcept
{
	PostArgs args;
	args['c'] = std::to_string(g_ConsoleID);
	args['u'] = RAUsers::LocalUser().Username();
	args['t'] = RAUsers::LocalUser().Token();
	DataStream Response;
	if (RAWeb::DoBlockingRequest(RequestGamesList, args, Response))
		_WriteBufferToFile(RA_GAME_LIST_FILENAME, Response);
}

void _FetchMyProgressFromWeb() noexcept
{
	PostArgs args;
	args['c'] = std::to_string(g_ConsoleID);
	args['u'] = RAUsers::LocalUser().Username();
	args['t'] = RAUsers::LocalUser().Token();
	DataStream Response;
	if (RAWeb::DoBlockingRequest(RequestAllProgress, args, Response))
		_WriteBufferToFile(RA_MY_PROGRESS_FILENAME, Response);
}

void RestoreWindowPosition(HWND hDlg, const char* sDlgKey, bool bToRight, bool bToBottom)
{
	WindowPosition new_pos;
	WindowPosition* pos;
	WindowPositionMap::iterator iter = g_mWindowPositions.find(std::string(sDlgKey));
	if (iter != g_mWindowPositions.end()) {
		pos = &iter->second;
	}
	else {
		pos = &new_pos;
		pos->nLeft = pos->nTop = pos->nWidth = pos->nHeight = WindowPosition::nUnset;
	}

	RECT rc;
	GetWindowRect(hDlg, &rc);
	const int nDlgWidth = rc.right - rc.left;
	if (pos->nWidth != WindowPosition::nUnset && pos->nWidth < nDlgWidth)
		pos->nWidth = WindowPosition::nUnset;
	const int nDlgHeight = rc.bottom - rc.top;
	if (pos->nHeight != WindowPosition::nUnset && pos->nHeight < nDlgHeight)
		pos->nHeight = WindowPosition::nUnset;

	RECT rcMainWindow;
	GetWindowRect(g_RAMainWnd, &rcMainWindow);

	rc.left = (pos->nLeft != WindowPosition::nUnset) ? (rcMainWindow.left + pos->nLeft) : bToRight ? rcMainWindow.right : rcMainWindow.left;
	rc.right = (pos->nWidth != WindowPosition::nUnset) ? (rc.left + pos->nWidth) : (rc.left + nDlgWidth);
	rc.top = (pos->nTop != WindowPosition::nUnset) ? (rcMainWindow.top + pos->nTop) : bToBottom ? rcMainWindow.bottom : rcMainWindow.top;
	rc.bottom = (pos->nHeight != WindowPosition::nUnset) ? (rc.top + pos->nHeight) : (rc.top + nDlgHeight);

	RECT rcWorkArea;
	if (SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0))
	{
		LONG offset = rc.right - rcWorkArea.right;
		if (offset > 0) {
			rc.left -= offset;
			rc.right -= offset;
		}
		offset = rcWorkArea.left - rc.left;
		if (offset > 0) {
			rc.left += offset;
			rc.right += offset;
		}

		offset = rc.bottom - rcWorkArea.bottom;
		if (offset > 0) {
			rc.top -= offset;
			rc.bottom -= offset;
		}
		offset = rcWorkArea.top - rc.top;
		if (offset > 0) {
			rc.top += offset;
			rc.bottom += offset;
		}
	}

	SetWindowPos(hDlg, null, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0);

	pos->bLoaded = true;

	if (pos == &new_pos)
		g_mWindowPositions.insert(std::make_pair(std::string(sDlgKey), new_pos));
}

void RememberWindowPosition(HWND hDlg, const char* sDlgKey)
{
	WindowPositionMap::iterator iter = g_mWindowPositions.find(std::string(sDlgKey));
	if (iter == g_mWindowPositions.end())
		return;

	if (!iter->second.bLoaded)
		return;

	RECT rcMainWindow;
	GetWindowRect(g_RAMainWnd, &rcMainWindow);

	RECT rc;
	GetWindowRect(hDlg, &rc);

	iter->second.nLeft = rc.left - rcMainWindow.left;
	iter->second.nTop = rc.top - rcMainWindow.top;
}

void RememberWindowSize(HWND hDlg, const char* sDlgKey)
{
	WindowPositionMap::iterator iter = g_mWindowPositions.find(std::string(sDlgKey));
	if (iter == g_mWindowPositions.end())
		return;

	if (!iter->second.bLoaded)
		return;

	RECT rc;
	GetWindowRect(hDlg, &rc);

	iter->second.nWidth = rc.right - rc.left;
	iter->second.nHeight = rc.bottom - rc.top;
}

// N.B.: For this work on modeless dialogs, they MUST have move semantics
// Trust me I tried - SyrianBallaS
// This function looks a lot ok OnCommand (WM_COMMAND)
// The ID is casted from WPARAM btw...
API void CCONV _RA_InvokeDialog(LPARAM nID)  noexcept
{
	switch (nID)
	{
	case IDM_RA_FILES_ACHIEVEMENTS:
	{
		// Is there a a hidden goto statement here? It keeps on going to rich precense
		// Now to test this, it SHOULD work

		// this uses move semantics, using the RAII paradigm the global can't
		// be initilized at compile time but at runtime, so it needs a new
		// temporary object. For the modals it doesn't matter since DoModal
		// doesn't return a handle. In C Modals are on the stack frame but we
		// can do more stuff in c++
		if (!g_AchievementsDialog.HasData())
		{
			// using a straight up rvalue didn't work
			g_AchievementsDialog.SetWindow();
		} // end if
        // Ok it seems to work
		if (g_AchievementsDialog.HasData()) // had this because closed windows couldn't be reopened
			g_AchievementsDialog.Show();
	}
	break;

	case IDM_RA_FILES_ACHIEVEMENTEDITOR:
		if (g_AchievementEditorDialog.GetHWND() == null)
#pragma warning(suppress : 5039) // CreateDialog throws
			g_AchievementEditorDialog.InstallHWND(CreateDialog(g_hThisDLLInst, MAKEINTRESOURCE(IDD_RA_ACHIEVEMENTEDITOR), g_RAMainWnd, g_AchievementEditorDialog.s_AchievementEditorProc));
		if (g_AchievementEditorDialog.GetHWND() != null)
			ShowWindow(g_AchievementEditorDialog.GetHWND(), SW_SHOW);
		break;

	case IDM_RA_FILES_MEMORYFINDER:
		if (g_MemoryDialog.GetHWND() == null)
#pragma warning(suppress : 5039)
			g_MemoryDialog.InstallHWND(CreateDialog(g_hThisDLLInst, MAKEINTRESOURCE(IDD_RA_MEMORY), g_RAMainWnd, g_MemoryDialog.s_MemoryProc));
		if (g_MemoryDialog.GetHWND() != null)
			ShowWindow(g_MemoryDialog.GetHWND(), SW_SHOW);
		break;

	case IDM_RA_FILES_LOGIN:
	{
		_RA Dlg_Login dlg;
		dlg.DoModal();
		_RA_SavePreferences();
	}
	break;

	case IDM_RA_FILES_LOGOUT:
		RAUsers::LocalUser().Clear();
		g_PopupWindows.Clear();
		_RA_SavePreferences();
		_RA_UpdateAppTitle();

		MessageBox(g_RAMainWnd, TEXT("You are now logged out."), TEXT("Info"), MB_OK);	//	##BLOCKING##
		_RA_RebuildMenu();
		break;

	case IDM_RA_FILES_CHECKFORUPDATE:

		_RA_CheckForUpdate();
		break;

	case IDM_RA_HARDCORE_MODE:
	{
		g_bHardcoreModeActive = !g_bHardcoreModeActive;
		_RA_ResetEmulation();

		g_PopupWindows.Clear();

		GameID nGameID = g_pCurrentGameData->GetGameID();
		if (nGameID != 0)
		{
			//	Delete Core and Unofficial Achievements so it is redownloaded every time:
			g_pCoreAchievements->DeletePatchFile(nGameID);
			g_pUnofficialAchievements->DeletePatchFile(nGameID);

			g_pCoreAchievements->Clear();
			g_pUnofficialAchievements->Clear();
			g_pLocalAchievements->Clear();

			//	Fetch remotely then load again from file
			AchievementSet::FetchFromWebBlocking(nGameID);

			g_pCoreAchievements->LoadFromFile(nGameID);
			g_pUnofficialAchievements->LoadFromFile(nGameID);
			g_pLocalAchievements->LoadFromFile(nGameID);
		}

		_RA_RebuildMenu();
	}
	break;

	case IDM_RA_REPORTBROKENACHIEVEMENTS:
	{
		if (g_pCurrentGameData->GetGameID() == 0)
		{
			no_rom_loaded();
			break;
		}
		Dlg_AchievementsReporter dlg;
		dlg.DoModal();
	}
	break;

	case IDM_RA_GETROMCHECKSUM:
	{
		if (g_pCurrentGameData->GetGameID() == 0)
		{
			no_rom_loaded();
			break;
		}
		Dlg_RomChecksum dlg;
		dlg.DoModal();


	}
	break;
	//if( g_pActiveAchievements->NumAchievements() == 0 )
	//	MessageBox( nullptr, "No ROM loaded!", "Error", MB_OK );
	//else
	//	MessageBox( nullptr, ( std::string( "Current ROM MD5: " ) + g_sCurrentROMMD5 ).c_str(), "Get ROM Checksum", MB_OK );
	//break;

	case IDM_RA_OPENUSERPAGE:
		if (RAUsers::LocalUser().IsLoggedIn())
		{
			std::string sTarget{
				tfm::format("http://%s/User/%s", RA_HOST_URL, _RA username())
			};
			ShellExecute(null,
				TEXT("open"),
				NativeStr(sTarget).c_str(),
				null,
				null,
				SW_SHOWNORMAL);
		}
		break;

	case IDM_RA_OPENGAMEPAGE:
		if (g_pCurrentGameData->GetGameID() != 0)
		{
			std::string sTarget{
				tfm::format("http://%s/Game/%d", RA_HOST_URL,
				g_pCurrentGameData->GetGameID())
			};
			ShellExecute(null,
				TEXT("open"),
				NativeStr(sTarget).c_str(),
				null,
				null,
				SW_SHOWNORMAL);
		}
		else
			no_rom_loaded();
		break;

	case IDM_RA_SCANFORGAMES:

		if (g_sROMDirLocation.length() == 0)
		{
			g_sROMDirLocation = GetFolderFromDialog();
		}

		if (g_sROMDirLocation.length() > 0)
		{
			if (g_GameLibrary.GetWindow() == null)
			{
				_FetchGameHashLibraryFromWeb();		//	##BLOCKING##
				_FetchGameTitlesFromWeb();			//	##BLOCKING##
				_FetchMyProgressFromWeb();			//	##BLOCKING##

				g_GameLibrary.SetWindow();
			}
            
			if (g_GameLibrary.GetWindow() != null)
            {
                g_GameLibrary.Show();
            }
		}
		break;

	case IDM_RA_PARSERICHPRESENCE:
		if (g_pCurrentGameData->GetGameID() != 0)
		{
			auto sRichPresenceFile{ tfm::format("%s%d-Rich.txt", RA_DIR_DATA,
				g_pCurrentGameData->GetGameID()) };

			//	Then install it
			g_RichPresenceInterpretter.ParseRichPresenceFile(sRichPresenceFile);

			// this shit is killing me, ireally don't know how it makes a difference but it does
			if (!g_RichPresenceDialog.HasData() || (g_RichPresenceDialog.GetWindow() == null))
				g_RichPresenceDialog.SetWindow();
			if (g_RichPresenceDialog.HasData())
			{
				g_RichPresenceDialog.Show();
                g_RichPresenceDialog.StartMonitoring();
			}
			
		}
		else
			no_rom_loaded();
		break;

	case IDM_RA_TOGGLELEADERBOARDS:
	{
		g_bLeaderboardsActive = !g_bLeaderboardsActive;

		std::string msg;
		msg += "Leaderboards are now ";
		msg += (g_bLeaderboardsActive ? "enabled." : "disabled.");
		msg += "\nNB. You may need to load ROM again to re-enable leaderboards.";

		show_success(msg);

		if (!g_bLeaderboardsActive)
			g_PopupWindows.LeaderboardPopups().Reset();

		_RA_RebuildMenu();
	}
	break;
	case IDM_RA_TOGGLE_LB_NOTIFICATIONS:
		g_bLBDisplayNotification = !g_bLBDisplayNotification;
		_RA_RebuildMenu();
		break;
	case IDM_RA_TOGGLE_LB_COUNTER:
		g_bLBDisplayCounter = !g_bLBDisplayCounter;
		_RA_RebuildMenu();
		break;
	case IDM_RA_TOGGLE_LB_SCOREBOARD:
		g_bLBDisplayScoreboard = !g_bLBDisplayScoreboard;
		_RA_RebuildMenu();
		break;

	default:
		//	Unknown!
		break;
	}
}

API void CCONV _RA_SetPaused(bool bIsPaused) noexcept
{
	//	TBD: store this state?? (Rendering?)
	if (bIsPaused)
		g_AchievementOverlay.Activate();
	else
		g_AchievementOverlay.Deactivate();
}

API const char* CCONV _RA_Username() noexcept
{
	return RAUsers::LocalUser().Username().c_str();
}

API void CCONV _RA_AttemptLogin(bool bBlocking) noexcept
{
	RAUsers::LocalUser().AttemptLogin(bBlocking);
}

API void CCONV _RA_OnSaveState(const char* sFilename) noexcept
{
	//	Save State is being allowed by app (user was warned!)
	if (RAUsers::LocalUser().IsLoggedIn())
	{
		if (g_bHardcoreModeActive)
		{
			g_bHardcoreModeActive = false;
			RA_RebuildMenu();
			//RA_ResetEmulation();
		}

		if (!g_bRAMTamperedWith)
		{
			g_pCoreAchievements->SaveProgress(sFilename);
		}
	}
}

API void CCONV _RA_OnLoadState(const char* sFilename) noexcept
{
	//	Save State is being allowed by app (user was warned!)
	if (RAUsers::LocalUser().IsLoggedIn())
	{
		if (g_bHardcoreModeActive)
		{
			auto msg = "Savestates are not allowed during Hardcore Mode!"s;
			show_warning(msg);
			g_bHardcoreModeActive = false;
			RA_RebuildMenu();
			RA_ResetEmulation();
		}

		g_pCoreAchievements->LoadProgress(sFilename);
		g_LeaderboardManager.Reset();
		g_PopupWindows.LeaderboardPopups().Reset();
		g_MemoryDialog.Invalidate();
	}
}

API void CCONV _RA_DoAchievementsFrame() noexcept
{
	if (RAUsers::LocalUser().IsLoggedIn())
	{
		g_pActiveAchievements->Test();
		g_LeaderboardManager.Test();
		g_MemoryDialog.Invalidate();
	}
}
// if this was a class you wouldn't need __declspec or the calling convention in both files but they it's an expr
// I can't seem to get rid of this
#pragma warning (disable : 5039) // might throw
API void CCONV
_RA_InstallSharedFunctions(bool(CCONV* fpIsActive)(),
	void(CCONV* fpCauseUnpause)(void), void(CCONV* fpRebuildMenu)(),
	void(CCONV* fpEstimateTitle)(char*), void(CCONV* fpResetEmulation)(),
	void(CCONV* fpLoadROM)(const char*)) noexcept
{
	void(*fpCausePause)() = null;

	// The throwing function
	_RA_InstallSharedFunctionsExt(fpIsActive, fpCauseUnpause, fpCausePause,
		fpRebuildMenu, fpEstimateTitle, fpResetEmulation, fpLoadROM);
}

API void CCONV _RA_InstallSharedFunctionsExt(bool(CCONV* fpIsActive)(void),
	void(CCONV* fpCauseUnpause)(void), void(CCONV* fpCausePause)(void),
	void(CCONV* fpRebuildMenu)(void), void(CCONV* fpEstimateTitle)(char*),
	void(CCONV* fpResetEmulation)(void), void(CCONV* fpLoadROM)(const char*)) noexcept
{
	//	NB. Must be called from within DLL
	_RA_GameIsActive = fpIsActive;
	_RA_CauseUnpause = fpCauseUnpause;
	_RA_CausePause = fpCausePause;
	_RA_RebuildMenu = fpRebuildMenu;
	_RA_GetEstimatedGameTitle = fpEstimateTitle;
	_RA_ResetEmulation = fpResetEmulation;
	_RA_LoadROM = fpLoadROM;
}

API bool _RA_UserLoggedIn() noexcept
{
	return(RAUsers::LocalUser().IsLoggedIn() == TRUE);
}


//////////////////////////////////////////////////////////////////////////
// We'll leave this alone since std::getline from istream does the same exact thing
BOOL _ReadTil(const char nChar, char buffer[], unsigned int nSize, DWORD* pCharsReadOut, FILE* pFile)
{
	char pNextChar = '\0';
	memset(buffer, '\0', nSize);

	//	Read title:
	(*pCharsReadOut) = 0;
	do
	{
		if (fread(&pNextChar, sizeof(char), 1, pFile) == 0)
			break;

		buffer[(*pCharsReadOut)++] = pNextChar;
	} while (pNextChar != nChar && (*pCharsReadOut) < nSize && !feof(pFile));

	//return ( !feof( pFile ) );
	return ((*pCharsReadOut) > 0);
}

// We'll leave it alone, same reason as _ReadTil
char* _ReadStringTil(char nChar, char*& pOffsetInOut, BOOL bTerminate)
{
	char* pStartString = pOffsetInOut;

	while ((*pOffsetInOut) != '\0' && (*pOffsetInOut) != nChar)
		pOffsetInOut++;

	if (bTerminate)
		(*pOffsetInOut) = '\0';

	pOffsetInOut++;

	return (pStartString);
}

void _WriteBufferToFile(const std::string& sFileName, const Document& doc)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	std::ofstream ofile{ sFileName };
	OStreamWrapper osw{ ofile };

	Writer<OStreamWrapper> writer{ osw };
	doc.Accept(writer);
}


void _WriteBufferToFile(const std::string& sFileName, const DataStream& raw)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	Data_ofstream ofile{ sFileName };
	ofile << raw;
}

void _WriteBufferToFile(const std::string& sFileName, const std::string& sData)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	std::ofstream ofile{ sFileName };
	ofile << sData;
}

void _WriteBufferToFile(const char* sFile, const BYTE* sBuffer, int nBytes)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	Data_ofstream ofile{ sFile, std::ios::binary };
	ofile.write(sBuffer, nBytes);
}

char* _MallocAndBulkReadFileToBuffer(const char* sFilename, std::streamsize nFileSizeOut)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	auto filename{ std::string{ sFilename } };
	std::ifstream ifile{ filename, std::ios::binary };

	//	Calculate filesize
	auto fsize{ static_cast<std::streamsize>(filesize(filename)) };
	nFileSizeOut = static_cast<long>(fsize);


	if (nFileSizeOut <= 0)
	{
		//	No good content in this file.
		ifile.close();
		return null;
	}

	//	malloc() must be managed!
	//	NB. By adding +1, we allow for a single \0 character :)
	std::string rawfile;
	auto cap{ static_cast<size_t>(nFileSizeOut) + 1_z };
	rawfile.reserve(cap);

	ifile.read(rawfile.data(), fsize);
	return rawfile.data();
}

std::string _TimeStampToString(time_t nTime)
{
	return std::to_string(nTime);
}

bool _FileExists(const std::string& sFileName)
{
	if (std::ifstream ifile{ sFileName, std::ios::binary })
	{
		return true;
	}
	return false;
}

std::string GetFolderFromDialog()
{
	std::string sRetVal;
	// This piece a shit changed too!?!?!?
    CComPtr<IFileOpenDialog> pDlg{ nullptr };
	
	if (auto hr = CoCreateInstance(CLSID_FileOpenDialog, null, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pDlg)); SUCCEEDED(hr))
	{
		pDlg->SetOptions(FOS_PICKFOLDERS);
		
		if (hr = pDlg->Show(nullptr); SUCCEEDED(hr))
		{
            CComPtr<IShellItem> pItem{ nullptr };
			
			if (hr = pDlg->GetResult(&pItem); SUCCEEDED(hr))
			{
                LPWSTR pStr = nullptr;
				
				if (hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pStr); SUCCEEDED(hr))
				{
                    std::wstring owner{ pStr };
					sRetVal = Narrow(owner);
                    
				}
                pStr = null;
				pItem.Release();
			}
		}
		pDlg.Release();
	}
	return sRetVal;
}

BOOL RemoveFileIfExists(const std::string& sFilePath)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());

	if (_access(sFilePath.c_str(), 06) != -1)	//	06= Read/write permission
	{
		if (remove(sFilePath.c_str()) == -1)
		{
			//	Remove issues?
			ASSERT(!"Could not remove patch file!?");
			return FALSE;
		}
		else
		{
			//	Successfully deleted
			return TRUE;
		}
	}
	else
	{
		//	Doesn't exist (or no permissions?)
		return TRUE;
	}
}

BOOL CanCausePause()
{
	return (_RA_CausePause != nullptr);
}

namespace ra {
int CALLBACK no_rom_loaded() noexcept {
	return MessageBox(GetActiveWindow(), TEXT("No ROM loaded!"), TEXT("Error!"),
		MB_OK | MB_ICONERROR);
} // end function no_rom_loaded


std::string CCONV prefs_filename() noexcept {
	return tfm::format("%s%s.json", RA_PREFERENCES_FILENAME_PREFIX, g_sClientName);
} // end function prefs_filename


// would it be better if was a pointer?
void CALLBACK PostNcDestroy(IRA_Dialog* dialog) noexcept
{
    if (dialog->GetWindow())
    {
        dialog->PostNcDestroy();
        dialog = null; // Well I never used new
    }
}




} // namespace ra

