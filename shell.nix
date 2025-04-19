let
  pkgs = import <nixpkgs> { };
  stdenv = pkgs.stdenv;
in
stdenv.mkDerivation {
  name = "env";
  buildInputs = with pkgs; [
    gcc-arm-embedded-13
    gnumake
    qemu
    gdb
    clang-tools
    rustup
    ddd
  ];
  shellHook = ''
    rustup default stable
    rustup target add armv7a-none-eabi
  '';
}
