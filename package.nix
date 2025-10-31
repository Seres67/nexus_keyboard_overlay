{
  cmake,
  stdenv,
  xxd,
}:
stdenv.mkDerivation {
  pname = "nexus_keyboard_overlay";
  version = "0.9.0.2";
  src = ./.;

  nativeBuildInputs = [
    cmake
    xxd
  ];

  installPhase = ''
    mkdir -p $out/lib
    cp ./*.dll $out/lib
    md5sum $out/lib/libnexus_keyboard_overlay.dll | awk '{print $1}' | xxd -r -p > $out/libnexus_keyboard_overlay.dll.md5
  '';
}
