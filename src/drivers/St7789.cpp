#include <cstring>
#include "drivers/St7789.h"
#include <hal/nrf_gpio.h>
#include <nrfx_log.h>
#include "drivers/Spi.h"
#include "task.h"

using namespace Pinetime::Drivers;

St7789::St7789(Spi& spi, uint8_t pinDataCommand, uint8_t pinReset) : spi {spi}, pinDataCommand {pinDataCommand}, pinReset {pinReset} {
}

void St7789::Init() {
  nrf_gpio_cfg_output(pinDataCommand);
  nrf_gpio_cfg_output(pinReset);
  nrf_gpio_pin_set(pinReset);
  HardwareReset();
  SoftwareReset();
  Command2Enable();
  SleepOut();
  PixelFormat();
  MemoryDataAccessControl();
  SetAddrWindow(0, 0, Width, Height);
// P8B Mirrored version does not need display inversion.
#ifndef DRIVER_DISPLAY_MIRROR
  DisplayInversionOn();
#endif
  PorchSet();
  FrameRateNormalSet();
  IdleFrameRateOff();
  NormalModeOn();
  SetVdv();
  PowerControl();
  GateControl();
  DisplayOn();
}

void St7789::WriteData(uint8_t data) {
  WriteData(&data, 1);
}

void St7789::WriteData(const uint8_t* data, size_t size) {
  WriteSpi(data, size, [pinDataCommand = pinDataCommand]() {
    nrf_gpio_pin_set(pinDataCommand);
  });
}

void St7789::WriteCommand(uint8_t data) {
  WriteCommand(&data, 1);
}

void St7789::WriteCommand(const uint8_t* data, size_t size) {
  WriteSpi(data, size, [pinDataCommand = pinDataCommand]() {
    nrf_gpio_pin_clear(pinDataCommand);
  });
}

void St7789::WriteSpi(const uint8_t* data, size_t size, const std::function<void()>& preTransactionHook) {
  spi.Write(data, size, preTransactionHook);
}

void St7789::SoftwareReset() {
  EnsureSleepOutPostDelay();
  WriteCommand(static_cast<uint8_t>(Commands::SoftwareReset));
  // If sleep in: must wait 120ms before sleep out can sent (see driver datasheet)
  // Unconditionally wait as software reset doesn't need to be performant
  sleepIn = true;
  lastSleepExit = xTaskGetTickCount();
  vTaskDelay(pdMS_TO_TICKS(125));
}

void St7789::Command2Enable() {
  WriteCommand(static_cast<uint8_t>(Commands::Command2Enable));
  constexpr uint8_t args[] = {
    0x5a, // Constant
    0x69, // Constant
    0x02, // Constant
    0x01, // Enable
  };
  WriteData(args, sizeof(args));
}

void St7789::SleepOut() {
  if (!sleepIn) {
    return;
  }
  WriteCommand(static_cast<uint8_t>(Commands::SleepOut));
  // Wait 5ms for clocks to stabilise
  // pdMS rounds down => 6 used here
  vTaskDelay(pdMS_TO_TICKS(6));
  // Cannot send sleep in or software reset for 120ms
  lastSleepExit = xTaskGetTickCount();
  sleepIn = false;
}

void St7789::EnsureSleepOutPostDelay() {
  TickType_t delta = xTaskGetTickCount() - lastSleepExit;
  // Due to timer wraparound, there is a chance of delaying when not necessary
  // It is very low (pdMS_TO_TICKS(125)/2^32) and waiting an extra 125ms isn't too bad
  if (delta < pdMS_TO_TICKS(125)) {
    vTaskDelay(pdMS_TO_TICKS(125) - delta);
  }
}

void St7789::SleepIn() {
  if (sleepIn) {
    return;
  }
  EnsureSleepOutPostDelay();
  WriteCommand(static_cast<uint8_t>(Commands::SleepIn));
  // Wait 5ms for clocks to stabilise
  // pdMS rounds down => 6 used here
  vTaskDelay(pdMS_TO_TICKS(6));
  sleepIn = true;
}

void St7789::PixelFormat() {
  WriteCommand(static_cast<uint8_t>(Commands::PixelFormat));
  // 65K colours, 16-bit per pixel
  WriteData(0x55);
}

void St7789::MemoryDataAccessControl() {
  WriteCommand(static_cast<uint8_t>(Commands::MemoryDataAccessControl));
#ifdef DRIVER_DISPLAY_MIRROR
  // [7] = MY = Page Address Order, 0 = Top to bottom, 1 = Bottom to top
  // [6] = MX = Column Address Order, 0 = Left to right, 1 = Right to left
  // [5] = MV = Page/Column Order, 0 = Normal mode, 1 = Reverse mode
  // [4] = ML = Line Address Order, 0 = LCD refresh from top to bottom, 1 = Bottom to top
  // [3] = RGB = RGB/BGR Order, 0 = RGB, 1 = BGR
  // [2] = MH = Display Data Latch Order, 0 = LCD refresh from left to right, 1 = Right to left
  // [0 .. 1] = Unused
  WriteData(0b01000000);
#else
  WriteData(0x00);
#endif
}

void St7789::DisplayInversionOn() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayInversionOn));
}

void St7789::NormalModeOn() {
  WriteCommand(static_cast<uint8_t>(Commands::NormalModeOn));
}

void St7789::IdleModeOn() {
  WriteCommand(static_cast<uint8_t>(Commands::IdleModeOn));
}

void St7789::IdleModeOff() {
  WriteCommand(static_cast<uint8_t>(Commands::IdleModeOff));
}

