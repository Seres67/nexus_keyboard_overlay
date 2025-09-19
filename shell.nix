{
  buildPackages,
  mkShell,
}:
mkShell {
  nativeBuildInputs = [
    buildPackages.stdenv.cc
    buildPackages.cmake
  ];
}
