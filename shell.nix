{
  mkShell,
  cmake,
  cmake-language-server,
  clang-tools,
  bintools,
}:
mkShell {
  nativeBuildInputs = [
    cmake
    cmake-language-server
    clang-tools
    bintools
  ];
}
