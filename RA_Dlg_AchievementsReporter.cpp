#include "RA_Dlg_AchievementsReporter.h"

#include "RA_Defs.h"
#include "RA_Core.h"
#include "RA_Resource.h"
#include "RA_User.h"
#include "RA_Achievement.h"
#include "RA_AchievementSet.h"
#include "RA_httpthread.h"


namespace
{
	const char* g_sColTitles[] = { "", "Title", "Description", "Author", "Achieved?" };
	const int g_nColSizes[] = { 19, 105, 205, 75, 62 };

	const size_t NUM_COLS = sizeof( g_sColTitles ) / sizeof( g_sColTitles[0] );
}

int Dlg_AchievementsReporter::m_nNumOccupiedRows = 0;
char Dlg_AchievementsReporter::m_lbxData[MAX_ACHIEVEMENTS][COL_NUMCOLS][MAX_TEXT_SIZE];

Dlg_AchievementsReporter g_AchievementsReporterDialog;

void Dlg_AchievementsReporter::SetupColumns( HWND hList )
{
	//	Remove all columns,
	while( ListView_DeleteColumn( hList, 0 ) ) {}

	//	Remove all data.
	ListView_DeleteAllItems( hList );

	char buffer[256];
	
	LV_COLUMN col;
	ZeroMemory( &col, sizeof( col ) );

	for( size_t i = 0; i < NUM_COLS; ++i )
	{
		col.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM|LVCF_FMT;
		col.cx = g_nColSizes[i];
		col.cchTextMax = 255;
		sprintf_s( buffer, 256, g_sColTitles[i] );
		col.pszText = buffer;
		col.iSubItem = i;

		col.fmt = LVCFMT_LEFT|LVCFMT_FIXED_WIDTH;
		if( i == NUM_COLS-1 )
			col.fmt |= LVCFMT_FILL; 

		ListView_InsertColumn( hList, i, (LPARAM)&col );
	}

	//ZeroMemory( &m_lbxData, sizeof(m_lbxData) );

	m_nNumOccupiedRows = 0;

	//BOOL bSuccess = ListView_SetExtendedListViewStyle( hList, LVS_EX_FULLROWSELECT );
	//bSuccess = ListView_EnableGroupView( hList, FALSE );
}

int Dlg_AchievementsReporter::AddAchievementToListBox( HWND hList, const Achievement* pAch )
{
	for( size_t i = 0; i < COL_NUMCOLS; ++i )
	{
		switch( i )
		{
			case COL_CHECKED:
				sprintf_s( m_lbxData[m_nNumOccupiedRows][i], MAX_TEXT_SIZE, "" );
	/*			break;
			case COL_ID:
				sprintf_s( m_lbxData[m_nNumOccupiedRows][i], MAX_TEXT_SIZE, "%d", pAch->ID() );
				break;*/
			case COL_TITLE:
				sprintf_s( m_lbxData[m_nNumOccupiedRows][i], MAX_TEXT_SIZE, pAch->Title().c_str() );
				break;
			case COL_DESC:
				sprintf_s( m_lbxData[m_nNumOccupiedRows][i], MAX_TEXT_SIZE, pAch->Description().c_str() );
				break;
			case COL_AUTHOR:
				sprintf_s( m_lbxData[m_nNumOccupiedRows][i], MAX_TEXT_SIZE, pAch->Author().c_str() );
				break;
			case COL_ACHIEVED:
				sprintf_s( m_lbxData[m_nNumOccupiedRows][i], MAX_TEXT_SIZE, !pAch->Active() ? "Yes" : "No" );
				break;
			default:
				assert( !"Unknown col!" );
				break;
		}
	}

	LV_ITEM item;
	ZeroMemory( &item, sizeof( item ) );

	item.mask = LVIF_TEXT;
	item.cchTextMax = 256;
	item.iItem = m_nNumOccupiedRows;

	item.iSubItem = 0;
	item.pszText = m_lbxData[m_nNumOccupiedRows][0];
	item.iItem = ListView_InsertItem( hList, &item );

	for( size_t i = 1; i < NUM_COLS; ++i )
	{
		item.iSubItem++;
		item.pszText = m_lbxData[m_nNumOccupiedRows][i];	
		ListView_SetItem( hList, &item );
	}

	assert( item.iItem == m_nNumOccupiedRows );

	m_nNumOccupiedRows++;	//	Last thing to do!
	return item.iItem;
}

