{pkgs ? import <nixpkgs> {}}: rec {
  i386-elf-binutils = pkgs.callPackage ./i386-elf-binutils.nix {};
  i386-elf-gcc = pkgs.callPackage ./i386-elf-gcc.nix {binutils = i386-elf-binutils;};
}