void St7789::PorchSet() {
  WriteCommand(static_cast<uint8_t>(Commands::Porch));
  constexpr uint8_t args[] = {
    0x02, // Normal mode front porch
    0x03, // Normal mode back porch
    0x01, // Porch control enable
    0xed, // Idle mode front:back porch
    0xed, // Partial mode front:back porch (partial mode unused but set anyway)
  };
  WriteData(args, sizeof(args));
}

void St7789::FrameRateNormalSet() {
  WriteCommand(static_cast<uint8_t>(Commands::FrameRateNormal));
  // Note that the datasheet table is imprecise - see formula below table
  WriteData(0x0a);
}

void St7789::IdleFrameRateOn() {
  WriteCommand(static_cast<uint8_t>(Commands::FrameRateIdle));
  // According to the datasheet, these controls should apply only to partial/idle mode
  // However they appear to apply to normal mode, so we have to enable/disable
  // every time we enter/exit always on
  constexpr uint8_t args[] = {
    0x12, // Enable frame rate control for partial/idle mode, 4x frame divider
    0x1e, // Idle mode frame rate
    0x1e, // Partial mode frame rate (unused)
  };
  WriteData(args, sizeof(args));
}

void St7789::IdleFrameRateOff() {
  WriteCommand(static_cast<uint8_t>(Commands::FrameRateIdle));
  constexpr uint8_t args[] = {
    0x00, // Disable frame rate control and divider
    0x0a, // Idle mode frame rate (normal)
    0x0a, // Partial mode frame rate (normal, unused)
  };
  WriteData(args, sizeof(args));
}

void St7789::DisplayOn() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayOn));
}

void St7789::PowerControl() {
  WriteCommand(static_cast<uint8_t>(Commands::PowerControl1));
  constexpr uint8_t args[] = {
    0xa4, // Constant
    0x00, // Lowest possible voltages
  };
  WriteData(args, sizeof(args));

  WriteCommand(static_cast<uint8_t>(Commands::PowerControl2));
  // Lowest possible boost circuit clocks
  WriteData(0xb3);
}

void St7789::GateControl() {
  WriteCommand(static_cast<uint8_t>(Commands::GateControl));
  // Lowest possible VGL/VGH
  WriteData(0x00);
}

void St7789::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  WriteCommand(static_cast<uint8_t>(Commands::ColumnAddressSet));
  uint8_t colArgs[] = {
    static_cast<uint8_t>(x0 >> 8), // x start MSB
    static_cast<uint8_t>(x0),      // x start LSB
    static_cast<uint8_t>(x1 >> 8), // x end MSB
    static_cast<uint8_t>(x1)       // x end LSB
  };
  WriteData(colArgs, sizeof(colArgs));

  WriteCommand(static_cast<uint8_t>(Commands::RowAddressSet));
  uint8_t rowArgs[] = {
    static_cast<uint8_t>(y0 >> 8), // y start MSB
    static_cast<uint8_t>(y0),      // y start LSB
    static_cast<uint8_t>(y1 >> 8), // y end MSB
    static_cast<uint8_t>(y1)       // y end LSB
  };
  memcpy(addrWindowArgs, rowArgs, sizeof(rowArgs));
  WriteData(addrWindowArgs, sizeof(addrWindowArgs));
}

void St7789::WriteToRam(const uint8_t* data, size_t size) {
  WriteCommand(static_cast<uint8_t>(Commands::WriteToRam));
  WriteData(data, size);
}

void St7789::SetVdv() {
  // By default there is a large step from pixel brightness zero to one.
  // After experimenting with VCOMS, VRH and VDV, this was found to produce good results.
  WriteCommand(static_cast<uint8_t>(Commands::VdvSet));
  WriteData(0x10);
}

void St7789::DisplayOff() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayOff));
}

void St7789::VerticalScrollStartAddress(uint16_t line) {
  verticalScrollingStartAddress = line;
  WriteCommand(static_cast<uint8_t>(Commands::VerticalScrollStartAddress));
  uint8_t args[] = {
    static_cast<uint8_t>(line >> 8), // Frame memory line pointer MSB
    static_cast<uint8_t>(line)       // Frame memory line pointer LSB
  };
  memcpy(verticalScrollArgs, args, sizeof(args));
  WriteData(verticalScrollArgs, sizeof(verticalScrollArgs));
}

void St7789::Uninit() {
}

void St7789::DrawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* data, size_t size) {
  SetAddrWindow(x, y, x + width - 1, y + height - 1);
  WriteToRam(data, size);
}

void St7789::HardwareReset() {
  nrf_gpio_pin_clear(pinReset);
  vTaskDelay(pdMS_TO_TICKS(1));
  nrf_gpio_pin_set(pinReset);
  // If hardware reset started while sleep out, reset time may be up to 120ms
  // Unconditionally wait as hardware reset doesn't need to be performant
  sleepIn = true;
  lastSleepExit = xTaskGetTickCount();
  vTaskDelay(pdMS_TO_TICKS(125));
}

void St7789::LowPowerOn() {
  IdleModeOn();
  IdleFrameRateOn();
  NRF_LOG_INFO("[LCD] Low power mode");
}

void St7789::LowPowerOff() {
  IdleModeOff();
  IdleFrameRateOff();
  NRF_LOG_INFO("[LCD] Normal power mode");
}

void St7789::Sleep() {
  SleepIn();
  nrf_gpio_cfg_default(pinDataCommand);
  NRF_LOG_INFO("[LCD] Sleep");
}

void St7789::Wakeup() {
  nrf_gpio_cfg_output(pinDataCommand);
  SleepOut();
  VerticalScrollStartAddress(verticalScrollingStartAddress);
  DisplayOn();
  NRF_LOG_INFO("[LCD] Wakeup")
}
