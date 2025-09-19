{
  mkShell,
  windows,
  cmake,
  clang-tools,
}:
mkShell {
  nativeBuildInputs = [
    cmake
    clang-tools
  ];

  buildInputs = [
    windows.mingw_w64
    windows.mingw_w64_headers
  ];
}
