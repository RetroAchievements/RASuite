#include "RA_Dlg_GameLibrary.h"


#include <stack>


//#include "RA_Defs.h" - no point in including twice
#include "RA_Core.h"
#include "RA_Resource.h"
#include "RA_User.h"
#include "RA_Achievement.h"
#include "RA_httpthread.h"
#include "RA_md5factory.h"

#define KEYDOWN(vkCode) ((GetAsyncKeyState(vkCode) & 0x8000) ? true : false)

_RA Dlg_GameLibrary g_GameLibrary;

namespace ra
{
constexpr auto num_col{ 4 };

using ColumnTitles = std::array<cstring, num_col>;
using ColumnWidths = std::array<int, num_col>; // int instead because of listviews

// this is fucking up...
constexpr ColumnTitles titles{
	"ID", "Game Title", "Completion", "File Path"
};
constexpr ColumnWidths widths{ 30, 230, 110, 170 };

// static assertion not needed anymore since it's checked already via constexpr

constexpr auto bCancelScan{ false };
// put this up here to reduce params
constexpr auto ROM_MAX_SIZE{ std::size_t{ 6 * 1024 * 1024 } };


using GbaExtentions  = std::array<cstring, 9>;
using GbcExtentions  = std::array<cstring, 6>;
using GbExtentions   = std::array<cstring, 7>;
using GensExtentions = std::array<cstring, 6>;
using N64Extentions  = std::array<cstring, 12>;
using NesExtentions  = std::array<cstring, 9>;
using PceExtentions  = std::array<cstring, 4>; // Never gonna test it, the emu is unreliable
using S32xExtentions = std::array<cstring, 2>;
using ScdExtentions  = std::array<cstring, 3>;
using SmsExentions   = std::array<cstring, 5>;
using SnesExtentions = std::array<cstring, 11>;

#pragma region Sega
// Not sure if these should be globals or locals

constexpr GensExtentions gens_ext{
"*.smd", "*.md", "*.bin", "*.gen", "*.zip", ".zsg"
};
constexpr S32xExtentions s32x_ext{ "*.32x", "*.zip" };
constexpr ScdExtentions scd_ext{ "*.bin", "*.iso", "*.raw" };
constexpr SmsExentions sms_ext{ "*.sms", "*.sg", "*.sc", "*.sf7", "*.bin" };
#pragma endregion

#pragma region Nintendo
constexpr GbExtentions gb_ext{
	"*.gb", "*.dmg", "*.sgb", "*.zip", "*.7z", "*.z", "*.gz"
};
constexpr GbcExtentions gbc_ext{
	"*.gbc", "*.cgb", "*.zip", "*.7z", "*.z", "*.gz"
};
constexpr GbaExtentions gba_ext{
	"*.gba", "*.agb", "*.bin", "*.elf", "*.mb", "*.zip", "*.7z", "*.z", "*.gz"
};

constexpr N64Extentions n64_ext{
	"*.n64", "*.v64", "*.z64", "*.u64", "*.usa", "*.jap", "*.pal", "*.eur",
	"*.bin", "*.ndd", "*.zip", "*.7z"
};

// There's also *.pas but I think only Nestopia supports that
constexpr NesExtentions nes_ext{
	"*.nes", "*.nsf", "*.fds", "*.unf", "*.zip", "*.rar", "*.7z",
	"*.zip", "*.gz"
};

constexpr SnesExtentions snes_ext{
	"*.smc", "*.zip", "*.gz", "*.swc", "*.fig", "*.sfc", "*.jma", "*.mgd",
	"*.7z", "*.rar", "*.078"
};
#pragma endregion



std::mutex mtx;






// Not too sure about this one, gonna leave it alone for the most part
// Seems almost identical to another function
// but should be a member function, haven't seen it used outside this class
_Use_decl_annotations_
bool Dlg_GameLibrary::ListFiles(std::string path, std::string mask,
	FileQueue& rFileList)
{

	std::stack<std::string> directories;
	// if you're gonna push it you may as well move it
	directories.push(std::move(path));

	while (!directories.empty())
	{
		path = directories.top();
		auto spec{ tfm::format("%s\\%s", path, mask) };
		directories.pop();

		WIN32_FIND_DATA ffd;
		auto hFind{ FindFirstFile(NativeStr(spec).c_str(), &ffd) };
		if (hFind == INVALID_HANDLE_VALUE)
			return false;

		do
		{
			// repetition... plus strings have operator overloads...
			auto sFilename = Narrow(ffd.cFileName);
			if ((sFilename == ".") || (sFilename == ".."))
				continue;

			auto full_path{ tfm::format("%s\\%s", path, sFilename) };
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				directories.push(std::move(full_path));
			else
				rFileList.push_front(std::move(full_path));
		} while (FindNextFile(hFind, &ffd) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			FindClose(hFind);
			return false;
		}

		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;

	} // end while
	return true;

}  // end function ListFiles


Dlg_GameLibrary::Dlg_GameLibrary() :
	IRA_Dialog{ IDD_RA_GAMELIBRARY }
{
} // end constructor


#pragma region Parsing stuff
// This was only called for this dialog, should be a member function
// I definitly know what to do here, significantly less complex
// We could template this out since there seems to repetition
_Use_decl_annotations_
void Dlg_GameLibrary::ParseGameHashLibraryFromFile(GameHashLib& gamehash_libary)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	std::ifstream ifile{ RA_GAME_HASH_FILENAME, std::ios::binary };

