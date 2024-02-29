mkdir debug
cd debug
cmake ..
cmake --build .
cp libnexus_keyboard_overlay.dll ~/Games/guild-wars-2/drive_c/Program\ Files/Guild\ Wars\ 2/addons/keyboard_overlay.dll
chmod -x ~/Games/guild-wars-2/drive_c/Program\ Files/Guild\ Wars\ 2/addons/keyboard_overlay.dll
echo "Finished!"
