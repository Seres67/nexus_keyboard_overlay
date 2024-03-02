# nexus_keyboard_overlay

A simple keyboard overlay to show keypresses, specifically made for JP speedrunning.

[[JP] Community Discord](https://gw2jp.net/discord)

## Usage

Download & install [Nexus](https://raidcore.gg/Nexus) and run Guild Wars 2 once to setup Nexus.  
Download the addon from the [Releases](https://github.com/Seres67/nexus_keyboard_overlay/releases), and put it in the GW2/addons directory.  
You should be able to hot-load it (so, no need to restart the game), but if it doesn't work just restart the game.  

## Build it yourself

### Linux

Dependencies:
- x86_64-w64-mingw32
- cmake

```bash
git clone https://github.com/Seres67/nexus_keyboard_overlay
cd nexus_keyboard_overlay
cmake ..
cmake --build .

```
Copy libnexus_keyboard_overlay.dll to your GW2/addons folder and rename it however your like (e.g. keyboard_overlay.dll)