	Document doc;
	doc.ParseStream(IStreamWrapper{ ifile });

	if (!doc.HasParseError() && doc.HasMember("Success") &&
		doc["Success"].GetBool() && doc.HasMember("MD5List"))
	{
		const auto& List{ doc["MD5List"] };
		// I swear I've seen this function before...
		for (auto iter = List.MemberBegin(); iter != List.MemberEnd(); ++iter)
		{
			if (iter->name.IsNull() || iter->value.IsNull())
				continue;


			gamehash_libary.emplace(iter->name.GetString(),
				iter->value.GetUint());
		} // end for
	} // end if
} // end function ParseGameHashLibraryFromFile


_Use_decl_annotations_
void Dlg_GameLibrary::ParseGameTitlesFromFile(GameTitleLib& gametitle_libary)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	std::ifstream ifile{ RA_TITLES_FILENAME, std::ios::binary };

	Document doc;
	doc.ParseStream(IStreamWrapper{ ifile });

	if (!doc.HasParseError() && doc.HasMember("Success") &&
		doc["Success"].GetBool() && doc.HasMember("Response"))
	{
		const auto& List{ doc["Response"] };
		// I swear I've seen this function before...
		for (auto iter = List.MemberBegin(); iter != List.MemberEnd(); ++iter)
		{
            // This should not be allowed to happen
			if (iter->name.IsNull() || iter->value.IsNull())
				continue;

			const auto nID{ std::stoul(iter->name.GetString()) };
			// Someone needs to check how this is done because this
			// happens everywhere, it overflows if checked

            // Finally remembered, it's because this is the wrong way to add
            // stuff to a dictionary

			gametitle_libary.emplace(nID, iter->value.GetString());
		} // end for
	} // end if
} // end function ParseGameTitlesFromFile

