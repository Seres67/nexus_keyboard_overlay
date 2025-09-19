{
  cmake,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "nexus_keyboard_overlay";
  version = "0.9.0.0";
  src = ./.;

  nativeBuildInputs = [
    cmake
  ];

  installPhase = ''
    mkdir -p $out/lib
    cp ./*.dll $out/lib
  '';
}
