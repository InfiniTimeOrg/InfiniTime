#include "St7789.h"
#include <hal/nrf_gpio.h>
#include <libraries/delay/nrf_delay.h>
#include <nrfx_log.h>
#include "Spi.h"

using namespace Pinetime::Drivers;

St7789::St7789(Spi& spi, uint8_t pinDataCommand) : spi {spi}, pinDataCommand {pinDataCommand} {
}

void St7789::Init() {
  spi.Init();
  nrf_gpio_cfg_output(pinDataCommand);
  nrf_gpio_cfg_output(26);
  nrf_gpio_pin_set(26);
  HardwareReset();
  SoftwareReset();
  SleepOut();
  ColMod();
  MemoryDataAccessControl();
  VerticalScrollDefinition(0,320,0);
  ColumnAddressSet();
  RowAddressSet();
  DisplayInversionOn();
  NormalModeOn();
  DisplayOn();
}

void St7789::WriteCommand(uint8_t cmd) {
  spi.Write(&cmd, 1, pinDataCommand, false);
}

void St7789::WriteDataByte(uint8_t data) {
  spi.Write(&data, 1, pinDataCommand, true);
}

void St7789::WriteData(const uint8_t* data, size_t size) {
  spi.Write(data, size, pinDataCommand, true);
}

void St7789::SoftwareReset() {
  WriteCommand(static_cast<uint8_t>(Commands::SoftwareReset));
  nrf_delay_ms(150);
}

void St7789::SleepOut() {
  WriteCommand(static_cast<uint8_t>(Commands::SleepOut));
}

void St7789::SleepIn() {
  WriteCommand(static_cast<uint8_t>(Commands::SleepIn));
}

void St7789::ColMod() {
  WriteCommand(static_cast<uint8_t>(Commands::ColMod));
  WriteDataByte(0x55);
  nrf_delay_ms(10);
}

void St7789::MemoryDataAccessControl() {
  WriteCommand(static_cast<uint8_t>(Commands::MemoryDataAccessControl));
  WriteDataByte(0x00);
}

void St7789::ColumnAddressSet() {
  WriteCommand(static_cast<uint8_t>(Commands::ColumnAddressSet));
  WriteDataByte(0x00);
  WriteDataByte(0x00);
  WriteDataByte(Width >> 8u);
  WriteDataByte(Width & 0xffu);
}

void St7789::RowAddressSet() {
  WriteCommand(static_cast<uint8_t>(Commands::RowAddressSet));
  WriteDataByte(0x00);
  WriteDataByte(0x00);
  WriteDataByte(320u >> 8u);
  WriteDataByte(320u & 0xffu);
}

void St7789::DisplayInversionOn() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayInversionOn));
  nrf_delay_ms(10);
}

void St7789::NormalModeOn() {
  WriteCommand(static_cast<uint8_t>(Commands::NormalModeOn));
  nrf_delay_ms(10);
}

void St7789::DisplayOn() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayOn));
}

void St7789::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  WriteCommand(static_cast<uint8_t>(Commands::ColumnAddressSet));
  WriteDataByte(x0 >> 8);
  WriteDataByte(x0 & 0xff);
  WriteDataByte(x1 >> 8);
  WriteDataByte(x1 & 0xff);

  WriteCommand(static_cast<uint8_t>(Commands::RowAddressSet));
  WriteDataByte(y0 >> 8);
  WriteDataByte(y0 & 0xff);
  WriteDataByte(y1 >> 8);
  WriteDataByte(y1 & 0xff);

  WriteToRam();
}

void St7789::WriteToRam() {
  WriteCommand(static_cast<uint8_t>(Commands::WriteToRam));
}

void St7789::DisplayOff() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayOff));
  nrf_delay_ms(500);
}

void St7789::VerticalScrollDefinition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines) {
  WriteCommand(static_cast<uint8_t>(Commands::VerticalScrollDefinition));
  WriteDataByte(topFixedLines >> 8u);
  WriteDataByte(topFixedLines & 0x00ffu);
  WriteDataByte(scrollLines >> 8u);
  WriteDataByte(scrollLines & 0x00ffu);
  WriteDataByte(bottomFixedLines >> 8u);
  WriteDataByte(bottomFixedLines & 0x00ffu);
}

void St7789::VerticalScrollStartAddress(uint16_t line) {
  verticalScrollingStartAddress = line;
  WriteCommand(static_cast<uint8_t>(Commands::VerticalScrollStartAddress));
  WriteDataByte(line >> 8u);
  WriteDataByte(line & 0x00ffu);
}

void St7789::Uninit() {
}

void St7789::DrawPixel(uint16_t x, uint16_t y, uint32_t color) {
  if (x >= Width || y >= Height) {
    return;
  }

  SetAddrWindow(x, y, x + 1, y + 1);

  WriteData(reinterpret_cast<const uint8_t*>(&color), 2);
}

void St7789::DrawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* data, size_t size) {
  SetAddrWindow(x, y, x + width - 1, y + height - 1);
  WriteData(data, size);
}

void St7789::HardwareReset() {
  nrf_gpio_pin_clear(26);
  nrf_delay_ms(10);
  nrf_gpio_pin_set(26);
}

void St7789::Sleep() {
  SleepIn();
  nrf_gpio_cfg_default(pinDataCommand);
  NRF_LOG_INFO("[LCD] Sleep");
}

void St7789::Wakeup() {
  nrf_gpio_cfg_output(pinDataCommand);
  // TODO why do we need to reset the controller?
  HardwareReset();
  SoftwareReset();
  SleepOut();
  ColMod();
  MemoryDataAccessControl();
  ColumnAddressSet();
  RowAddressSet();
  DisplayInversionOn();
  NormalModeOn();
  VerticalScrollStartAddress(verticalScrollingStartAddress);
  DisplayOn();
  NRF_LOG_INFO("[LCD] Wakeup")
}