_Use_decl_annotations_
void Dlg_GameLibrary::ParseMyProgressFromFile(ProgressLib& progress_library)
{
	SetCurrentDirectory(NativeStr(g_sHomeDir).c_str());
	std::ifstream ifile{ RA_MY_PROGRESS_FILENAME, std::ios::binary };

	Document doc;
	doc.ParseStream(IStreamWrapper{ ifile });

	if (!doc.HasParseError() && doc.HasMember("Success") &&
		doc["Success"].GetBool() && doc.HasMember("Response"))
	{
		const auto& List{ doc["Response"] };
		// I swear I've seen this function before...
		for (auto iter = List.MemberBegin(); iter != List.MemberEnd(); ++iter)
		{
			auto nID{ static_cast<GameID>(std::stoul(iter->name.GetString())) };	//	KEYS MUST BE STRINGS
			const auto nNumAchievements{ iter->value["NumAch"].GetUint() };
			const auto nEarned{ iter->value["Earned"].GetUint() };
			const auto nEarnedHardcore{ iter->value["HCEarned"].GetUint() };

			std::stringstream sstr;
			sstr << nEarned;
			if (nEarnedHardcore > 0)
				tfm::format(sstr, "(%d)", nEarnedHardcore);

			tfm::format(sstr, " / %d", nNumAchievements);

			if (nNumAchievements > 0)
			{
				const auto nNumEarnedTotal = nEarned + nEarnedHardcore;
				auto res{ static_cast<float>(nNumEarnedTotal) / static_cast<float>(nNumAchievements) * 100.0f };
				tfm::format(sstr, " (%1.1f%%)", res);
			} // end if
			// again? damn
			progress_library.emplace(nID, sstr.str());
		} // end for
	} // end if
} // end function ParseMyProgressFromFile  
#pragma endregion




void Dlg_GameLibrary::SetupColumns(HWND hList)
{
	//	Remove all columns,
	while (ListView_DeleteColumn(hList, 0)) {}

	//	Remove all data.
	ListView_DeleteAllItems(hList);

	// I'm a damn idiot...
	// int because casting is annoying, listview uses int
	auto pos{ 0_i };
	// Oh no... this looks like a recipe for disater... changing it
	for (auto& i : titles)
	{


		// also for the array to be constexpr it can't have std::string
		tstring sCol{ titles.at(pos) };	//	scoped cache

		// based off the struct
		LV_COLUMN col{
			UINT{LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT},
			LVCFMT_LEFT | LVCFMT_FIXED_WIDTH,
			widths.at(pos),
			sCol.data(),
			255,
			pos

		};

		if (pos == num_col - 1)	//	Final column should fill
			col.fmt |= LVCFMT_FILL;

		// macro already casts it
		ListView_InsertColumn(hList, i, &col);

		pos++;
	} // end for
} // end function SetupColumns

void Dlg_GameLibrary::AddTitle(GameEntry&& game_entry)
{
	auto my_entry{ std::move(game_entry) };

	// TODO: make this a class
	LV_ITEM item{ LVIF_TEXT, 0, 0, 0_z, 0_z, nullptr, 255 };


	item.iItem = static_cast<std::intptr_t>(m_vGameEntries.size());


	//	id:
	//auto sID{  };	//scoped cache!
	item.pszText = std::to_string(my_entry.m_nGameID).data();
	item.iItem   = ListView_InsertItem(m_hList, &item);

	item.iSubItem = 1;
	ListView_SetItemText(m_hList, item.iItem, 1, my_entry.m_sTitle.data());

	item.iSubItem = 2;
	ListView_SetItemText(m_hList, item.iItem, 2,
		m_ProgressLibrary.at(my_entry.m_nGameID).data());

	item.iSubItem = 3;
	ListView_SetItemText(m_hList, item.iItem, 3, my_entry.m_sFilename.data());

	m_vGameEntries.push_back(std::move(my_entry));
} // end function AddTitle

void Dlg_GameLibrary::ClearTitles()
{
	nNumParsed = 0;

	m_vGameEntries.clear();

	ListView_DeleteAllItems(m_hList);
	VisibleResults.clear();
}

