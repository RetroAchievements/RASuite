#include "RA_Dlg_AchievementsReporter.h"

#include "RA_Achievement.h"
#include "RA_AchievementSet.h"
#include "RA_Core.h"
#include "RA_Defs.h"
#include "RA_GameData.h"
#include "RA_httpthread.h"
#include "RA_Resource.h"
#include "RA_User.h"

namespace ra
{
// Keys are titles and values are widths
// Has to be unordered because we can't have them ordered
// std::map can't be constexpr either




// to not have to some crazy shit
constexpr auto col_num{ 5_z };
constexpr std::array<cstring, col_num> col_names{ "", "Title", "Description", "Author", "Achieved?" };
constexpr std::array<int, col_num> col_sizes{ 19, 105, 205, 75, 62 };
constexpr std::array<cstring, 3> PROBLEM_STR{ "Unknown", "Triggers at wrong time", "Didn't trigger at all" };






void Dlg_AchievementsReporter::SetupColumns(HWND hList)
{
	//	Remove all columns,
	while (ListView_DeleteColumn(hList, 0)) {}

	//	Remove all data.
	ListView_DeleteAllItems(hList);
	auto limit{ static_cast<int>(col_num) };
	for (auto i = 0; i < limit; i++)
	{

		auto sStr{ std::string{col_names.at(i)} }; // you can't do it directly

		// would be easier with delegates...
		// Probably should be made in to a class from repetition
		LV_COLUMN col
		{
			LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT,
			LVCFMT_LEFT | LVCFMT_FIXED_WIDTH,
			col_sizes.at(i),
			sStr.data(),
			255,
			i
		};

		if (i == limit - 1)	//If the last element: fill to the end
			col.fmt |= LVCFMT_FILL;

		ListView_InsertColumn(hList, i, &col);
	}
}

int Dlg_AchievementsReporter::AddAchievementToListBox(HWND hList,
	const Achievement* pAch)
{
	for (auto& i : repcol_arr)
	{
		// I'd suggest initilizing the achievement strings to a fixed capacity using reserve() instead
		auto col{ etoi(i) };
		switch (i)
		{
		case ReporterColumns::Checked:
			ms_lbxData.at(ms_nNumOccupiedRows).at(col) = "";
			break;
		case ReporterColumns::Title:
			ms_lbxData.at(ms_nNumOccupiedRows).at(col) = pAch->Title();
			break;
		case ReporterColumns::Desc:
			ms_lbxData.at(ms_nNumOccupiedRows).at(col) = pAch->Description();
			break;
		case ReporterColumns::Author:
			ms_lbxData.at(ms_nNumOccupiedRows).at(col) = pAch->Author();
			break;
		case ReporterColumns::Achieved:
			ms_lbxData.at(ms_nNumOccupiedRows).at(col) = !pAch->Active() ? "Yes" : "No";
			break;
		default:
			throw std::out_of_range{ "Unknown col!" };
		}
	}




	LV_ITEM item{ LVIF_TEXT, ms_nNumOccupiedRows, 0, 0_z, 0_z, nullptr, 256 };


	for (size_t i = 0; i < num_rep_cols; ++i)
	{
		item.iSubItem = i;
		tstring sStr = ms_lbxData[ms_nNumOccupiedRows][i];	//Scoped cache
		item.pszText = sStr.data();

		if (i == 0)
			item.iItem = ListView_InsertItem(hList, &item);
		else
			ListView_SetItem(hList, &item);
	}

	ASSERT(item.iItem == ms_nNumOccupiedRows);

	ms_nNumOccupiedRows++;	//	Last thing to do!
	return item.iItem;
}

INT_PTR Dlg_AchievementsReporter::DoModal()
{
	if (!g_pActiveAchievements->NumAchievements())
	{
		MessageBox(GetParent(GetActiveWindow()), TEXT("No ROM loaded!"),
			TEXT("Error"), MB_OK);
		return 0;
	}
	else
		return IRA_Dialog::DoModal();
}

BOOL Dlg_AchievementsReporter::OnInitDialog(HWND hwnd, HWND hwndFocus,
	LPARAM lParam)
{
	m_hList = GetDlgItem(hwnd, IDC_RA_REPORTBROKENACHIEVEMENTSLIST);
	SetupColumns(m_hList);

	for (size_t i = 0; i < g_pActiveAchievements->NumAchievements(); ++i)
	{
		AddAchievementToListBox(m_hList,
			&g_pActiveAchievements->GetAchievement(i));
	} // end for

	ListView_SetExtendedListViewStyle(m_hList,
		LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP);

	m_hBugReporter = GetDlgItem(hwnd, IDC_RA_BROKENACH_BUGREPORTER);
	SetText(m_hBugReporter, cusername());
	return FALSE;
} // end function OnInitDialog

void Dlg_AchievementsReporter::OnCommand(HWND hwnd, int id, HWND hwndCtl,
	UINT codeNotify)
{
    // Putting a default check from preventing the warning if closing the window
    // X button in windows is the same as OK button (IDOK)
	m_hProblem1 = GetDlgItem(hwnd, IDC_RA_PROBLEMTYPE1);
	Button_SetCheck(m_hProblem1, TRUE);
	switch (id)
	{
	case IDOK:

		OnOK(hwnd);
		break;

	case IDCANCEL:
		OnCancel(hwnd);
	}
}

void Dlg_AchievementsReporter::OnNCDestroy(HWND hwnd)
{
	Close(m_hList);
	Close(m_hBugReporter);
	Close(m_hProblem1);
	Close(m_hProblem2);
	Close(m_hComment);
	IRA_Dialog::OnNCDestroy(hwnd);
}

void Dlg_AchievementsReporter::OnOK(HWND hwnd)
{

	m_hProblem2 = GetDlgItem(hwnd, IDC_RA_PROBLEMTYPE2);





	const auto bProblem1Sel{ Button_GetCheck(m_hProblem1) };
	const auto bProblem2Sel{ Button_GetCheck(m_hProblem2) };

	if ((bProblem1Sel == false) && (bProblem2Sel == false))
	{
		MessageBox(nullptr, TEXT("Please select a problem type."),
			TEXT("Warning"), MB_ICONWARNING);
		return;
	}
	// 0==?
	auto nProblemType{ bProblem1Sel ? 1 : bProblem2Sel ? 2 : 0 };
	auto sProblemTypeNice{ PROBLEM_STR.at(nProblemType) };

	std::string sBuggedIDs;
	sBuggedIDs.reserve(1024);

	int nReportCount = 0;

	const size_t nListSize = ListView_GetItemCount(m_hList);
	for (size_t i = 0; i < nListSize; ++i)
	{
		if (ListView_GetCheckState(m_hList, i) != 0)
		{
			//	NASTY big assumption here...
			auto buffer{
				tfm::format("%d,",
				g_pActiveAchievements->GetAchievement(i).ID())
			};
			sBuggedIDs+=buffer;

			//ListView_GetItem( hList );	
			nReportCount++;
		}
	}

    // Needs another check
	if (sBuggedIDs == "")
	{
        // even with this it might be strange, there has to be a better way to do this...
        // The close button will still close it even though this warning will show up
		MessageBox(GetActiveWindow(),
			_T("You need to to select at least one achievement"),
			_T("Warning"), MB_OK);
		return;
	}

	if (nReportCount > 5)
	{
		if (MessageBox(nullptr,
			TEXT("You have over 5 achievements selected. Is this OK?"),
			TEXT("Warning"), MB_YESNO) == IDNO)
			return;
	}


	m_hComment = GetDlgItem(hwnd, IDC_RA_BROKENACHIEVEMENTREPORTCOMMENT);

	// Now I remember
	auto len{ GetTextLength(m_hComment) + 1 };
	std::string sBugReportComment;

	// This ones is extremly important or the capacity will change
	sBugReportComment.reserve(static_cast<std::size_t>(len));
	GetText(m_hComment, len, sBugReportComment.data());


	//	Intentionally MBCS
	auto sBugReportInFull{ tfm::format(
		"--New Bug Report--\n"
		"\n"
		"Game: %s\n"
		"Achievement IDs: %s\n"
		"Problem: %s\n"
		"Reporter: %s\n"
		"ROM Checksum: %s\n"
		"\n"
		"Comment: %s\n"
		"\n"
		"Is this OK?",
		g_pCurrentGameData->GameTitle(),
		sBuggedIDs,
		sProblemTypeNice,
		username(),
		g_sCurrentROMMD5,
		sBugReportComment.c_str()) // strange, it won't show itself as a regular string
	};

	if (MessageBox(nullptr, NativeStr(sBugReportInFull).c_str(),
		TEXT("Summary"), MB_YESNO) == IDNO)
		return;

	PostArgs args
	{
		{ 'u', cusername() },
		{ 't', user_token()},
		{ 'i', sBuggedIDs.c_str() },
		{ 'p', std::to_string(nProblemType) },
		{ 'n', sBugReportComment.c_str() },
		{ 'm', g_sCurrentROMMD5.c_str() }
	};

	Document doc;
	// Something is wrong with this function...
	if (RAWeb::DoBlockingRequest(RequestSubmitTicket, args, doc))
	{

		// really weird, success is in there but it's not
		// really bizzare need to check the contents of JSON and do a new approach

		//for (auto& i : doc.GetObjectA())
		//{
		//	RA_LOG("Type of member %s is %s\n", i.name.GetString(),
		//		i.value.GetType());
		//}




		if (doc["Success"].GetBool())
		{
			auto msg{
				"Submitted OK!\n"
				"\n"
				"Thank you for reporting that bug(s), and sorry it hasn't worked correctly.\n"
				"\n"
				"The development team will investigate this bug as soon as possible\n"
				"and we will send you a message on RetroAchievements.org\n"
				"as soon as we have a solution.\n"
				"\n"
				"Thanks again!"
			};

			MessageBox(hwnd, NativeStr(msg).c_str(), TEXT("Success!"), MB_OK);
			// this is so strange, the achievements get sent over but says it's failing
			IRA_Dialog::OnOK(hwnd);
			return;
		}
		else
		{
			auto buffer{ tfm::format(
				"Failed!\n"
				"\n"
				"Response From Server:\n"
				"\n"
				"Error code: %d", doc.GetParseError())
			};
			MessageBox(hwnd, NativeStr(buffer).c_str(), TEXT("Error from server!"), MB_OK);
			return;
		}
	}
	else
	{
		MessageBox(hwnd,
			TEXT("Failed!\n")
			TEXT("\n")
			TEXT("Cannot reach server... are you online?\n")
			TEXT("\n"),
			TEXT("Error!"), MB_OK);
		return;
	}
}

INT_PTR Dlg_AchievementsReporter::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

Dlg_AchievementsReporter::Dlg_AchievementsReporter() :
	IRA_Dialog{ IDD_RA_REPORTBROKENACHIEVEMENTS }
{
}






} // namespace ra
