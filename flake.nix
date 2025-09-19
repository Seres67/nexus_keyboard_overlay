{
  description = "A Nexus addon to display a modular keyboard overlay.";
  inputs = {
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
    imgui = {
      url = "github:RaidcoreGG/imgui/master";
      flake = false;
    };
    mumble = {
      url = "github:RaidcoreGG/RCGG-lib-mumble-api/main";
      flake = false;
    };
    nexus = {
      url = "github:RaidcoreGG/RCGG-lib-nexus-api/main";
      flake = false;
    };
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = {
    flake-utils,
    nixpkgs,
    ...
  } @ inputs:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
        crossSystem = {
          config = "x86_64-w64-mingw32";
        };
      };
    in {
      packages.default = pkgs.callPackage ./package.nix {};
      devShells.default = pkgs.callPackage ./shell.nix {};
    });
}
