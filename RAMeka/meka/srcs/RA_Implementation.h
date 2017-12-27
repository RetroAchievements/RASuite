//	RetroAchievements Integration files:
//	Example implementation file: copy .h/.cpp to your emulator and modify functionality.
//	www.retroachievements.org

//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
extern bool GameIsActive();

//	Perform whatever action is required to unpause emulation.
extern void CauseUnpause();
//	Perform whatever action is required to Pause emulation.
extern void CausePause();

//	Perform whatever function in the case of needing to rebuild the menu.
extern void RebuildMenu();

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
extern void GetEstimatedGameTitle(char* sNameOut);

extern void ResetEmulation();

//	Called BY the toolset to tell the emulator to load a particular ROM.
extern void LoadROM(const char* sFullPath);

//	Installs these shared functions into the DLL
extern void RA_InitShared();


//extras

//Most of the other emulators define their version in their MakeBuildVer.bat scripts. 
//But we're using NuGet/Allegro/VS2015 so nothing here is "straightforward"
#define RAMEKA_VERSION  "0.019535"

#include <windows.h> //for WPARAM (kind of wasteful really)

//See: RA_MemManager.h _RAMByteReadFn _RAMByteWriteFn
unsigned char	RAMeka_SMS_RAMByteReadFn (unsigned int Offset);
void			RAMeka_SMS_RAMByteWriteFn(unsigned int Offset, unsigned int nVal);

void RAMeka_Stash_Meka_CurrentDirectory();
void RAMeka_Restore_Meka_CurrentDirectory();
void RAMeka_Stash_RA_RootDirectory();
void RAMeka_Restore_RA_RootDirectory();

void RAMeka_RA_Setup();
void RAMeka_RA_Shutdown();
void RAMeka_RA_InvokeDialog(WPARAM wParam);

void RAMeka_RA_SetPaused(bool bIsPaused);
void RAMeka_RA_OnSaveStateLoad(char* filename);
void RAMeka_RA_OnSaveStateSave(char* filename);

void RAMeka_RA_MountMasterSystemROM();


void RAMeka_MakePlaceholderRAMenu();
void RAMeka_InstallRA();

void RAMeka_ValidateHardcoreMode();

void RAMeka_RA_AchievementsFrameCheck();


enum RAMeka_Softcore_Feature {
	SCF_MEMORY_EDITOR,
	SCF_DEBUGGER,
	SCF_CHEAT_FINDER,
	SCF_SAVE_LOAD,
//	SCF_LOAD,
//	SCF_SAVE,
	SCF_UNKNOWN
};

bool RAMeka_HardcoreIsActiveCheck(RAMeka_Softcore_Feature current_feature);
bool RAMeka_HardcoreDeactivateConfirm(RAMeka_Softcore_Feature current_feature);
