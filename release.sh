mkdir release
cd release || return
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
strip libnexus_keyboard_overlay.dll
cp libnexus_keyboard_overlay.dll ~/Games/guild-wars-2/drive_c/Program\ Files/Guild\ Wars\ 2/addons/keyboard_overlay.dll
mv libnexus_keyboard_overlay.dll keyboard_overlay.dll
chmod -x ~/Games/guild-wars-2/drive_c/Program\ Files/Guild\ Wars\ 2/addons/keyboard_overlay.dll
echo "Finished!"
