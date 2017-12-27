#include "RA_Implementation.h"
#include "RA_Interface.h"
#include <windows.h>

//#ifdef ARCH_WIN32
#include "projects/msvc/resource.h"
//#endif

#include "vmachine.h"

//Don't like using externs, but many of the corressponding header files don't agree well with inclusion here
//extern void Machine_UnPause();						//machine.c
//extern void Machine_Pause();						//machine.c
extern HWND ConsoleHWND(void);						//message.c
extern void ConsolePrintf(const char *format, ...);	//message.c

#include "shared.h"
#include "machine.h"
#include "app_memview.h"
#include "app_cheatfinder.h"
#include "debugger.h"

//extern t_memory_viewer *MemoryViewer_MainInstance;
//extern struct t_cheat_finder *g_CheatFinder_MainInstance;
//extern struct t_debugger   Debugger;


//Required for new RA Memory Bank interface
//See: RA_MemManager.h _RAMByteReadFn _RAMByteWriteFn
unsigned char RAMeka_SMS_RAMByteReadFn(unsigned int Offset) {
	return static_cast<unsigned char>(RAM[Offset]);
}
void RAMeka_SMS_RAMByteWriteFn(unsigned int Offset, unsigned int nVal) {
	//why is nVal an int and not a char?
	RAM[Offset] = static_cast<unsigned char>(nVal);
}



//Needed as RA and Meka both use GetCurrentDirectory and SetCurrentDirectory seperately
#define RAMEKA_DIR_PATH_SIZE (2048)
static char RA_rootDir[RAMEKA_DIR_PATH_SIZE];
static char Meka_currDir[RAMEKA_DIR_PATH_SIZE];

void RAMeka_Stash_Meka_CurrentDirectory() {
	GetCurrentDirectory(RAMEKA_DIR_PATH_SIZE, Meka_currDir);
}
void RAMeka_Restore_Meka_CurrentDirectory() {
	SetCurrentDirectory(Meka_currDir);
}
void RAMeka_Stash_RA_RootDirectory() {
	GetCurrentDirectory(RAMEKA_DIR_PATH_SIZE, RA_rootDir);
}
void RAMeka_Restore_RA_RootDirectory() {
	SetCurrentDirectory(RA_rootDir);
}



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

	//	bool check = (g_machine_flags ^ MACHINE_RUN) == MACHINE_RUN) &&
	//			((g_machine_flags & MACHINE_PAUSED ) == MACHINE_PAUSED)	
		//return (g_machine_flags ^ MACHINE_ROM_LOADED)== MACHINE_ROM_LOADED; // doesn't detect system
	return true;  //otherwise debugger won't work.
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	Machine_UnPause();
	//Machine_Pause(); Don't do this. Running pause code from within Machin_Pause() now.
	//FCEUI_SetEmulationPaused(false);
}

//	Perform whatever action is required to Pause emulation.
void CausePause()
{
	Machine_Pause();
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

}

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
void GetEstimatedGameTitle(char* sNameOut)
{
	//if( emu && emu->get_NES_ROM() )
	//	strcpy_s( sNameOut, 49, emu->get_NES_ROM()->GetRomName() );
}


void Machine_Reset(); //machine.c
void ResetEmulation()
{
	Machine_Reset(); // FCEUI_ResetNES();
}

void LoadROM(const char* sFullPath)
{
	//FCEUI_LoadGame(sFullPath, 0);
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
	RA_InstallSharedFunctions(&GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM);
}


//extras
//RAMeka specific helper/refactoring code
//Called from within the emulator

//Handle initial setup of RA integration, including attaching RA Menu to Consolw prompt
void RAMeka_RA_Setup() {

	RAMeka_Stash_Meka_CurrentDirectory();

	RAMeka_MakePlaceholderRAMenu();
	RAMeka_InstallRA(); //after install, CurrentDirectory should be whatever RA uses by default

	//Make a record of RA_rootDir and restore Meka working directory
	RAMeka_Stash_RA_RootDirectory();  
	RAMeka_Restore_Meka_CurrentDirectory();

	ConsolePrintf("%s\n--\n", "RA Init Completed");
}


