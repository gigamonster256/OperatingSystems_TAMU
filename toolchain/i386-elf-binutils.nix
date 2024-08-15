# inspired by https://github.com/nativeos/i386-elf-toolchain
{
  stdenv,
  fetchurl,
  texinfo,
}: let
  version = "2.43";
  url = "http://ftpmirror.gnu.org/binutils/binutils-${version}.tar.gz";
in
  stdenv.mkDerivation {
    inherit version;
    pname = "i386-elf-binutils";
    src = fetchurl {
      inherit url;
      sha256 = "sha256-AlxDbRUEkHbr5RHSllHMR4XuUCllqIOZNqZVGFgr3WQ=";
    };

    nativeBuildInputs = [texinfo];

    configurePhase = ''
      runHook preConfigure

      mkdir build
      pushd build

      ../configure \
        --prefix=$out \
        --target=i386-elf \
        --disable-nls

      popd

      runHook postConfigure
    '';

    buildPhase = ''
      runHook preBuild

      pushd build
      make
      popd

      runHook postBuild
    '';

    installPhase = ''
      runHook preInstall

      pushd build
      make install
      popd

      runHook postInstall
    '';
  }
