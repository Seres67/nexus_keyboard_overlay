{
  buildPackages,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "nexus_keyboard_overlay";
  version = "0.9.0.0";
  src = ./.;

  nativeBuildInputs = [
    buildPackages.cmake
    stdenv.cc
  ];

  cmakeFlags = [
    "-DCMAKE_C_COMPILER=${stdenv.cc}/bin/${stdenv.cc.targetPrefix}cc"
    "-DCMAKE_CXX_COMPILER=${stdenv.cc}/bin/${stdenv.cc.targetPrefix}c++"
  ];
}
