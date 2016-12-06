#include "RA_Dlg_AchievementsReporter.h"

#include "RA_Achievement.h"
#include "RA_AchievementSet.h"
#include "RA_Core.h"
#include "RA_Defs.h"
#include "RA_httpthread.h"
#include "RA_Resource.h"
#include "RA_User.h"


namespace
{
	const char* COL_TITLE[] = { "", "Title", "Description", "Author", "Achieved?" };
	const int COL_SIZE[] = { 19, 105, 205, 75, 62 };
	static_assert( SIZEOF_ARRAY( COL_TITLE ) == SIZEOF_ARRAY( COL_SIZE ), "Must match!" );
	
	const char* PROBLEM_STR[] = { "Unknown", "Triggers at wrong time", "Didn't trigger at all" };
}

int Dlg_AchievementsReporter::ms_nNumOccupiedRows = 0;
char Dlg_AchievementsReporter::ms_lbxData[ MAX_ACHIEVEMENTS ][ NumReporterColumns ][ MAX_TEXT_LEN ];

Dlg_AchievementsReporter g_AchievementsReporterDialog;

void Dlg_AchievementsReporter::SetupColumns( HWND hList )
{
	//	Remove all columns,
	while( ListView_DeleteColumn( hList, 0 ) ) {}

	//	Remove all data.
	ListView_DeleteAllItems( hList );

	LV_COLUMN col;
	ZeroMemory( &col, sizeof( col ) );

	for( size_t i = 0; i < SIZEOF_ARRAY( COL_TITLE ); ++i )
	{
		col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		col.cx = COL_SIZE[ i ];
		col.cchTextMax = 255;
		std::wstring sStrWide = Widen( COL_TITLE[ i ] );
		col.pszText = const_cast<LPWSTR>( sStrWide.c_str() );
		col.iSubItem = i;

		col.fmt = LVCFMT_LEFT | LVCFMT_FIXED_WIDTH;
		if( i == SIZEOF_ARRAY( COL_TITLE ) - 1 )
			col.fmt |= LVCFMT_FILL;

		ListView_InsertColumn( hList, i, reinterpret_cast<LPARAM>( &col ) );
	}

	ms_nNumOccupiedRows = 0;
}

