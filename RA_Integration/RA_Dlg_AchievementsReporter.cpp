#include "stdafx.h"
#include "RA_Dlg_AchievementsReporter.h"

#include "RA_Achievement.h"
#include "RA_AchievementSet.h"
#include "RA_Core.h"
#include "RA_Defs.h"
#include "RA_GameData.h"
#include "RA_httpthread.h"
#include "RA_Resource.h"
#include "RA_User.h"

using std::array;
using gsl::narrow_cast;
using std::string;
using std::move;

// NB: The only difference is that this won't crash the app and the
//     arrays are built at compile-time instead of runtime. constexpr
// is what static_assert has been trying to do but fails at it. It also
// alleviates the stack.

// TODO: unnamed namespaces may lead to unhandled exceptions at runtime
// NB: It's universally considered bad practice to use all caps, except
//     if it's a symbolic constant (define)
namespace
{
constexpr array<const char*, 5> COL_TITLE{"", "Title", "Description", "Author", "Achieved?"};
constexpr array<size_t, 5> COL_SIZE{19, 105, 205, 75, 62};

// It's impossible for them not to be the same size now
//static_assert(SIZEOF_ARRAY( COL_TITLE ) == SIZEOF_ARRAY( COL_SIZE ), "Must match!");

constexpr array<const char*, 3> PROBLEM_STR{
	"Unknown", "Triggers at wrong time", "Didn't trigger at all"
};
}

// There's really no point in having static data members in C++, it disables the destructor
size_t Dlg_AchievementsReporter::ms_nNumOccupiedRows{0};

// I know C# has Vector3 and Vector4 but I don't think this is for a 3d game.
char Dlg_AchievementsReporter::ms_lbxData[MAX_ACHIEVEMENTS][NumReporterColumns][MAX_TEXT_LEN];

Dlg_AchievementsReporter g_AchievementsReporterDialog;

void Dlg_AchievementsReporter::SetupColumns( HWND hList )
{
	//	Remove all columns,
	while ( ListView_DeleteColumn( hList, 0 ) ) {}

	//	Remove all data.
	ListView_DeleteAllItems( hList );

	LV_COLUMN col;
	ZeroMemory( &col, sizeof( col ) );

	for ( size_t i = 0; i < SIZEOF_ARRAY( COL_TITLE ); ++i )
	{
		col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		col.cx = COL_SIZE[i];
		col.cchTextMax = 255;
		std::wstring sStrWide = Widen( COL_TITLE[i] );
		col.pszText = const_cast<LPWSTR>(sStrWide.c_str());
		col.iSubItem = i;

		col.fmt = LVCFMT_LEFT | LVCFMT_FIXED_WIDTH;
		if ( i == SIZEOF_ARRAY( COL_TITLE ) - 1 )
			col.fmt |= LVCFMT_FILL;

		ListView_InsertColumn( hList, i, reinterpret_cast<LPARAM>(&col) );
	}

	ms_nNumOccupiedRows = 0;
}

int Dlg_AchievementsReporter::AddAchievementToListBox( HWND hList, const Achievement* pAch )
{
	for ( size_t i = 0; i < NumReporterColumns; ++i )
	{
		switch ( i )
		{
		case Checked:
			sprintf_s( ms_lbxData[ms_nNumOccupiedRows][i], MAX_TEXT_LEN, "" );
			break;
		case Title:
			sprintf_s( ms_lbxData[ms_nNumOccupiedRows][i], MAX_TEXT_LEN, pAch->Title().c_str() );
			break;
		case Desc:
			sprintf_s( ms_lbxData[ms_nNumOccupiedRows][i], MAX_TEXT_LEN, pAch->Description().c_str() );
			break;
		case Author:
			sprintf_s( ms_lbxData[ms_nNumOccupiedRows][i], MAX_TEXT_LEN, pAch->Author().c_str() );
			break;
		case Achieved:
			sprintf_s( ms_lbxData[ms_nNumOccupiedRows][i], MAX_TEXT_LEN, !pAch->Active() ? "Yes" : "No" );
			break;
		default:
			ASSERT( !"Unknown col!" );
			break;
		}
	}

	LV_ITEM item;
	ZeroMemory( &item, sizeof( item ) );

	item.mask = LVIF_TEXT;
	item.cchTextMax = 256;
	item.iItem = ms_nNumOccupiedRows;

	for ( size_t i = 0; i < NumReporterColumns; ++i )
	{
		item.iSubItem = i;
		std::wstring sStrWide = Widen( ms_lbxData[ms_nNumOccupiedRows][i] );	//Scoped cache
		item.pszText = const_cast<LPWSTR>(sStrWide.c_str());

		if ( i == 0 )
			item.iItem = ListView_InsertItem( hList, &item );
		else
			ListView_SetItem( hList, &item );
	}

	ASSERT( item.iItem == ms_nNumOccupiedRows );

	ms_nNumOccupiedRows++;	//	Last thing to do!
	return item.iItem;
}

