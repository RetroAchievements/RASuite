RASuite
=======

All emulators used for interfacing with RetroAchievements.org


Instructions on using the packages.config file
----------------------------------------------
Automatic package restore doesn't seem to work for non .NET/Core projects so you'll need to go to the root directory and type the following.

NuGet restore %solution_file%

You normally don't need to put the solution name but there's more than one solution.


Compiling Snes9x w/o all the Junk 
---------------------------------
Note: this is for Snes9x 1.54.1 by itself, I didn't integrate it with RA yet.

You also need Windows 7.1a SDK, if you don't have get chocolatey from chocolatey.org
After it's installed, as an admin type in cinst windows-sdk-7.1 in cmd or powershell and it'll install in the background.

I'm assuming you followed the instructions above.
You'll need 4 enviornment variables, do the following in cmd:

1) setx DXSDK_DIR %Path_to_dir%\packages\DXSDK_March09_x86.9.26.1590\DXSDK_March09_x86
2) setx ZLIB_DIR %Path_to_dir%\packages\zlib.v140.windesktop.msvcstl.dyn.rt-dyn.1.2.8.8\build\native
3) setx CG_INC_PATH %Path_to_dir%\packages\NVIDIA_Cg_x86.3.1.13\NVIDIA_Cg_x86
4) setx CG_LIB_PATH %CG_INC_PATH%

If not done already put this in the include directories in VC++
$(DXSDK_DIR);$(DXSDK_DIR)\include;$(ZLIB_DIR)\include;$(CG_INC_PATH)\;$(IncludePath)

Also for libraries
$(DXSDK_DIR)\Lib\x86;$(CG_LIB_PATH);$(LibraryPath)

Build and Run, for now I decided not have fmod audio support since it's taking too long to download