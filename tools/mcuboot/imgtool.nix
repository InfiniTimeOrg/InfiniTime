#
# Nix environment for imgtool
#
# To install the environment
#
# $ nix-env --file imgtool.nix --install env-imgtool
#
# To load the environment
#
# $ load-env-imgtool
#
with import <nixpkgs> {};
let
  # Nixpkgs has fairly recent versions of the dependencies, so we can
  # rely on them without having to build our own derivations.
  imgtoolPythonEnv = python37.withPackages (
    _: [
      python37.pkgs.click
      python37.pkgs.cryptography
      python37.pkgs.intelhex
      python37.pkgs.setuptools
      python37.pkgs.cbor
    ]
  );
in
myEnvFun {
  name = "imgtool";

  buildInputs = [ imgtoolPythonEnv ];
}
