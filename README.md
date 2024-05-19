# nexus_keyboard_overlay

A simple keyboard overlay to show keypresses, specifically made for JP speedrunning.

[[JP] Community Discord](https://gw2jp.net/discord)

## Features

Youtube video by Delta showcasing the addon [here](https://youtu.be/LspSSRQHLpE)  

- Handles normal keys, NUMPAD and mouse keys. ⚠️ shifted keys are not handled (though shift can be added as a seperate key) ⚠️
- Add and place keys where you want!
- Your layout is saved and loaded on launch
- Toggle-able transparency (opacity is set to 80% by default so you can almost see what's going on under it)

## Usage

Download & install [Nexus](https://raidcore.gg/Nexus) and run Guild Wars 2 once to set up Nexus.  
Download the addon from the [Releases](https://github.com/Seres67/nexus_keyboard_overlay/releases/latest), and put it in the GW2/addons directory.  
You should be able to hot-load it (so, no need to restart the game), but if it doesn't work just restart the game.  
Configure it by pressing the Nexus icon -> Options -> Addons -> Keyboard Overlay

## Planned features

I have a list of planned features [here](https://github.com/Seres67/nexus_keyboard_overlay/blob/main/todo.norg)

## Build it yourself

### Linux

Dependencies:
- x86_64-w64-mingw32
- cmake

```bash
git clone https://github.com/Seres67/nexus_keyboard_overlay
cd nexus_keyboard_overlay
mkdir build
cd build
cmake ..
cmake --build .
```
Copy libnexus_keyboard_overlay.dll to your GW2/addons folder and rename it however your like (e.g. keyboard_overlay.dll)


### Windows

Dependencies:
- Visual Studio / Visual Studio Build Tools

```bash
git clone https://github.com/Seres67/nexus_keyboard_overlay
cd nexus_keyboard_overlay
mkdir build
cd build
cmake ..
cmake --build .
```
Copy `Debug/nexus_keyboard_overlay.dll` to your GW2/addons folder and rename it however your like (e.g. keyboard_overlay.dll)