INT_PTR CALLBACK AchievementsReporterProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
 	switch( uMsg )
 	{
 	case WM_INITDIALOG:
	{
		HWND hList = GetDlgItem( hDlg, IDC_RA_REPORTBROKENACHIEVEMENTSLIST );	
		Dlg_AchievementsReporter::SetupColumns( hList );

		for( size_t i = 0; i < g_pActiveAchievements->NumAchievements(); ++i )
		{
			Dlg_AchievementsReporter::AddAchievementToListBox( hList, &g_pActiveAchievements->GetAchievement( i ) );
		}

		ListView_SetExtendedListViewStyle( hList, LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP );
		//ListView_SetExtendedListViewStyle( hList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT );

		SetDlgItemText( hDlg, IDC_RA_BROKENACH_BUGREPORTER, RAUsers::LocalUser.Username().c_str() );

		return FALSE;
		//return DefWindowProc( hDlg, uMsg, wParam, lParam );
	}
	break;
 
 	case WM_COMMAND:
 		switch( LOWORD(wParam) )
 		{
 		case IDOK:
 			{
				HWND hList = GetDlgItem( hDlg, IDC_RA_REPORTBROKENACHIEVEMENTSLIST );	
			
				const bool bProblem1 = IsDlgButtonChecked( hDlg, IDC_RA_PROBLEMTYPE1 ) != 0;
				const bool bProblem2 = IsDlgButtonChecked( hDlg, IDC_RA_PROBLEMTYPE2 ) != 0;
				
				if( ( bProblem1 == false ) && ( bProblem2 == false ) )
				{
					MessageBox( NULL, "Please select a problem type.", "Warning", MB_ICONWARNING );
					return FALSE;
				}
				
				const char* sProblemStr[] = { "1", "2" };
				const char* sProblemType = bProblem1 ? sProblemStr[0] : sProblemStr[1];

				const char* sProblemStrNice[] = { "Triggers at wrong time", "Didn't trigger at all" };
				const char* sProblemTypeNice = bProblem1 ? sProblemStrNice[0] : sProblemStrNice[1];

				char buffer[1024];

				char sBuggedIDs[1024];
				sprintf_s( sBuggedIDs, 1024, "" );
				
				int nReportCount = 0;

				const size_t nListSize = ListView_GetItemCount( hList );
				for( size_t i = 0; i < nListSize; ++i )
				{
					if( ListView_GetCheckState( hList, i ) != 0 )
					{
						//	NASTY big assumption here...
						sprintf_s( buffer, 1024, "%d,", g_pActiveAchievements->GetAchievement( i ).ID() );
						strcat_s( sBuggedIDs, 1024, buffer );

						//ListView_GetItem( hList );	
						nReportCount++;
					}
				}

				if( nReportCount > 5 )
				{
					if( MessageBox( NULL, "You have over 5 achievements selected. Is this OK?", "Warning", MB_YESNO ) == IDNO )
						return FALSE;
				}

				char sBugReportComment[4096];
				GetDlgItemText( hDlg, IDC_RA_BROKENACHIEVEMENTREPORTCOMMENT, sBugReportComment, 4096 );

				char sBugReportInFull[8192];
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
					CoreAchievements->GameTitle(),
					sBuggedIDs,
					sProblemTypeNice,
					RAUsers::LocalUser.Username(),
					g_sCurrentROMMD5,
					sBugReportComment );

				if( MessageBox( NULL, sBugReportInFull, "Summary", MB_YESNO ) == IDNO )
					return FALSE;
								
				PostArgs args;
				args['u'] = RAUsers::LocalUser.Username();
				args['t'] = RAUsers::LocalUser.Token();
				args['i'] = sBuggedIDs;
				args['p'] = sProblemType;
				args['n'] = sBugReportComment;
				args['m'] = g_sCurrentROMMD5;

				Document doc;
				if( RAWeb::DoBlockingRequest( RequestSubmitTicket, args, doc ) )
				{
					if( doc["Success"].GetBool() )
					{
						sprintf_s( buffer, 1024, "Submitted OK!\n"
							"\n"
							"Thankyou for reporting that bug(s), and sorry it hasn't worked correctly.\n"
							"\n"
							"The development team will investigate this bug as soon as possible\n"
							"and we will send you a message on RetroAchievements.org\n"
							"as soon as we have a solution.\n"
							"\n"
							"Thanks again!" );

						MessageBox( hDlg, buffer, "Success!", MB_OK );
		 				EndDialog( hDlg, TRUE );
 						return TRUE;
					}
					else
					{
						char bufferFeedback[2048];
 						sprintf_s( bufferFeedback, 2048, 
							"Failed!\n"
							"\n"
							"Response From Server:\n"
							"\n"
							"Error code: %d", doc.GetParseError() );
 						MessageBox( hDlg, bufferFeedback, "Error from server!", MB_OK );
						return FALSE;
					}
 				}
 				else
 				{
 					MessageBox( hDlg, 
						"Failed!\n"
						"\n"
						"Cannot reach server... are you online?\n"
						"\n",
						"Error!", MB_OK );
					return FALSE;
 				}
 			}
			break;
 		case IDCANCEL:
 			EndDialog( hDlg, TRUE );
 			return TRUE;
 			break;
		}
 		break;
 	case WM_CLOSE:
 		// 		if (Full_Screen)
 		// 		{
 		// 			while (ShowCursor(true) < 0);
 		// 			while (ShowCursor(false) >= 0);
 		// 		}
 
 		EndDialog( hDlg, FALSE );
 		return TRUE;
 		break;
 	}

	return FALSE;
	//return DefWindowProc( hDlg, uMsg, wParam, lParam );
}

//static
void Dlg_AchievementsReporter::DoModalDialog( HINSTANCE hInst, HWND hParent )
{
	if( g_pActiveAchievements->NumAchievements() == 0 )
	{
		MessageBox( hParent, "No ROM loaded!", "Error", MB_OK );
	}
	else
	{
		DialogBox( hInst, MAKEINTRESOURCE(IDD_RA_REPORTBROKENACHIEVEMENTS), hParent, AchievementsReporterProc );
	}
}