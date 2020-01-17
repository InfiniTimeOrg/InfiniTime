#include <hal/nrf_gpio.h>
#include <libraries/delay/nrf_delay.h>
#include "St7789.h"
#include "SpiMaster.h"

using namespace Pinetime::Drivers;

St7789::St7789(SpiMaster &spiMaster, uint8_t pinDataCommand) : spi{spiMaster}, pinDataCommand{pinDataCommand} {

}


void St7789::Init() {
  nrf_gpio_cfg_output(pinDataCommand);
  nrf_gpio_cfg_output(26);
  nrf_gpio_pin_set(26);
  HardwareReset();
  SoftwareReset();
  SleepOut();
  ColMod();
  MemoryDataAccessControl();
  ColumnAddressSet();
  RowAddressSet();
  DisplayInversionOn();
  NormalModeOn();
  DisplayOn();
}

void St7789::WriteCommand(uint8_t cmd) {
  nrf_gpio_pin_clear(pinDataCommand);
  WriteSpi(&cmd, 1);
}

void St7789::WriteData(uint8_t data) {
  nrf_gpio_pin_set(pinDataCommand);
  WriteSpi(&data, 1);
}


void St7789::WriteSpi(const uint8_t* data, size_t size) {
  spi.Write(data, size);
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
  WriteData(0x55);
  nrf_delay_ms(10);
}

void St7789::MemoryDataAccessControl() {
  WriteCommand(static_cast<uint8_t>(Commands::MemoryDataAccessControl));
  WriteData(0x00);
}

void St7789::ColumnAddressSet() {
  WriteCommand(static_cast<uint8_t>(Commands::ColumnAddressSet));
  WriteData(0x00);
  WriteData(0x00);
  WriteData(Height >> 8);
  WriteData(Height & 0xff);
}

void St7789::RowAddressSet() {
  WriteCommand(static_cast<uint8_t>(Commands::RowAddressSet));
  WriteData(0x00);
  WriteData(0x00);
  WriteData(Width >> 8);
  WriteData(Width & 0xff);
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

void St7789::FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
  BeginDrawBuffer(x, y, width, height);

  uint32_t c = color + (color << 16);
  uint8_t w = width/2;

  for(y=height+ST7789_ROW_OFFSET; y>ST7789_ROW_OFFSET; y--) {
    for(x=w; x>0; x--) {
      NextDrawBuffer(reinterpret_cast<const uint8_t *>(&c), 4);
    }
  }
  EndDrawBuffer();
}

void St7789::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  WriteCommand(static_cast<uint8_t>(Commands::ColumnAddressSet));
  WriteData(x0 >> 8);
  WriteData(x0 & 0xff);
  WriteData(x1 >> 8);
  WriteData(x1 & 0xff);

  WriteCommand(static_cast<uint8_t>(Commands::RowAddressSet));
  WriteData(y0>>8);
  WriteData(y0 & 0xff);
  WriteData(y1 >> 8);
  WriteData(y1 & 0xff);

  WriteToRam();
}

void St7789::WriteToRam() {
  WriteCommand(static_cast<uint8_t>(Commands::WriteToRam));
}

void St7789::DisplayOff() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayOff));
  nrf_delay_ms(500);
}

void St7789::Uninit() {

}

void St7789::DrawPixel(uint16_t x, uint16_t y, uint32_t color) {
  if((x < 0) ||(x >= Width) || (y < 0) || (y >= Height)) return;

  SetAddrWindow(x, y, x+1, y+1);

  nrf_gpio_pin_set(pinDataCommand);
  WriteSpi(reinterpret_cast<const uint8_t *>(&color), 2);
}

void St7789::BeginDrawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
  if((x >= Width) || (y >= Height)) return;
  if((x + width - 1) >= Width)  width = Width  - x;
  if((y + height - 1) >= Height) height = Height - y;

  SetAddrWindow(0+x, ST7789_ROW_OFFSET+y, x+width-1, y+height-1);
  nrf_gpio_pin_set(pinDataCommand);
}

void St7789::EndDrawBuffer() {
}

void St7789::NextDrawBuffer(const uint8_t *data, size_t size) {
  spi.Write(data, size);
}

void St7789::HardwareReset() {
  nrf_gpio_pin_clear(26);
  nrf_delay_ms(10);
  nrf_gpio_pin_set(26);
}

void St7789::Sleep() {
  SleepIn();
  nrf_gpio_cfg_default(pinDataCommand);
  spi.Sleep();
}

void St7789::Wakeup() {
  spi.Wakeup();

  nrf_gpio_cfg_output(pinDataCommand);
  // TODO why do we need to reset the controller?
  SoftwareReset();
  SleepOut();
  ColMod();
  MemoryDataAccessControl();
  ColumnAddressSet();
  RowAddressSet();
  DisplayInversionOn();
  NormalModeOn();
  DisplayOn();
}


