//
//	Overloadable functions that should be implemented on an emulator-specific level.
//

//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
extern bool GameIsActive();

//	Perform whatever action is required to unpause emulation.
extern void CauseUnpause();

//	Perform whatever function in the case of needing to rebuild the menu.
extern void RebuildMenu();

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
extern void GetEstimatedGameTitle( char* sNameOut );


extern void ResetEmulation();

//	Called BY the toolset to tell the emulator to load a particular ROM.
extern void LoadROMFromEmu( char* sFullPath );

//	Installs these shared functions into the DLL
extern void RA_InitShared();