// This is verly likely to throw, but we want to see what they are so we can
// handle them
void Dlg_GameLibrary::ThreadedScanProc() //noexcept
{
	ThreadProcessingActive = true;

	while (FilesToScan.size() > 0)
	{
		// scoped mutex might be better...
		mtx.lock();
		if (!ThreadProcessingAllowed)
		{
			mtx.unlock();
			break;
		} // end if
		mtx.unlock();



		// weird... simple test
		// ok it works now, needs to use value_type instead of actual type
		// is_char_v is in ra not in std, it doesn't exist there
		static_assert(is_char_v<std::string::value_type>);

		// obtain file size:
        // Ok it's giving me a real number now
        // Checking "Zunou Senkan Galg (Japan).nes"
		// size: 40.0 KB (40,976 bytes)
        // size on disk: 44.0 KB (45,056 bytes)
        // fsize: 40976, YES!!!!!!!!! This was the problem, very proud to have figured it out.

		auto fsize{
			static_cast<std::streamsize>(filesize(FilesToScan.front()))
		};
		// a bit a strech 
		
        // would it make more sense to use the numeric_limit of unsigned char
#undef max
        // don't care, C++ has one anyway
		constexpr auto cap{ 6 * 1024 * 1024 }; // 7


        // OK, it can't use conversions inside it
        

		// This opens and closes automatically, otherwise throws an exception
		// if this file always exists it won't throw
		// let's try something else
		//auto dfilename{stodata_stream(FilesToScan.front())};
		Data_ifstream ifile{ FilesToScan.front(), std::ios::binary};
        // ................
		auto dstr = new BYTE[cap];
        // I hate pointers and their damn access violations
        // Still wrong MD5, last resort
		ifile.read(dstr, fsize);
        
        // this crap is giving the wrong MD5
        // Getting this weird string
        // game: "Zunou Senkan Galg (Japan).nes"
        // str: "NES\x1a\x2\x1" is that expected?
        // It's getting closer but still wrong
        // What we need:        f5a9605e36ce33f2d6b9160f7d5124ae
        // A little bit closer: f23b3fb954701b22ca16258e25d3e1c4
        // Tried the old way and it made no difference

        // These all give different results, none of them are correct!
        // just to see it faster
		auto md5_tmp = RAGenerateMD5(DataStream{ dstr }); // "729af7ee2cf263834c44850ce43fb694"
		auto md5_tmp2 = RAGenerateMD5(DataStreamAsString(dstr)); // "f23b3fb954701b22ca16258e25d3e1c4"
		auto md5_tmp3 = RAGenerateMD5(dstr, static_cast<size_t>(fsize)); // "6feacb4ce68100d10d50336912a5bd12", "f14332e7a1fd953d07f3dfcb25026535"

		Results.emplace(FilesToScan.front(), RAGenerateMD5(DataStream{ dstr }));

		if (dstr)
			delete dstr;
		// WM_TIMER is a nonqueued message so it's posted
		// Could use OnTimer instead but doesn't have a TimerProc or default
		OnTimer(m_hGameLibWnd, 0U);

		mtx.lock();
		FilesToScan.pop_front();
		mtx.unlock();
	} // end while

	ThreadProcessingActive = false;
	ExitThread(0);
} // end function ThreadedScanProc


// put this ina region to show it was originally one function
#pragma region ScanAndAddRomsRecursive
void Dlg_GameLibrary
::ScanAndAddRomsRecursive(const std::string& sBaseDir)
{
	// this looked like all kinds of wrong...

	auto sSearchDir{ tfm::format("%s\\*.*", sBaseDir) };

	WIN32_FIND_DATA ffd;
	auto hFind{ FindFirstFile(NativeStr(sSearchDir).c_str(), &ffd) };

	// We could reduce complexity by using Contracts but not really trying
	// to change everything to depend on GSL for now

	if (hFind != INVALID_HANDLE_VALUE)
	{


		DataStream sROMRawData;
		sROMRawData.reserve(ROM_MAX_SIZE);

		do
		{
			if (KEYDOWN(VK_ESCAPE))
				break;

			const auto sFilename{ Narrow(ffd.cFileName) };
			if ((sFilename == ".") || (sFilename == ".."))
			{
				//	Ignore 'this'
			} // end if
			else if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				RA_LOG("Directory found: %s\n", ffd.cFileName);
				auto sRecurseDir{ tfm::format("%s\\%s", sBaseDir, sFilename) };
				ScanAndAddRomsRecursive(sRecurseDir);
			} // end else if
			else
				TryToParseFile(ffd, sFilename, sBaseDir, sROMRawData);
		} while (FindNextFile(hFind, &ffd) != 0); // end do..while

		_CSTD FindClose(hFind);
	}
	SetText(m_hScannerFoundInfo, TEXT("Scanning complete"));
} // end function ScanAndAddRomsRecursive

