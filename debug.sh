mkdir debug
cd debug
cmake ..
cmake --build .
cp libnexus_keyboard_overlay.dll "/mnt/f/Guild Wars 2/addons/keyboard_overlay.dll"
chmod -x "/mnt/f/Guild Wars 2/addons/keyboard_overlay.dll"
echo "Finished!"
