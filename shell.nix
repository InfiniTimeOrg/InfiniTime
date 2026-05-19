{ pkgs ? import <nixpkgs> {} }:

let
  infinitime-nrf5-sdk = pkgs.nrf5-sdk.overrideAttrs (old: {
    version = "15.3.0";
    src = pkgs.fetchzip {
      url = "https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5sdk153059ac345.zip";
      sha256 = "sha256-pfmhbpgVv5x2ju489XcivguwpnofHbgVA7bFUJRTj08=";
    };
  });
in pkgs.mkShell {
    # build tools
    nativeBuildInputs = with pkgs; [
      cmake
      lv_img_conv
      nodePackages.lv_font_conv
      python3
      python3.pkgs.cbor
      python3.pkgs.click
      python3.pkgs.cryptography
      python3.pkgs.intelhex
      python3.pkgs.adafruit-nrfutil

      (pkgs.writeShellScriptBin "cmake_infinitime" ''
        cmake -DARM_NONE_EABI_TOOLCHAIN_PATH="${pkgs.gcc-arm-embedded-10}" \
              -DNRF5_SDK_PATH="${infinitime-nrf5-sdk}/share/nRF5_SDK" \
              "$@"
        '')
    ];

    # libraries/dependencies
    buildInputs = with pkgs; [];
}