// This is exactly why we need structure, the function is too big to be analyzed.
INT_PTR CALLBACK Dlg_AchievementsReporter::AchievementsReporterProc( HWND hDlg, UINT uMsg,
	WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		HANDLE_MSG( hDlg, WM_INITDIALOG, achrep_OnInitDialog );
		HANDLE_MSG( hDlg, WM_COMMAND, achrep_OnCommand );
		HANDLE_MSG( hDlg, WM_CLOSE, achrep_OnClose );
	default:
		return FALSE; // This should actually go to the message router but it's not here yet
	} // end switch
}

//static
// Again, Modals weren't designed to be static, C# and JavaScript have Modals too.
// There's a lot of good training videos at Microsoft Virtual Academy
void Dlg_AchievementsReporter::DoModalDialog( HINSTANCE hInst, HWND hParent )
{
	if ( g_pActiveAchievements->NumAchievements() == 0 )
		MessageBox( hParent, L"No ROM loaded!", L"Error", MB_OK );
	else
		DialogBox( hInst, MAKEINTRESOURCE( IDD_RA_REPORTBROKENACHIEVEMENTS ), hParent, AchievementsReporterProc );
}

void achrep_OnClose( HWND hwnd )
{
	EndDialog( hwnd, IDCLOSE );
	PostQuitMessage( 0 );
}

// TODO: Ask what notification code should be returned at each failure
void achrep_OnCommand( HWND hDlg, int id, HWND hwndCtl, UINT codeNotify )
{
	switch ( LOWORD( id ) )
	{
	case IDOK:
		achrep_OnOK();
		break;

	case IDCANCEL:
		OnCancel();
		break;
	} // end switch
} // end function achrep_OnCommand

BOOL achrep_OnInitDialog( HWND hwnd, HWND hwndFocus, LPARAM lParam )
{
	auto hList{GetDlgItem( hwnd, IDC_RA_REPORTBROKENACHIEVEMENTSLIST )};
	Dlg_AchievementsReporter::SetupColumns( hList );

	// NB: prefer post-increment for primitives and pre-increment to objects
	for ( size_t i = 0; i < g_pActiveAchievements->NumAchievements(); i++ )
	{
		Dlg_AchievementsReporter::AddAchievementToListBox( hList,
			&g_pActiveAchievements->GetAchievement( i ) );
	}

	ListView_SetExtendedListViewStyle( hList, LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP );
	SetDlgItemText(
		hwnd, IDC_RA_BROKENACH_BUGREPORTER, Widen( RAUsers::LocalUser().Username() ).c_str()
	);

	// I don't know which control supposed to have focus... FALSE doesn't set focus to anything
	return FALSE;
}

void OnOK()
{
	EndDialog( GetActiveWindow(), IDOK );
} // end function OnOK

