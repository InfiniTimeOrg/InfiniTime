{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
  };

  outputs =
    { self, ... }@inputs:
    let
      forAllSystems =
        function:
        inputs.nixpkgs.lib.genAttrs
          [
            "x86_64-linux"
            "aarch64-linux"
          ]
          (
            system:
            function (
              import inputs.nixpkgs {
                inherit system;
                config.allowUnfree = true;
              }
            )
          );
    in
    {
      packages = forAllSystems (
        pkgs:
        let
          infinitime-nrf5-sdk = pkgs.nrf5-sdk.overrideAttrs (old: {
            version = "15.3.0";
            src = pkgs.fetchzip {
              url = "https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5sdk153059ac345.zip";
              sha256 = "sha256-pfmhbpgVv5x2ju489XcivguwpnofHbgVA7bFUJRTj08=";
            };
          });
        in
        with pkgs;
        rec {
          infinitime = stdenv.mkDerivation rec {
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

            buildInputs = with pkgs; [ cmake ];

            postPatch = ''
              # /usr/bin/env is not available in the build sandbox
              substituteInPlace src/displayapp/fonts/generate.py --replace "'/usr/bin/env', 'patch'" "'patch'"
              substituteInPlace tools/mcuboot/imgtool.py --replace "/usr/bin/env python3" "${python3}/bin/python3"
            '';

            cmakeFlags =
              let
                lvImgConvPath = "${lv_img_conv.outPath}/bin";
              in
              [
                ''-DARM_NONE_EABI_TOOLCHAIN_PATH=${gcc-arm-embedded-10}''
                ''-DNRF5_SDK_PATH=${infinitime-nrf5-sdk}/share/nRF5_SDK''
                ''-DBUILD_DFU=1''
                ''-DBUILD_RESOURCES=1''
                ''-DCMAKE_SOURCE_DIR=${src}''
                ''-DCMAKE_PROGRAM_PATH=${lvImgConvPath}''
              ];

            installPhase = ''
              SOURCES_DIR=${src} BUILD_DIR=. OUTPUT_DIR=$out ./post_build.sh
            '';
          };
          default = infinitime;
          lv_img_conv = python3Packages.buildPythonApplication rec {
            pname = "lv_img_conv";
            version = "unstable";
            format = "other";

            propagatedBuildInputs = with python3Packages; [ pillow ];

            dontUnpack = true;
            installPhase =
              let
                scriptPath = ./src/resources/lv_img_conv.py;
              in
              ''
                install -Dm755 ${scriptPath} $out/bin/${pname}
              '';
          };
        }
      );

      devShells = forAllSystems (
        pkgs:
        let
          infinitime-nrf5-sdk = pkgs.nrf5-sdk.overrideAttrs (old: {
            version = "15.3.0";
            src = pkgs.fetchzip {
              url = "https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5sdk153059ac345.zip";
              sha256 = "sha256-pfmhbpgVv5x2ju489XcivguwpnofHbgVA7bFUJRTj08=";
            };
          });
        in
        with pkgs;
        {
          default = mkShell {
            packages =
              [
                (writeShellScriptBin "cmake_infinitime" ''
                  ${cmake}/bin/cmake -DARM_NONE_EABI_TOOLCHAIN_PATH="${gcc-arm-embedded-10}" \
                        -DNRF5_SDK_PATH="${infinitime-nrf5-sdk}/share/nRF5_SDK" \
                        "$@"
                '')
                ninja
              ]
              ++ (with self.packages.${pkgs.system}; [
                infinitime
                lv_img_conv
              ]);
          };
        }
      );
    };
}
