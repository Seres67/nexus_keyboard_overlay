{
  buildPackages,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "nexus_keyboard_overlay";
  version = "0.9.0.0";
  src = ./.;

  nativeBuildInputs = [
    buildPackages.stdenv.cc
    buildPackages.cmake
  ];

  # buildInputs = [
  #   stdenv.cc
  #   pkgs.curl
  #   pkgs.nlohmann_json
  # ];

  # patchPhase = ''
  #   mkdir -p ./modules/
  #   cp --no-preserve=mode -r ${imgui} ./modules/imgui
  #   cp --no-preserve=mode -r ${nexus} ./modules/nexus
  #   cp --no-preserve=mode -r ${mumble} ./modules/mumble
  # '';

  # installPhase = ''
  #   mkdir -p $out/lib
  #   cp ./*.dll $out/lib
  # '';

  # postBuild = ''
  #   mkdir -p $out/share
  #   cp ./compile_commands.json $out/share/
  # '';
}