// I'm almost certain the stack overflow is caused from too many buffer, just have one.
// C++ makes this easy...
void achrep_OnOK()
{
	// temp
	auto hDlg{GetActiveWindow()};
	auto hList{GetDlgItem( hDlg, IDC_RA_REPORTBROKENACHIEVEMENTSLIST )};

	// NB: non-constant expressions should not constant expressions,
	//     it's an oxymoron
	// NB: These can't be const because IsDlgButtonChecked is not a
	//     constant expression, otherwise it wouldn't need to bool.
	auto bProblem1Sel{(IsDlgButtonChecked( hDlg, IDC_RA_PROBLEMTYPE1 ) == BST_CHECKED)};
	auto bProblem2Sel{(IsDlgButtonChecked( hDlg, IDC_RA_PROBLEMTYPE2 ) == BST_CHECKED)};

	// ! is the same thing as ~ in boolean algebra, or NOT. I'm sorry
	// for stating the obvious but it should already be that way.
	// NB: bool is either true/false, 1/0, on/off. These conditions are
	//     a waste of space.

	if ( !(bProblem1Sel) && !(bProblem2Sel) )
	{
		MessageBox( nullptr, L"Please select a problem type.", L"Warning", MB_ICONWARNING );
		return;
	} // end if

	auto nProblemType{bProblem1Sel ? 1 : bProblem2Sel ? 2 : 0};	// 0==?, 0=="Unknown"
	auto sProblemTypeNice{PROBLEM_STR.at( nProblemType )};

	char sBuggedIDs[1024];
	sprintf_s( sBuggedIDs, 1024, "" );

	// NB: You can't have a negative amount of reports
	auto nReportCount{0U};

	// NB: STL array's size is a constant expression, the rhs is not
	// We need a narrowing conversion
	auto nListSize{narrow_cast<size_t>(ListView_GetItemCount( hList ))};
	string buffer{};
	buffer.reserve( 1024 );

	for ( size_t i = 0; i < nListSize; i++ )
	{
		// TODO: Convince everyone that a chunk of classes here can
		//       easily be transformed into containers.
		if ( ListView_GetCheckState( hList, i ) != 0 )
		{
			//	NASTY big assumption here...
			// I'll fix it for you.
			sprintf_s( buffer._Myptr(), buffer.capacity(), "%d,",
				g_pActiveAchievements->GetAchievement( i ).ID() );
			strcat_s( sBuggedIDs, 1024, buffer.c_str() );

			//ListView_GetItem( hList );
			nReportCount++;
		} // end if
	} // end for

	if ( nReportCount > 5 )
	{
		if ( MessageBox( nullptr, L"You have over 5 achievements selected. Is this OK?",
			L"Warning", MB_YESNO ) == IDNO )
		{
			return;
		} // end if
	} // end if

	buffer.clear();

	// reserve != resize
	buffer.reserve( 4096 );

	// size_t might be 8 bytes so a narrowing conversion is required
	GetDlgItemText(
		hDlg, IDC_RA_BROKENACHIEVEMENTREPORTCOMMENT, Widen( buffer )._Myptr(),
		narrow_cast<int>(buffer.capacity())
	);

	// Copy sucks if the rhs isn't gonna be used, that's where move
	// semantics come in, If you're using RapidJSON and C++ you can't
	// complain about move semantics. Those entire libraries are class templates.

	// We need to do this to prevent corruption and overflows
	auto buf2{move( buffer )}; // buffer is empty now, buf2 jacked buffer's data
	buffer.reserve( 8192 );

	// Too long I put it in a region
#pragma region Message
	sprintf_s( buffer._Myptr(), buffer.capacity(),
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
		g_pCurrentGameData->GameTitle().c_str(),
		sBuggedIDs,
		sProblemTypeNice,
		RAUsers::LocalUser().Username().c_str(),
		g_sCurrentROMMD5.c_str(),
		buf2.c_str() );
#pragma endregion

	if ( MessageBox( nullptr, Widen( buffer ).c_str(), L"Summary", MB_YESNO ) == IDNO )
		return;

	PostArgs args;
	args['u'] = RAUsers::LocalUser().Username();
	args['t'] = RAUsers::LocalUser().Token();
	args['i'] = sBuggedIDs;
	args['p'] = std::to_string( nProblemType );
	args['n'] = buf2.c_str();
	args['m'] = g_sCurrentROMMD5;

	Document doc;
	if ( RAWeb::DoBlockingRequest( RequestSubmitTicket, args, doc ) )
	{
		if ( doc["Success"].GetBool() )
		{
			// I'm assuming the previous message was sent
			buffer.clear();
			buffer.reserve( 2048 );

			achrep_OnOK_submitmsg( hDlg, buffer );
			return;
		} // end if
		else
		{
			buffer.clear();
			buffer.reserve( 2048 );
			achrep_OnOK_servererr( doc, hDlg, buffer );
			return;
		} // end else
	} // end if
	else
		achrep_OnOK_lastmsg( hDlg );
} // end function achrep_OnOK

void achrep_OnOK_submitmsg( HWND hDlg, stringref buffer )
{
	Expects( buffer.empty() );
	{
		sprintf_s( buffer._Myptr(), buffer.capacity(), "Submitted OK!\n"
			"\n"
			"Thank you for reporting that bug(s), and sorry it hasn't worked correctly.\n"
			"\n"
			"The development team will investigate this bug as soon as possible\n"
			"and we will send you a message on RetroAchievements.org\n"
			"as soon as we have a solution.\n"
			"\n"
			"Thanks again!" );
		MessageBox( hDlg, Widen( buffer ).c_str(), L"Success!", MB_OK );
		OnOK();
		buffer.clear();
	}
	Ensures( buffer.empty() );
}

void achrep_OnOK_servererr( Document& doc, HWND hDlg, stringref buffer )
{
	Expects( buffer.empty() );
	{
		sprintf_s( buffer._Myptr(), buffer.capacity(),
			"Failed!\n"
			"\n"
			"Response From Server:\n"
			"\n"
			"Error code: %d", doc.GetParseError() );
		MessageBox( hDlg, Widen( buffer ).c_str(), L"Error from server!", MB_OK );
		buffer.clear();
	}
	Ensures( buffer.empty() );
}

void achrep_OnOK_lastmsg( HWND hDlg )
{
	MessageBox( hDlg,
		L"Failed!\n"
		L"\n"
		L"Cannot reach server... are you online?\n"
		L"\n",
		L"Error!", MB_OK );
} // end function achrep_OnOK_lastmsg

void OnCancel()
{
	EndDialog( GetActiveWindow(), IDCANCEL );
} // end function OnCancel