int Dlg_AchievementsReporter::AddAchievementToListBox( HWND hList, const Achievement* pAch )
{
	for( size_t i = 0; i < NumReporterColumns; ++i )
	{
		switch( i )
		{
		case Checked:
			sprintf_s( ms_lbxData[ ms_nNumOccupiedRows ][ i ], MAX_TEXT_LEN, "" );
			break;
		case Title:
			sprintf_s( ms_lbxData[ ms_nNumOccupiedRows ][ i ], MAX_TEXT_LEN, pAch->Title().c_str() );
			break;
		case Desc:
			sprintf_s( ms_lbxData[ ms_nNumOccupiedRows ][ i ], MAX_TEXT_LEN, pAch->Description().c_str() );
			break;
		case Author:
			sprintf_s( ms_lbxData[ ms_nNumOccupiedRows ][ i ], MAX_TEXT_LEN, pAch->Author().c_str() );
			break;
		case Achieved:
			sprintf_s( ms_lbxData[ ms_nNumOccupiedRows ][ i ], MAX_TEXT_LEN, !pAch->Active() ? "Yes" : "No" );
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

	for( size_t i = 0; i < NumReporterColumns; ++i )
	{
		item.iSubItem = i;
		std::wstring sStrWide = Widen( ms_lbxData[ ms_nNumOccupiedRows ][ i ] );	//Scoped cache
		item.pszText = const_cast<LPWSTR>( sStrWide.c_str() );

		if( i == 0 )
			item.iItem = ListView_InsertItem( hList, &item );
		else
			ListView_SetItem( hList, &item );
	}

	ASSERT( item.iItem == ms_nNumOccupiedRows );

	ms_nNumOccupiedRows++;	//	Last thing to do!
	return item.iItem;
}

INT_PTR CALLBACK Dlg_AchievementsReporter::AchievementsReporterProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:
		{
			HWND hList = GetDlgItem( hDlg, IDC_RA_REPORTBROKENACHIEVEMENTSLIST );
			SetupColumns( hList );

			for( size_t i = 0; i < g_pActiveAchievements->NumAchievements(); ++i )
				AddAchievementToListBox( hList, &g_pActiveAchievements->GetAchievement( i ) );

			ListView_SetExtendedListViewStyle( hList, LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP );
			SetDlgItemText( hDlg, IDC_RA_BROKENACH_BUGREPORTER, Widen( RAUsers::LocalUser().Username() ).c_str() );
		}
		return false;

	case WM_COMMAND:
		switch( LOWORD( wParam ) )
		{
		case IDOK:
		{
			HWND hList = GetDlgItem( hDlg, IDC_RA_REPORTBROKENACHIEVEMENTSLIST );

			const bool bProblem1Sel = ( IsDlgButtonChecked( hDlg, IDC_RA_PROBLEMTYPE1 ) == BST_CHECKED );
			const bool bProblem2Sel = ( IsDlgButtonChecked( hDlg, IDC_RA_PROBLEMTYPE2 ) == BST_CHECKED );

			if( ( bProblem1Sel == false ) && ( bProblem2Sel == false ) )
			{
				MessageBox( nullptr, L"Please select a problem type.", L"Warning", MB_ICONWARNING );
				return false;
			}

			int nProblemType = bProblem1Sel ? 1 : bProblem2Sel ? 2 : 0;	// 0==?
			const char* sProblemTypeNice = PROBLEM_STR[ nProblemType ];

			char sBuggedIDs[ 1024 ];
			sprintf_s( sBuggedIDs, 1024, "" );

			int nReportCount = 0;

			const size_t nListSize = ListView_GetItemCount( hList );
			for( size_t i = 0; i < nListSize; ++i )
			{
				if( ListView_GetCheckState( hList, i ) != 0 )
				{
					//	NASTY big assumption here...
					char buffer[ 1024 ];
					sprintf_s( buffer, 1024, "%d,", g_pActiveAchievements->GetAchievement( i ).ID() );
					strcat_s( sBuggedIDs, 1024, buffer );

					//ListView_GetItem( hList );	
					nReportCount++;
				}
			}

			if( nReportCount > 5 )
			{
				if( MessageBox( nullptr, L"You have over 5 achievements selected. Is this OK?", L"Warning", MB_YESNO ) == IDNO )
					return false;
			}

			wchar_t sBugReportCommentWide[ 4096 ];
			GetDlgItemText( hDlg, IDC_RA_BROKENACHIEVEMENTREPORTCOMMENT, sBugReportCommentWide, 4096 );
			std::string sBugReportComment = Narrow( sBugReportCommentWide );

			char sBugReportInFull[ 8192 ];
			sprintf_s( sBugReportInFull, 8192,
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
					CoreAchievements->GameTitle().c_str(),
					sBuggedIDs,
					sProblemTypeNice,
					RAUsers::LocalUser().Username().c_str(),
					g_sCurrentROMMD5.c_str(),
					sBugReportComment.c_str() );

			if( MessageBox( nullptr, Widen( sBugReportInFull ).c_str(), L"Summary", MB_YESNO ) == IDNO )
				return false;

			PostArgs args;
			args[ 'u' ] = RAUsers::LocalUser().Username();
			args[ 't' ] = RAUsers::LocalUser().Token();
			args[ 'i' ] = sBuggedIDs;
			args[ 'p' ] = std::to_string( nProblemType );
			args[ 'n' ] = sBugReportComment.c_str();
			args[ 'm' ] = g_sCurrentROMMD5;

			Document doc;
			if( RAWeb::DoBlockingRequest( RequestSubmitTicket, args, doc ) )
			{
				if( doc[ "Success" ].GetBool() )
				{
					char buffer[ 2048 ];
					sprintf_s( buffer, 2048, "Submitted OK!\n"
							"\n"
							"Thankyou for reporting that bug(s), and sorry it hasn't worked correctly.\n"
							"\n"
							"The development team will investigate this bug as soon as possible\n"
							"and we will send you a message on RetroAchievements.org\n"
							"as soon as we have a solution.\n"
							"\n"
							"Thanks again!" );

					MessageBox( hDlg, Widen( buffer ).c_str(), L"Success!", MB_OK );
					EndDialog( hDlg, true );
					return true;
				}
				else
				{
					char buffer[ 2048 ];
					sprintf_s( buffer, 2048,
							"Failed!\n"
							"\n"
							"Response From Server:\n"
							"\n"
							"Error code: %d", doc.GetParseError() );
					MessageBox( hDlg, Widen( buffer ).c_str(), L"Error from server!", MB_OK );
					return false;
				}
			}
			else
			{
				MessageBox( hDlg,
							L"Failed!\n"
							L"\n"
							L"Cannot reach server... are you online?\n"
							L"\n",
							L"Error!", MB_OK );
				return false;
			}
		}
		break;

		case IDCANCEL:
			EndDialog( hDlg, true );
			return true;
		}
		return false;

	case WM_CLOSE:
		EndDialog( hDlg, false );
		return true;

	default:
		return false;
	}
}

//static
void Dlg_AchievementsReporter::DoModalDialog( HINSTANCE hInst, HWND hParent )
{
	if( g_pActiveAchievements->NumAchievements() == 0 )
		MessageBox( hParent, L"No ROM loaded!", L"Error", MB_OK );
	else
		DialogBox( hInst, MAKEINTRESOURCE( IDD_RA_REPORTBROKENACHIEVEMENTS ), hParent, AchievementsReporterProc );
}