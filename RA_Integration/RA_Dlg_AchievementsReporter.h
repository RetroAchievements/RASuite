#pragma once
#include "RA_Defs.h"

class Achievement;

// NB: constant expressions should be constant expressions (obvious but not obvious enough).
// TODO: Discuss whether the constants should be global and non-static, or members and static.

class Dlg_AchievementsReporter
{
public:

	// size_t is used here instead of U because it's more portable, plus it slaps on a U anyway

	// Examples like this are why primitives are expensive to pass by
	// reference because they refer to themselves, in other-words their
	// address and value are in the same location, so it doesn't make
	// sense to pass it by reference.
	static constexpr auto MAX_ACHIEVEMENTS{size_t{200}};
	static constexpr auto MAX_TEXT_LEN{size_t{250}};

public:
	enum ReporterColumns
	{
		Checked,

		//ID,
		Title,
		Desc,
		Author,
		Achieved,

		NumReporterColumns
	};

public:

	// NB: Window Procedures and DoModal should not be static, if you
	//     need a hook (like a message router) make that static. If you
	//     know what MFC is, WindowProc and DoModal are virtual
	//     functions because they are not static in the WinAPI because
	//     it could cause deadlocks because threads need to be
	//     destroyed and renewed but static prevents that. MFC is just
	//     a C++ interface for the WinAPI but has some symbols that
	//     prevent it from being cross-platform. Plus DoModal
	//     incorrectly implemented here. Those other conditions should
	//     be in another function or the constructor.
	static void DoModalDialog( HINSTANCE hInst, HWND hParent );
	static INT_PTR CALLBACK AchievementsReporterProc( HWND, UINT, WPARAM, LPARAM );

public:

	// If you want these to stay static, there's no problem with it.

	static void SetupColumns( HWND hList );

	// NB: it's safer and cheaper to pass objects by reference, just
	//     use an r-value reference to move if for some odd reason the
	//     address is in another location (move constructors are
	//     implicitly defined in C++)
	static int AddAchievementToListBox( HWND hList, const Achievement* pAch );

public:

	// These shouldn't be static
	static size_t ms_nNumOccupiedRows;
	static char ms_lbxData[MAX_ACHIEVEMENTS][NumReporterColumns][MAX_TEXT_LEN];
};

extern Dlg_AchievementsReporter g_AchievementsReporterDialog;

// Search for warning C6262 for reasons

// Temp workaround for the DialogProc, these are message crackers from
// WindowsX, eventually it'll be more structured. These normally should be in a base class.

// Some breakdown, hwnd is the active window/dialog in the message queue (DialogProc/FIFO).
// id is a command id
// hwndCtl is a control
// codeNotify is a notification code (obviously...)

// This is how the Operating System would have handled them in DefWindowProc

// These currently have prefixes because they aren't in a class
// Message handlers.
void achrep_OnClose( HWND hwnd );
void achrep_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );
BOOL achrep_OnInitDialog( HWND hwnd, HWND hwndFocus, LPARAM lParam );

// Dialog ID stuff
void OnOK();  // NB: this is would be in a base class, just working around.

// TODO: tmp, should put this in RA_Defs
// NB: using != typedef, the semantics are similar but using declares an alias not a type
using stringref = std::string&;

// NB: this function needs to be broken up, or dynamically allocated...
void achrep_OnOK(); // NB: this would be an override so I'm just calling it this
void achrep_OnOK_submitmsg( HWND hDlg, stringref buffer );
void achrep_OnOK_servererr( Document& doc, HWND hDlg, stringref buf );
void achrep_OnOK_lastmsg( HWND hDlg );

void OnCancel();
