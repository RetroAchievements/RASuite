#include "RA_Implementation.h"
#include "RA_Interface.h"
#include <windows.h>


//#include "message.h"

void Machine_UnPause(); //machine.c
HWND ConsoleHWND(void); //message.c

// returns -1 if not found
int GetMenuItemIndex(HMENU hMenu, const char* ItemName)
{
	int index = 0;
	char buf[256];

	while (index < GetMenuItemCount(hMenu))
	{
		if (GetMenuString(hMenu, index, buf, sizeof(buf) - 1, MF_BYPOSITION))
		{
			if (!strcmp(ItemName, buf))
				return index;
		}
		index++;
	}
	return -1;
}


//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
bool GameIsActive()
{
	//return (g_machine_flags ^ MACHINE_ROM_LOADED)== MACHINE_ROM_LOADED; // doesn't detect system
	return true;
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	Machine_UnPause();
	//Machine_Pause(); Don't do this. Running pause code from within Machin_Pause() now.
	//FCEUI_SetEmulationPaused(false);
}

//	Perform whatever function in the case of needing to rebuild the menu.
void RebuildMenu()
{
	//Meka uses Allegro window (Not Win32)
	//So going to use the Console window from message.c to attach the RA Menu to because using SetMenu distorts the window
	//We actually need to create the entire menu in the first place to do this (Console doesn't have one)
	
	HMENU MainMenu = GetMenu(ConsoleHWND());
	if (!MainMenu) return;
	
	// get file menu index
	int index = GetMenuItemIndex(MainMenu, "&RetroAchievements");
	if (index >= 0)
		DeleteMenu(MainMenu, index, MF_BYPOSITION);

	//	##RA embed RA
	AppendMenu(MainMenu, MF_POPUP | MF_STRING, (UINT_PTR)RA_CreatePopupMenu(), TEXT("&RetroAchievements"));

	InvalidateRect(ConsoleHWND(), NULL, TRUE);

	DrawMenuBar(ConsoleHWND());

/*
	AppendMenu(AboutMenu, MF_STRING, 100001, "&About Dud");
	AppendMenu(MainMenu, MF_STRING | MF_POPUP, (UINT)AboutMenu, "&About");
	SetMenu(ConsoleHWND(), MainMenu);

	SetWindowText(ConsoleHWND(), "Testing Meka RA Build Really");

	//disable close window for console because I am lazy
	EnableMenuItem(GetSystemMenu(ConsoleHWND(), FALSE), SC_CLOSE,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	DrawMenuBar(ConsoleHWND());

	*/
	//HMENU hMainMenu = GetMenu(hAppWnd); HMENU hMainMenu = GetMenu(hAppWnd);

	// get main menu handle
	/*
	HMENU hMainMenu = GetMenu(hAppWnd);
	if (!hMainMenu) return;

	// get file menu index
	int index = GetMenuItemIndex(hMainMenu, "&RetroAchievements");
	if (index >= 0)
		DeleteMenu(hMainMenu, index, MF_BYPOSITION);

	index = GetMenuItemIndex(hMainMenu, "&Debug");
	if (index >= 0)
		DeleteMenu(hMainMenu, index, MF_BYPOSITION);

	//	##RA embed RA
	AppendMenu(hMainMenu, MF_POPUP | MF_STRING, (UINT_PTR)RA_CreatePopupMenu(), TEXT("&RetroAchievements"));

	DrawMenuBar(hAppWnd);
	*/
}

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
void GetEstimatedGameTitle(char* sNameOut)
{
	//if( emu && emu->get_NES_ROM() )
	//	strcpy_s( sNameOut, 49, emu->get_NES_ROM()->GetRomName() );
}

void ResetEmulation()
{
	//FCEUI_ResetNES();
}

void LoadROM(char* sFullPath)
{
	//FCEUI_LoadGame(sFullPath, 0);
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
	RA_InstallSharedFunctions(&GameIsActive, &CauseUnpause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM);
}

