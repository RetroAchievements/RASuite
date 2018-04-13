#include "RA_Implementation.h"
#include "RA_Interface.h"

//	Include any emulator-side headers, externs or functions here
#include "../Common.h"
#include "movie.h"

// returns -1 if not found
int GetMenuItemIndex(HMENU hMenu, const char* ItemName)
{
  int index = 0;
  char buf[256];

  while(index < GetMenuItemCount(hMenu))
  {
    if(GetMenuString(hMenu, index, buf, sizeof(buf)-1, MF_BYPOSITION))
    {
      if(!strcmp(ItemName, buf))
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
	return true;
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	FCEUI_SetEmulationPaused( false );
}

//	Perform whatever action is required to pause emulation.
void CausePause()
{
	FCEUI_SetEmulationPaused(true);
}

//	Perform whatever function in the case of needing to rebuild the menu.
void RebuildMenu()
{
	// get main menu handle
	HMENU hMainMenu = GetMenu(hAppWnd);
	if(!hMainMenu) return;

	// get file menu index
	int index = GetMenuItemIndex(hMainMenu, "&RetroAchievements");
	if(index >= 0)
		DeleteMenu( hMainMenu, index, MF_BYPOSITION );

	index = GetMenuItemIndex(hMainMenu, "&Debug");
	if(index >= 0)
		DeleteMenu( hMainMenu, index, MF_BYPOSITION );

	//	##RA embed RA
	AppendMenu( hMainMenu, MF_POPUP|MF_STRING, (UINT_PTR)RA_CreatePopupMenu(), TEXT("&RetroAchievements") );

	DrawMenuBar( hAppWnd );
}

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
void GetEstimatedGameTitle( char* sNameOut )
{
	//if( emu && emu->get_NES_ROM() )
	//	strcpy_s( sNameOut, 49, emu->get_NES_ROM()->GetRomName() );
}

void ResetEmulation()
{
	FCEUI_StopMovie();
	FCEUI_ResetNES();
}

void LoadROM( const char* sFullPath )
{
	FCEUI_LoadGame( sFullPath, 0 );
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
	RA_InstallSharedFunctions( &GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM );
}

