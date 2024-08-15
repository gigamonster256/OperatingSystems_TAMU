{
  description = "CSCE 611: Operating Systems";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
  };

  outputs = inputs @ {
    self,
    nixpkgs,
    ...
  }: let
    inherit (nixpkgs) lib;
    systems = [
      "x86_64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ];
    pkgsFor = lib.genAttrs systems (
      system: import nixpkgs {inherit system;}
    );
    forEachSystem = f: lib.genAttrs systems (system: f (pkgsFor.${system} // toolchain.${system}));
    toolchain = forEachSystem (pkgs: import ./toolchain {inherit pkgs;});
    MPs = forEachSystem (pkgs: import ./MPs.nix {inherit pkgs;});
  in {
    formatter = forEachSystem (pkgs: pkgs.alejandra);
    packages = lib.recursiveUpdate toolchain MPs;
    devShells = forEachSystem (pkgs: import ./shell.nix {inherit pkgs;});
  };
}
