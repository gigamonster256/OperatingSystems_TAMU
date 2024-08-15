{pkgs}: {
  default = pkgs.mkShellNoCC {
    nativeBuildInputs = with pkgs; [
      i386-elf-gcc
      i386-elf-binutils
      nasm
      bochs
    ];
  };
}
