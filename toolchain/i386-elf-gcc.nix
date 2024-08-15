# inspired by https://github.com/nativeos/i386-elf-toolchain
{
  stdenv,
  fetchurl,
  gmp,
  mpfr,
  libmpc,
  binutils,
}: let
  version = "13.2.0";
  url = "http://ftpmirror.gnu.org/gcc/gcc-${version}/gcc-${version}.tar.gz";
in
  stdenv.mkDerivation {
    inherit version;
    pname = "i386-elf-gcc";
    src = fetchurl {
      inherit url;
      sha256 = "sha256-jLS+N5ZlGXb5S5NW+gjYM1JPYkINYpLFAzqaJq8xUHg=";
    };

    buildInputs = [
      gmp
      mpfr
      libmpc
    ];

    propagatedBuildInputs = [
      binutils
    ];

    hardeningDisable = ["format"]; # -Werror=format-security

    configurePhase = ''
      runHook preConfigure

      mkdir build
      pushd build

      ../configure \
        --prefix=$out \
        --target=i386-elf \
        --disable-nls \
        --without-headers \
        --enable-languages=c,c++ \
        --with-gmp-include=${gmp.dev}/include \
        --with-gmp-lib=${gmp.out}/lib \
        --with-mpfr-include=${mpfr.dev}/include \
        --with-mpfr-lib=${mpfr.out}/lib \
        --with-mpc=${libmpc} \
        --with-as=${binutils}/bin/i386-elf-as \
        --with-ld=${binutils}/bin/i386-elf-ld

      popd

      runHook postConfigure
    '';

    buildPhase = ''
      runHook preBuild

      pushd build
      make all-gcc
      make all-target-libgcc
      popd

      runHook postBuild
    '';

    installPhase = ''
      runHook preInstall

      pushd build
      make install-gcc
      make install-target-libgcc
      popd

      runHook postInstall
    '';
  }
