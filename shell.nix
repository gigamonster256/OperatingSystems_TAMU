{
  pkgs,
  selfPkgs,
}: {
  default = pkgs.mkShellNoCC {
    nativeBuildInputs = [
      selfPkgs.i386-elf-gcc
      selfPkgs.i386-elf-binutils
      pkgs.nasm
    ];
  };
}
