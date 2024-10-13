{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-compat.url = "https://flakehub.com/f/edolstra/flake-compat/1.tar.gz";
  };

  outputs = { self, ... }@inputs:
    let
      forAllSystems = function:
        inputs.nixpkgs.lib.genAttrs [
          "x86_64-linux"
          "aarch64-linux"
        ]
          (system: function (import inputs.nixpkgs {
            inherit system;
            config.allowUnfree = true;
          }));
    in
    {
      packages = forAllSystems (pkgs:
        let
          infinitime-nrf5-sdk = pkgs.nrf5-sdk.overrideAttrs (old: {
            version = "15.3.0";
            src = pkgs.fetchzip {
              url = "https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5sdk153059ac345.zip";
              sha256 = "sha256-pfmhbpgVv5x2ju489XcivguwpnofHbgVA7bFUJRTj08=";
            };
          });
        in
        with pkgs; {
          default = stdenv.mkDerivation rec {
            name = "infinitime";
            version = "1.14.1";

            src = fetchFromGitHub {
              owner = "InfiniTimeOrg";
              repo = "InfiniTime";
              rev = "refs/tags/${version}";
              hash = "sha256-IrsN+9LgEjgfoRR6H7FhsdLMK+GLxc41IBnSbdpwv/E=";
              fetchSubmodules = true;
            };

            nativeBuildInputs = [
              adafruit-nrfutil
              nodePackages.lv_font_conv
              patch
              python3
              python3.pkgs.cbor
              python3.pkgs.click
              python3.pkgs.cryptography
              python3.pkgs.intelhex
              python3.pkgs.pillow
            ];

            postPatch = ''
              # /usr/bin/env is not available in the build sandbox
              substituteInPlace src/displayapp/fonts/generate.py --replace "'/usr/bin/env', 'patch'" "'patch'"
              substituteInPlace tools/mcuboot/imgtool.py --replace "/usr/bin/env python3" "${python3}/bin/python3"
            '';

            cmakeFlags = [
              ''-DARM_NONE_EABI_TOOLCHAIN_PATH=${gcc-arm-embedded-10}''
              ''-DNRF5_SDK_PATH=${infinitime-nrf5-sdk}/share/nRF5_SDK''
              ''-DBUILD_DFU=1''
              ''-DBUILD_RESOURCES=1''
              ''-DCMAKE_SOURCE_DIR=${src}''
            ];

            installPhase = ''
              SOURCES_DIR=${src} BUILD_DIR=. OUTPUT_DIR=$out ./post_build.sh
            '';
          };
        });

      devShells = forAllSystems (pkgs:
        let
          infinitime-nrf5-sdk = pkgs.nrf5-sdk.overrideAttrs (old: {
            version = "15.3.0";
            src = pkgs.fetchzip {
              url = "https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5sdk153059ac345.zip";
              sha256 = "sha256-pfmhbpgVv5x2ju489XcivguwpnofHbgVA7bFUJRTj08=";
            };
          });
        in
        with pkgs; {
          default =
            mkShell {
              packages = [
                (writeShellScriptBin "cmake_infinitime" ''
                  ${cmake}/bin/cmake -DARM_NONE_EABI_TOOLCHAIN_PATH="${gcc-arm-embedded-10}" \
                        -DNRF5_SDK_PATH="${infinitime-nrf5-sdk}/share/nRF5_SDK" \
                        "$@"
                '')
                ninja
              ] ++ self.packages.${pkgs.system}.default.nativeBuildInputs;
            };
        });
    };
}
      