void Dlg_GameLibrary::TryToParseFile(WIN32_FIND_DATA& ffd,
	const std::string& sFilename, const std::string& sBaseDir,
	DataStream& sROMRawData)
{
	// msft needs to get there shit together...
	auto high_part{ static_cast<LONG>(ffd.nFileSizeHigh) };
	LARGE_INTEGER filesize{ ffd.nFileSizeLow, high_part };
	//	Ignore: wrong size
	if (filesize.QuadPart < 2048 || filesize.QuadPart > ROM_MAX_SIZE)
		RA_LOG("Ignoring %s, wrong size\n", sFilename.c_str());
	else
	{
		//	Parse as ROM!
		RA_LOG("%s looks good: parsing!\n", sFilename);

		auto sAbsFileDir{
			tfm::format("%s\\%s", sBaseDir, sFilename)
		};

		auto hROMReader{
			// We could make a wrapper function for this
			CreateFile(NativeStr(sAbsFileDir).c_str(),
			GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr)
		};

		PrepareToAddGame(hROMReader, sROMRawData, sAbsFileDir);
	} // end else
} // end function TryToParseFile

void Dlg_GameLibrary::PrepareToAddGame(const HANDLE& hROMReader,
	DataStream& sROMRawData, std::string& sAbsFileDir)
{
	if (hROMReader != INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION File_Inf;
		auto nSize{ 0 };
		if (GetFileInformationByHandle(hROMReader, &File_Inf))
		{
			nSize = (File_Inf.nFileSizeHigh << 16) +
				File_Inf.nFileSizeLow;
		} // end if

		auto nBytes{ 0_dw };
		auto bResult{ ReadFile(hROMReader,
			reinterpret_cast<LPVOID>(sROMRawData.data()),
			nSize, &nBytes, nullptr) };

		const auto sHashOut{
			RAGenerateMD5(sROMRawData.c_str(), nSize)
		};

		// lots of repeition, it's ok!
		AddGameFromHashLibrary(sHashOut, sAbsFileDir);
		CloseHandle(hROMReader);
	} // end if
} // end function PrepareToAddGame

void Dlg_GameLibrary::AddGameFromHashLibrary(const std::string& sHashOut,
	std::string& sAbsFileDir)
{
	if (m_GameHashLibrary.find(sHashOut) !=
		m_GameHashLibrary.end())
	{
		const auto nGameID{
			m_GameHashLibrary.at(sHashOut)
		};
		// tinyformat doesn't require it since it uses
		// ostringstream
		RA_LOG("Found one! Game ID %d (%s)", nGameID,
			m_GameTitlesLibrary.at(nGameID));

		const auto& sGameTitle{
			m_GameTitlesLibrary.at(nGameID)
		};

		SetText(m_hGlibName,
			NativeStr(sGameTitle).c_str());

		// This seems send WM_ERASEBKRND and WM_NCPAINT
		InvalidateRect(m_hGameLibWnd, nullptr, TRUE);
		AddTitle(GameEntry{ sGameTitle, sAbsFileDir,
			nGameID });
	} // end if
} // end function AddGameFromHashLibrary
#pragma endregion



