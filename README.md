RASuite
=======

All emulators used for interfacing with RetroAchievements.org


Instructions on using the packages.config file
----------------------------------------------
Automatic package restore doesn't seem to work for non .NET/Core projects so you'll need to go to the root directory and type the following.

NuGet restore %solution_file%

You normally don't need to put the solution name but there's more than one solution.
