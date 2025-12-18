# AB-Mod

A mod for ANTONBLAST, made possible thanks to Aurie Framework and YYToolkit.

Currently features the following:
* Access to the GameMaker debug overlay
* Some basic debug functionality (such as debug collision visibility)
* Custom room loading functionality (WIP)

There are a few keys that have been added that do some sort of action:
* F1 - Dump global variable names to the console
* F2 - Dump player variable names to the console
* F3 - Load custom room from testrm.json
* F4 - Toggle debug collision visibility
* F5 - Change current page in menu

The mod in its current state is not finished and is still a work in progress.  
Room loading functionality will have more details when it is in a more usable form.

## Install

The installation process is a bit complicated, but there's nothing I can do about that, especially since this is all relying on, at the time of writing this, still WIP tools.

* Create the following directory structure as seen below where ANTONBLAST is installed:
```
├─── ANTONBLAST.exe
└─── mods
    ├─── Aurie
    └─── Native
```
* Download AurieCore.dll and AuriePatcher.exe from [here](https://github.com/AurieFramework/Aurie/releases/tag/v2.0.0b)  
Make sure you download these files from the "Aurie v2" release!
* Download YYToolkit.dll from [here](https://github.com/AurieFramework/YYToolkit/releases/tag/v5.0.0a)  
Again, make sure it's from the "v5 open-beta, build A" release.
* Copy AurieCore.dll into the `mods/Native` folder
* Copy YYToolkit.dll into the `mods/Aurie` folder
* Patch the game exe using AuriePatcher  
You can do it either from the command line or just launch the patcher and it'll show some dialog boxes that do the same thing.
* Download AB-Mod.dll from the releases page [here](https://github.com/basiccube/AB-Mod/releases)
* Copy AB-Mod.dll into the `mods/Aurie` folder
* You should be ready to launch the game now!

If everything was done correctly, you should see the debug overlay when the game opens.

## Build

Building requires Visual Studio 2026 with the "Desktop development with C++" workload in the Visual Studio Installer selected.  
Open the solution and then build it as usual.

## Credits

This wouldn't be possible without the following:
* [Aurie Framework](https://github.com/AurieFramework/Aurie)
* [YYToolkit](https://github.com/AurieFramework/YYToolkit)

Additionally, this project uses the following libraries/projects:
* [JSON for Modern C++](https://github.com/nlohmann/json)