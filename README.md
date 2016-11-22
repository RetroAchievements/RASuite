RASuite
=======

All emulators used for interfacing with RetroAchievements.org


Instructions on using the packages.config file
----------------------------------------------

1) Get NuGet.CommandLine 3.4.3

2) You should get a .nuget folder in %USERPROFILE%

3) Add NuGet.exe to the path (optional, easier)

4) In the root directory of the repository (RASuite) type in the following
- NuGet.exe Install -NonInteractive -OutputDirectory %PackageDir% .nuget\packages.config
- PackageDir is where ever you wanted to the unpacked files to be

5) All the files will be unpacked to PackageDir, move them to the appropriate place if source files
- Add a reference to the project (RA_Integration) if it's a .dll
