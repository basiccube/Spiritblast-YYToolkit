# ![Spiritblast Logo](/assets/spiritblast.png) Spiritblast

A mod for ANTONBLAST (YYC builds), made possible thanks to Aurie Framework and YYToolkit.  
The vm_public branch will not work with this mod!

Currently features the following:
* Access to the GameMaker debug overlay
* Some basic debug functionality (such as debug collision visibility)
* Borderless fullscreen option
* Custom room loading functionality (WIP)
* Ability to add custom code via Lua (WIP)

There are a few keys that have been added that do some sort of action:
* **F1** - Dump global variable names to the console
* **F2** - Dump specified object variable names to the console
* **F3** - Run custom lua code from test.lua
* **F4** - Toggle debug collision visibility
* **F5** - Change current page in menu

To be able to use these keys, you need to first enable them within the new "Debug" page in the options menu.

## Install

The installation process is a bit complicated, and there's nothing I can do about that, especially since this is all relying on, at the time of writing this, still WIP tools.  
Make sure to read the installation steps thoroughly, as not doing any of these steps properly will more than likely make the mod not work!

* **Create the following directory structure as seen below where ANTONBLAST is installed:**
```
├─── ANTONBLAST.exe
└─── mods
    ├─── Aurie
    └─── Native
```
* **Download AurieCore.dll and AuriePatcher.exe from [here](https://github.com/AurieFramework/Aurie/releases/tag/v2.0.0b)**  
Make sure you download these files from the "Aurie v2" release!
* **Download YYToolkit.dll from [here](https://github.com/AurieFramework/YYToolkit/releases/tag/v5.0.0a)**  
Again, make sure it's from the "v5 open-beta, build A" release. The newer v5.0.0b release causes issues with the mod.
* **Copy AurieCore.dll into the `mods/Native` folder**
* **Copy YYToolkit.dll into the `mods/Aurie` folder**
* **Patch ANTONBLAST.exe using AuriePatcher**  
You can do it either from the command line or just launch the patcher and it'll show some dialog boxes that do the same thing.  
If you're using the patcher the non-command line way, it should first show that you have to select the game exe, then afterwards you'd have to select the AurieCore.dll file you put in the `mods/Native` folder.  
Once you've done that, pressing the `Install` button should patch the game.
* **Download Spiritblast.dll from the releases page [here](https://github.com/basiccube/Spiritblast/releases)**
* **Copy Spiritblast.dll into the `mods/Aurie` folder**
* **You should be ready to launch the game now!**

If everything was done correctly, you should see a command prompt window appear when the game launches, and, if you pay close attention, you should also see the text `Mod successfully initialized!` in that window appear somewhere.  
To check that the mod is working properly, open the options menu and you should see the new "Debug" page.  

If you do not see the command prompt window or the new page in the options menu, make sure that you installed the mod properly and check if you did all of the steps correctly.

If the game at any point gets an update, then the exe will almost certainly become unpatched. Simply redo the AuriePatcher step and it should work again.

## Build

Building requires Visual Studio 2026 with the "Desktop development with C++" workload in the Visual Studio Installer selected.  
Open the solution and then build it as usual.

## Credits

This wouldn't be possible without the following:
* [Aurie Framework](https://github.com/AurieFramework/Aurie)
* [YYToolkit](https://github.com/AurieFramework/YYToolkit)

Additionally, this project uses the following libraries/projects:
* [JSON for Modern C++](https://github.com/nlohmann/json)
* [Pluto](https://github.com/PlutoLang/Pluto)
* [LuaCpp](https://github.com/jordanvrtanoski/luacpp)
* [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended)

See the licenses folder for the licenses these other projects use.