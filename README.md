RASuite
=======

All emulators used for interfacing with RetroAchievements.org


<h2>Instructions on using the packages.config file</h2>
<p>Before you try this, first try right clicking the solution in solution explorer and click restore nuget packages</p>
<p>Automatic package restore doesn't work for native projects so you'll need to go to the root directory and type the following.</p>
<ul>
	<li>NuGet restore %solution_file%</li>
	<li>You normally don't need to put the solution name but there's more than one solution.</li>
</ul>


<h2>Compiling Snes9x w/o all the Junk </h2>
<p><i>Note: this is for Snes9x 1.54.1 by itself, I didn't integrate it with RA yet.</i></p>
<em>Also these instructions only work for a Win32 build, any paths, etc need to use x86 or Win32 folders <strong>NOT</strong>x64</em>
<p>
	You also need Windows 7.1a SDK, if you don't have get chocolatey from chocolatey.org<br />
	After it's installed, as an admin type in cinst windows-sdk-7.1 in cmd or powershell and it'll install in the background. Read directly below this for a workaround if it fails.<br />
</p>

<p><strong>If you have an AMD Archetecture:</strong></p>
<ul>
	<li>Download this specfic <a href="http://download.microsoft.com/download/F/1/0/F10113F5-B750-4969-A255-274341AC6BCE/GRMSDKX_EN_DVD.iso">ISO</a></li>
	<li>Mount the image</li>
	<li>Load SDKSetup.exe directly</li>
</ul>


<p>The DirectX SDK that comes with the project doesn't have everything (just what was available on RASuite), I suggest getting the latest one since it's backwards compatible. You will need this, but you also need to copy and paste the ddraw files because they've been removed from the most recent SDK.</p>
<ul>
	<li>Get it from <a href="https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe" >Microsoft</a></li>
	<li>If you have any problems with it, uninstall VC++ 2010</li>
</ul>


<p>
	I'm assuming you followed the instructions above.<br />
	You'll need 3 enviornment variables, do the following in cmd:
</p>
<em>Note: You don't need the exact pathnames, you can just copy and paste the files in the appropriate directories specified by the project Property Pages</em>
<ol>
	<li>setx ZLIB_DIR %Path_to_dir%\packages\zlib.v140.windesktop.msvcstl.dyn.rt-dyn.1.2.8.8\build\native</li>
	<li>setx CG_INC_PATH %Path_to_dir%\packages\NVIDIA_Cg_x86.3.1.13\NVIDIA_Cg_x86</li>
	<li>setx CG_LIB_PATH %CG_INC_PATH%</li>
</ol>
<p>You could use the include/lib folders for the full SDK for %DXSDK_DIR% but I don't guarantee it'll work</p>

<h3>Library Directories</h3>
<ul>
	<li>$(DXSDK_DIR)Lib\x86;</li>
	<li>$(CG_LIB_PATH);</li>
	<li>$(LibraryPath);</li>
</ul>

<h3>Include Directories</h3>
<ul>
	<li>$(DXSDK_DIR);</li>
	<li>$(DXSDK_DIR)include;</li>
	<li>$(ZLIB_DIR)include;</li>
	<li>$(CG_INC_PATH);</li>
	<li>$(IncludePath)</li>
<ul>

Build and Run, for now I decided not have fmod audio support since it's taking too long to download
