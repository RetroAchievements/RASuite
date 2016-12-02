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


<h2>Compiling RASnes9x </h2>
<em>Also these instructions only work for a Win32 build, any paths, etc need to use x86 or Win32 folders <strong>NOT</strong>x64</em>

<h3>Requirements</h3>
<ol>
	<li>Windows SDK 7.1</li>
	<li>DirectX SDK (June 2010)</li>
	<li>NVIDIA Cg Toolkit</li>
</ol>

Below is instructions if you're having trouble getting Windows SDK 7.1A installed, the first option on Microsoft's website supports almost all archetectures.
<p><strong>If you have an AMD Archetecture:</strong></p>
<ul>
	<li>Download this specfic <a href="http://download.microsoft.com/download/F/1/0/F10113F5-B750-4969-A255-274341AC6BCE/GRMSDKX_EN_DVD.iso">ISO</a></li>
	<li>Mount the image</li>
	<li>Load SDKSetup.exe directly</li>
</ul>


<p>This needs the "latest" legacy DirectX SDK</p>
<ul>
	<li>Get it from <a href="https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe" >Microsoft</a></li>
	<li>If you have any problems with it, uninstall VC++ 2010</li>
	<li>Drag the DXSDK_Missing contents from the packages folder into the installation folder</li>
</ul>

All Settings have been configured for you, there could be some issues working on a different branch but shouldn't get in the way of testing.

Build and Run, for now I decided not have fmod audio support since it's taking too long to download.
