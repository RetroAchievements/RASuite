RASuite
=======

All emulators used for interfacing with RetroAchievements.org


Instructions on using the packages.config file
----------------------------------------------
<p>Before you try this, first try right clicking the solution in solution explorer and click restore nuget packages</p>
<p>Automatic package restore doesn't work for native projects so you'll need to go to the root directory and type the following.</p>
<ul>
	<li>NuGet restore %solution_file%</li>
	<li>You normally don't need to put the solution name but there's more than one solution.</li>
</ul>


Compiling Snes9x w/o all the Junk 
---------------------------------
<p><i>Note: this is for Snes9x 1.54.1 by itself, I didn't integrate it with RA yet.</i></p>

<p>
	You also need Windows 7.1a SDK, if you don't have get chocolatey from chocolatey.org<br />
	After it's installed, as an admin type in cinst windows-sdk-7.1 in cmd or powershell and it'll install in the background.<br />
</p>

<p><strong>If you have an AMD Archetecture:</strong></p>
<ul>
	<li>Download this specfic <a href="http://download.microsoft.com/download/F/1/0/F10113F5-B750-4969-A255-274341AC6BCE/GRMSDKX_EN_DVD.iso">ISO</a></li>
	<li>Mount the image</li>
	<li>Load SDKSetup.exe directly</li>
</ul>


<p>The DirectX SDK that comes with the project doesn't have everything (just what was available on RASuite), I suggest getting the latest one since it's backwards compatible</p>
<ul>
	<li>Get it from <a href="https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe" >Microsoft</a></li>
	<li>If you have any problems with it, uninstall VC++ 2010</li>
</ul>


<p>I'm assuming you followed the instructions above.</p>
You'll need 4 enviornment variables, do the following in cmd:
<ol>
	<li>setx DXSDK_DIR %Path_to_dir%\packages\DXSDK_March09_x86.9.26.1590\DXSDK_March09_x86</li>
	<li>setx ZLIB_DIR %Path_to_dir%\packages\zlib.v140.windesktop.msvcstl.dyn.rt-dyn.1.2.8.8\build\native</li>
	<li>setx CG_INC_PATH %Path_to_dir%\packages\NVIDIA_Cg_x86.3.1.13\NVIDIA_Cg_x86</li>
	<li>setx CG_LIB_PATH %CG_INC_PATH%</li>
</ol>
<p>You could use the include/lib folders for the full SDK for %DXSDK_DIR% but I don't guarantee it'll work</p>

<p>If not done already put this in the include directories in VC++</p>
<ul>
	<li>$(DXSDK_DIR);<br />$(DXSDK_DIR)\include;<br />$(ZLIB_DIR)\include;<br />$(CG_INC_PATH)\;<br />$(IncludePath)</li>
<ul>

<p>Also for libraries</p>
<ul>
	<li>$(DXSDK_DIR)\Lib\x86;<br />$(CG_LIB_PATH);<br />$(LibraryPath)</li>
</ul>

<p>Build and Run, for now I decided not have fmod audio support since it's taking too long to download</p>