//Wrapper around RA Call. For neatness.
void RAMeka_RA_InvokeDialog(WPARAM wParam) {

	RA_InvokeDialog(LOWORD(wParam));
}

//Attaches the placeholder RA_Menu to the Meka Win32 startup console window (ConsoleHWND() returns handle to this)
//RA_Integration will use the placeholder to attach its menu items later on
//This is essentially the easierest way for me to do this right now because RA_Integration naturally attaches itself to windows menus, not to Allegro menus.
void RAMeka_MakePlaceholderRAMenu() {

	//See Meka.rc for Console window display parameters.
	//disable close window for console because I am lazy.
	EnableMenuItem(GetSystemMenu(ConsoleHWND(), FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	//Make new placeholder menu for console (We will put the RA menu on the console because this is the easiest way for me right now)
	HMENU MainMenu = CreateMenu();

	//Created placeholder menu called "RetroAchievements". RA_Integration will populate this later.
	HMENU RAMenu = CreatePopupMenu();  
	AppendMenu(RAMenu, MF_POPUP | MF_STRING, 100001, "(RA Not Yet Loaded)");
	AppendMenu(MainMenu, MF_STRING | MF_POPUP, (UINT)RAMenu, "&RetroAchievements");
	SetMenu(ConsoleHWND(), MainMenu);
	InvalidateRect(ConsoleHWND(), NULL, TRUE);
	DrawMenuBar(ConsoleHWND());

}

//Various calls to initialise RA, login, install menus, etc
void RAMeka_InstallRA() {

	RA_Init(ConsoleHWND(), RA_Meka, RAMEKA_VERSION);

	RA_InitShared();
	RA_InitDirectX();
	RA_UpdateAppTitle("RAMEKA");

	RebuildMenu();
	RA_HandleHTTPResults();
	RA_AttemptLogin(true);
	RebuildMenu();

}

//Silently disable Hardcore Mode if any of several Meka dialogs is active.
void RAMeka_ValidateHardcoreMode() {

	//If either the Memory_Viewer (allows editing), or the CheatFinder (is cheating),
	//or the Debugger are active, hardcore mode must be disabled  
	if (MemoryViewer_MainInstance->active || Debugger.active /*|| g_CheatFinder_MainInstance->active*/ ) {

		RA_DisableHardcoreMode();
		//Silently disable Hardcore mode if any  of these are active at startup
		//(Will also need code to disable when they are activated as well)
	}
}

//Performs needed procedures for shutting down achievement system
void RAMeka_RA_Shutdown() {

	RA_HandleHTTPResults();
	RA_Shutdown();
}

//Call to RA made once every frame
void RAMeka_RA_AchievementsFrameCheck() {

	RAMeka_Stash_Meka_CurrentDirectory();

	RA_DoAchievementsFrame();
	RA_HandleHTTPResults();

	RAMeka_Restore_Meka_CurrentDirectory();
}

//wrapper around RA_SetPaused
void RAMeka_RA_SetPaused(bool bIsPaused) {

	if (g_driver->id == DRV_SMS) { // only SMS machine handled for now.
		{
			RAMeka_Stash_Meka_CurrentDirectory();

			RA_SetPaused(bIsPaused);

			RAMeka_Restore_Meka_CurrentDirectory();
		}
	}

}

//Wrapper and directory handling around .rap file load call
void RAMeka_RA_OnSaveStateLoad(char* filename) {

	RAMeka_Stash_Meka_CurrentDirectory();
	RAMeka_Restore_RA_RootDirectory();

	RA_OnLoadState(filename);

	RAMeka_Restore_Meka_CurrentDirectory();

}

//Wrapper and directory handling around .rap file creation call
void RAMeka_RA_OnSaveStateSave(char* filename) {

	RAMeka_Stash_Meka_CurrentDirectory();
	RAMeka_Restore_RA_RootDirectory();

	RA_OnSaveState(filename);

	RAMeka_Restore_Meka_CurrentDirectory();

}

//Perform all operations needed to setup a new (Master System) ROM for RA to monitor
void RAMeka_RA_MountMasterSystemROM() { 
	//Set proper working directory or else RA will leave its log files etc lying all over the filesystem
	RAMeka_Stash_Meka_CurrentDirectory();
	RAMeka_Restore_RA_RootDirectory();

	//Just giving RA the 8k of z80 RAM
	//Meka allocates this in a segment of the RAM variable in a confusing way using the Map_8k_RAM function
	//Have determined that, while in SMS mode, the 8k of z80 ram is contained in the first 8k of RAM (pointed to by Mem_Pages[0] ?)(Am not confident that this ram is being allocated correctly actually)
	//Be wary of this 
	//Note: Instead of starting at 0xc000, master system memory will start at 0x0000
	//See: Machine_Set_Mapping() g_machine.mapper=default? 
	//RA_OnLoadNewRom(ROM, tsms.Size_ROM, RAM, 0x10000, NULL, 0);  //previously gave the whole 64k RAM file

	RA_SetConsoleID(MasterSystem);
	RA_ClearMemoryBanks();
	RA_InstallMemoryBank(0, RAMeka_SMS_RAMByteReadFn, RAMeka_SMS_RAMByteWriteFn, 0x2000); //8KB
	RA_OnLoadNewRom(ROM, tsms.Size_ROM);

	//Old memory model call
	//RA_OnLoadNewRom(ROM, tsms.Size_ROM, RAM, 0x2000, NULL, 0);

	RAMeka_Restore_Meka_CurrentDirectory(); //give back directory state to meka.
}


//Check if hardcore mode is active, and if so, warns that current feature will be disabled
bool RAMeka_HardcoreIsActiveCheck(RAMeka_Softcore_Feature current_feature) {

	const char *disabling_feature_messages[] = {
		"Hardcore Mode active. Disabling Memory Editor",
		"Hardcore Mode active. Disabling Debugger",
		"Hardcore Mode active. Disabling CheatFinder",
		"Hardcore Mode active. Disabling Save/Load states" //not actually used
//		"Hardcore Mode active. Disabling Load states", //not actually used
//		"Hardcore Mode active. Disabling Save states", //not actually used
	};

	if (RA_HardcoreModeIsActive()) {
		const char *warning_message = disabling_feature_messages[current_feature];
		MessageBox(NULL, warning_message, "Warning!", MB_ICONWARNING);
		return true;
	}
	else {
		return false;
	}

}

//If harcore mode is active, request explicit user confirmation before deactivating it
bool RAMeka_HardcoreDeactivateConfirm(RAMeka_Softcore_Feature current_feature) {

	const char *deactivation_confirm_messages[] = {
		"Hardcore mode is active. If you enable the Memory Editor, Hardcore Mode will be disabled. Continue?",
		"Hardcore mode is active. If you enable the Debugger, Hardcore Mode will be disabled. Continue?",
		"Hardcore mode is active. If you enable the Cheat Finder, Hardcore Mode will be disabled. Continue?", //strictly nessesary?
		"Hardcore mode is active. If you Save/Load a state, Hardcore Mode will be disabled. Continue?"
//		"Hardcore mode is active. If you Load a state, Hardcore Mode will be disabled. Continue?",
//		"Hardcore mode is active. If you Save a state, Hardcore Mode will be disabled. Continue?",
	};

	if (!RA_HardcoreModeIsActive()) {
		return true; //no need to ask for confirmation
	}
	else {
		const char *warning_message = deactivation_confirm_messages[current_feature];
		bool user_confirmation_response =
			(MessageBox(NULL, warning_message, "Warning", (MB_YESNO | MB_SETFOREGROUND)) == IDYES);

		if (user_confirmation_response == true) {
			RA_DisableHardcoreMode();
		}
		return user_confirmation_response;
	}

}
