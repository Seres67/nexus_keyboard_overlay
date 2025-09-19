{
  buildPackages,
  mkShell,
  windows,
}:
mkShell {
  nativeBuildInputs = [
    buildPackages.stdenv.cc
    buildPackages.cmake
  ];

  buildInputs = [
    windows.mingw_w64_headers
  ];
}
