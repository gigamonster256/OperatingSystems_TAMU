{
  description = "A simple groupme discord bridge designed to be deployed on cloudflare workers";

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
    forEachSystem = f: lib.genAttrs systems (system: f pkgsFor.${system});
  in rec {
    formatter = forEachSystem (pkgs: pkgs.alejandra);
    packages = forEachSystem (
      pkgs: import ./toolchain {inherit pkgs;}
    );
    devShells = lib.mapAttrs (system: shell: shell self.packages.${system}) (
      forEachSystem (
        pkgs: selfPkgs: import ./shell.nix {inherit pkgs selfPkgs;}
      )
    );
  };
}