void Dlg_GameLibrary::ReloadGameListData()
{
	ClearTitles();

	auto len{ GetTextLength(m_hRomDir) };
	tstring sRomDir;
	sRomDir.reserve(len);
	GetText(m_hRomDir, len, sRomDir.data());

	// looks weird but correct
	sRomDir = sRomDir.data();

	mtx.lock();
	// I swear to god I've seen this already
	while (FilesToScan.size() > 0)
		FilesToScan.pop_front();
	mtx.unlock();


	// for reference
	// ListFiles(Narrow(sRomDir), "*.bin", FilesToScan)

	// It could just be me, but I think we need to account for all rom file types
	// This seems it was just for genesis
	bool bOK{ true };
	ReloadByConsoleId(bOK, sRomDir);


	if (bOK)
	{
		// NOW I goddamn remember...
		using namespace std::placeholders;  // for _1, _2, _3...
		std::function<void()> thread_proc =
			std::bind(&Dlg_GameLibrary::ThreadedScanProc, this);
		std::thread scanner{ thread_proc };
		scanner.detach();
	}
}

// This function is complex but (score of 23, but can't figure out to make it
// simplier, breaking up the function won't do anything) It's still linear in
// the average case though so that's good If you guys have an idea, and it
// works, go ahead
_Use_decl_annotations_
void Dlg_GameLibrary::ReloadByConsoleId(bool& bOK, tstring& sRomDir)
{
	// need to check if this is honored
	switch (g_ConsoleID)
	{
	case ConsoleID::MegaDrive:
	{
		for (auto& i : gens_ext)
			bOK |= ListFiles(Narrow(sRomDir), i, FilesToScan);
	}
	break;
	case ConsoleID::N64:
	{
		for (auto& i : n64_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	break;
	case ConsoleID::SNES:
	{
		for (auto& i : snes_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	break;
	case ConsoleID::GB:
	{
		for (auto& i : gb_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	break;
	case ConsoleID::GBA:
	{
		for (auto& i : gba_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	case ConsoleID::GBC:
	{
		for (auto& i : gbc_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	break;
	case ConsoleID::NES:
	{
		for (auto& i : nes_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	break;
	case ConsoleID::PCEngine:
	{
		MessageBox(GetActiveWindow(), _T("Not implemented!"), _T("Warning!"),
			MB_OK);
	}
	break;
	case ConsoleID::SegaCD:
	{
		for (auto& i : scd_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	break;
	case ConsoleID::Sega32X:
	{
		for (auto& i : s32x_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	break;
	case ConsoleID::MasterSystem:
	{
		for (auto& i : sms_ext)
			bOK |= ListFiles(sRomDir, i, FilesToScan);
	}
	//default: // don't feel like handing it right now
	}
}

void Dlg_GameLibrary::RefreshList()
{
	// damn cuz... it's ok!
	for (auto& i : Results)
	{
		auto filepath{ i.first };
		auto md5{ i.second };

		if (VisibleResults.find(filepath) == VisibleResults.end())
		{
			//	Not yet added,
			if (m_GameHashLibrary.find(md5) != m_GameHashLibrary.end())
			{
				//	Found in our hash library!
				// N.B.: MD5 is known to have frequent collisions
				const auto nGameID{ m_GameHashLibrary.at(md5) };
				RA_LOG("Found one! Game ID %d (%s)", nGameID,
					m_GameTitlesLibrary.at(nGameID).c_str());

				const auto& sGameTitle{ m_GameTitlesLibrary.at(nGameID) };

				// Need to check these were drained, if so, we have to put this
				// last
				
				SetText(m_hScannerFoundInfo, NativeStr(sGameTitle).c_str());
				VisibleResults.at(filepath) = md5;	//	Copy to VisibleResults
				AddTitle(GameEntry{ sGameTitle, filepath, nGameID });
			} // end if
		} // end if
	} // end for
} // end function RefreshList

bool Dlg_GameLibrary::LaunchSelected()
{
	if (const auto nSel{ ListView_GetSelectionMark(m_hList) }; nSel != -1)
	{
		auto len{ GetTextLength(m_hList) };
		// it's a little different
		tstring buffer;
		buffer.reserve(len);

		ListView_GetItemText(m_hList, nSel, 1, buffer.data(), len);
		SetText(m_hGlibName, buffer.c_str());

		ListView_GetItemText(m_hList, nSel, 3, buffer.data(), len);
		_RA_LoadROM(Narrow(buffer).c_str());

		return true;
	} // end if
	else
		return false;
} // end function LaunchSelected

void Dlg_GameLibrary::LoadAll()
{
	mtx.lock();
	// this one does need to be in binary mode if you want to know the characters read
	std::ifstream ifile{ RA_MY_GAME_LIBRARY_FILENAME, std::ios::binary };

	auto nCharsRead1{ 0_ss };
	auto nCharsRead2{ 0_ss };
	do
	{
		// you need them to be 0 each time?
		nCharsRead1 = 0_ss;
		nCharsRead2 = 0_ss;

		std::string filebuf;
		std::string md5buf;

		// You know there's getline() right?
		ifile.getline(filebuf.data(), 2048);

		if (nCharsRead1 = ifile.gcount(); nCharsRead1 > 0_i)
		{
			ifile.getline(md5buf.data(), 64);
			nCharsRead2 = ifile.gcount() - nCharsRead1;
		} // end if

		// I don't how that's possible but w/e
		if (((filebuf.front() != '\0') && (md5buf.front() != '\0')) &&
			((nCharsRead1 > 0) && (nCharsRead2 > 0)))
		{
			// Strings already null-terminated, but yeah character arrays are not
			//	Add
			Results.at(filebuf) = md5buf;
		} // end if

	} while ((nCharsRead1 > 0_i) && (nCharsRead2 > 0_i)); // end do..while

	mtx.unlock();
} // end function LoadAll

void Dlg_GameLibrary::SaveAll()
{
	mtx.lock();
	// doesn't seem to need to be in binary mode, are we reading dat files?

	std::ofstream ofile{ RA_MY_GAME_LIBRARY_FILENAME };

	// seemed more complicated than it needs to be
	for (auto& i : Results)
		tfm::format(ofile, "%s\n%s\n", i.first, i.second);

	mtx.unlock();
} // end function SaveAll


void Dlg_GameLibrary::KillThread()
{
	ThreadProcessingAllowed = false;
	while (Dlg_GameLibrary::ThreadProcessingActive)
	{
		RA_LOG("Waiting for background scanner...");
		Sleep(200);
	}
}

#pragma region Message Handlers
BOOL Dlg_GameLibrary::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	// Might as well initlize handles to the controls here
	// FINALLY FIGURED IT OUT!
	m_hList             = GetDlgItem(hwnd, IDC_RA_LBX_GAMELIST);
	m_hRomDir           = GetDlgItem(hwnd, IDC_RA_ROMDIR);
	m_hGlibName         = GetDlgItem(hwnd, IDC_RA_GLIB_NAME);
	m_hScannerFoundInfo = GetDlgItem(hwnd, IDC_RA_SCANNERFOUNDINFO);

	SetupColumns(m_hList);

	ListView_SetExtendedListViewStyle(m_hList,
		LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	SetText(m_hRomDir, NativeStr(g_sROMDirLocation).c_str());
	SetText(m_hGlibName, TEXT(""));

	m_GameHashLibrary.clear();
	m_GameTitlesLibrary.clear();
	m_ProgressLibrary.clear();
	ParseGameHashLibraryFromFile(m_GameHashLibrary); // function is fine after switching to a hash table
	ParseGameTitlesFromFile(m_GameTitlesLibrary);
	ParseMyProgressFromFile(m_ProgressLibrary);

	//int msBetweenRefresh = 1000;	//	auto?
	//SetTimer( hwnd, 1, msBetweenRefresh, (TIMERPROC)g_GameLibrary.s_GameLibraryProc );
	RefreshList();
	// Bounds are a serious problem in this program
	return FALSE;
} // end function OnInitDialog

void Dlg_GameLibrary::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    // Alright initially codeNotify is 1024(not sure),
	// and id is 1593 (IDC_RA_ROMDIR)
    // We made hwndCtls in to members because I don't know which control it is
    // IDC_GLIBNAME is getting passed now...
	switch (id)
	{
	case IDOK:
		OnOK(hwnd);
		break;

	case IDC_RA_RESCAN:
		OnRescan();
		break;

	case IDC_RA_PICKROMDIR:
		OnPickRomDir();
		break;

	case IDC_RA_LBX_GAMELIST:
		// This seems identical in OnNotify, it is
		OnLbxGamelist();
		break;

	case IDC_RA_REFRESH:
		RefreshList();
	} // end switch
} // end function OnCommand

void Dlg_GameLibrary::OnLbxGamelist()
{
	const auto nSel{ ListView_GetSelectionMark(m_hList) };
	if (nSel != -1)
	{
		auto length{ GetTextLength(m_hList) };
		tstring buffer;
		buffer.reserve(length);

		ListView_GetItemText(m_hList, nSel, 1, buffer.data(), length);
		SetText(m_hGlibName, buffer.c_str());
	} // end if
} // end function OnLbxGamelist

void Dlg_GameLibrary::OnPickRomDir()
{
	g_sROMDirLocation = GetFolderFromDialog();

	RA_LOG("Selected Folder: %s\n", g_sROMDirLocation);
	SetText(m_hRomDir, NativeStr(g_sROMDirLocation).c_str());
} // end function OnPickRomDir

void Dlg_GameLibrary::OnRescan()
{
	ReloadGameListData();

	// I really think a scoped mutex is more approprate if this is synchronous
	mtx.lock();	//?
	SetText(m_hScannerFoundInfo, TEXT("Scanning..."));
	mtx.unlock();
} // end function OnRescan

void Dlg_GameLibrary::OnTimer(HWND hwnd, UINT id)
{
	// It should be impossible (first one), or else it will crash
	// How can a nonexistent window be visible?
	if (IsWindowVisible(hwnd))
		RefreshList();
} // end function OnTimer

// HANDLE_WM_NOTIFY
// This is here just to quickly look at the cracker
LRESULT Dlg_GameLibrary::OnNotify(HWND hwnd, int idFrom, NMHDR * pnmhdr)
{
	switch (idFrom)
	{
	case IDC_RA_LBX_GAMELIST:
	{
		switch (pnmhdr->code)
		{
		case LVN_ITEMCHANGED:
			OnLbxGamelist();
			break;

		case NM_CLICK:
			break;

		case NM_DBLCLK:
			OnOK(hwnd);
			return 1_i;
		} // end switch
	} // end case IDC_RA_LBX_GAMELIST
	return 0_i; // LRESULT could be long or long long

	default:
		RA_LOG("%08x, %08x\n", idFrom, pnmhdr);
		return 0_i;
	} // end switch
} // end function OnNotify

void Dlg_GameLibrary::OnOK(HWND hwnd)
{
	if (LaunchSelected())
		Close(hwnd);
} // end function OnOK

void Dlg_GameLibrary::OnPaint([[maybe_unused]] HWND hwnd)
{
	if (nNumParsed != Results.size())
		nNumParsed = Results.size();
} // end function OnPaint

void Dlg_GameLibrary::OnNCDestroy(HWND hwnd)
{
	DestroyControl(m_hList);
	DestroyControl(m_hRomDir);
	DestroyControl(m_hGlibName);
	DestroyControl(m_hScannerFoundInfo);
	IRA_Dialog::OnNCDestroy(hwnd);
} // end function OnNCDestroy

INT_PTR Dlg_GameLibrary::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	return 0;
} // end function DialogProc  
#pragma endregion

} // namespace ra
