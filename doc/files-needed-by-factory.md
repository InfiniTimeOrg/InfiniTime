# How to generate files needed by the factory

These files are needed by the Pine64 factory to flash InfiniTime as the default firmware on the PineTimes.

Two files are needed: an **HEX (.hex)** file that contains the content of the internal flash memory (bootloader + InfiniTime) and a **binary (.bin)** file that contains the content of the external flash memory (recovery firmware).

## merged-internal.hex

First, convert the bootloader to hex:

```
<ARM TOOLCHAIN>/bin/arm-none-eabi-objcopy -I binary -O ihex ./bootloader.bin ./bootloader.hex
```

where `bootloader.bin` is the [last stable version](https://github.com/JF002/pinetime-mcuboot-bootloader/releases) of the [bootloader](https://github.com/JF002/pinetime-mcuboot-bootloader).

Then, convert the MCUBoot image of InfiniTime:

```
<ARM TOOLCHAIN>/bin/arm-none-eabi-objcopy -I binary -O ihex --change-addresses 0x8000 ./pinetime-mcuboot-app-image-1.6.0.bin ./pinetime-mcuboot-app-image-1.6.0.hex
```

where `pinetime-mcuboot-app-image-1.6.0.bin` is [the bin of the last MCUBoot image](https://github.com/InfiniTimeOrg/InfiniTime/releases) of [InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime).

Pay attention to the parameter `--change-addresses 0x8000`. It's needed to ensure the image will be flashed at the offset expected by the bootloader (0x8000).

Finally, merge them together with **mergehex**:

```
/opt/mergehex/mergehex -m ./bootloader.hex ./pinetime-mcuboot-app-image-1.6.0.hex  -o merged-internal.hex
```

This file must be flashed at offset **0x00** of the internal memory of the NRF52832.

## spinor.bin

This file is the MCUBoot image of the last stable version of the recovery firmware. It must be flashed at offset **0x00** of the external SPINOR flash memory.
