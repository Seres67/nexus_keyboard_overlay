{
  mkShell,
  windows,
  cmake,
  clang-tools,
  stdenv,
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

  shellHook = ''
    export CPLUS_INCLUDE_PATH="${stdenv.cc.cc}/include/c++/${stdenv.cc.cc.version}";
  '';
}
