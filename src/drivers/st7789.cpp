#include <hal/nrf_gpio.h>
#include <libraries/delay/nrf_delay.h>
#include "st7789.h"
#include "spi_master_fast.h"

using namespace Pinetime::Drivers;

ret_code_t st7789::Init() {
  InitHw();
  InitCommands();
  return 0;
}

ret_code_t st7789::InitHw() const {
  nrf_gpio_cfg_output(ST7735_DC_PIN);
  SPI_config_t spi_config;

  spi_config.pin_SCK                 = ST7735_SCK_PIN;
  spi_config.pin_MOSI                 = ST7735_MOSI_PIN;
  spi_config.pin_MISO                 = ST7735_MISO_PIN;
  spi_config.pin_CSN                 = ST7735_SS_PIN;
  spi_config.frequency                 = SPI_FREQ_8MBPS;
  spi_config.config.fields.mode                 = SPI_MODE3;
  spi_config.config.fields.bit_order                 = SPI_BITORDER_MSB_LSB;


  spi_master_init(SPI0, &spi_config);

  return 0;
}

void st7789::InitCommands() {
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

void st7789::WriteCommand(uint8_t cmd) {
  nrf_gpio_pin_clear(ST7735_DC_PIN);
  WriteSpi(&cmd, 1);
}

void st7789::WriteData(uint8_t data) {
  nrf_gpio_pin_set(ST7735_DC_PIN);
  WriteSpi(&data, 1);
}


void st7789::WriteSpi(const uint8_t* data, size_t size) {
//  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, data, size, nullptr, 0));
  spi_master_tx(SPI0, size, data);
}

void st7789::SoftwareReset() {
  WriteCommand(static_cast<uint8_t>(Commands::SoftwareReset));
  nrf_delay_ms(150);
}

void st7789::SleepOut() {
  WriteCommand(static_cast<uint8_t>(Commands::SleepOut));
  nrf_delay_ms(500);
}

void st7789::ColMod() {
  WriteCommand(static_cast<uint8_t>(Commands::ColMod));
  WriteData(0x55);
  nrf_delay_ms(10);
}

void st7789::MemoryDataAccessControl() {
  WriteCommand(static_cast<uint8_t>(Commands::MemoryDataAccessControl));
  WriteData(0x00);
}

void st7789::ColumnAddressSet() {
  WriteCommand(static_cast<uint8_t>(Commands::ColumnAddressSet));
  WriteData(0x00);
  WriteData(0x00);
  WriteData(Height >> 8);
  WriteData(Height & 0xff);
}

void st7789::RowAddressSet() {
  WriteCommand(static_cast<uint8_t>(Commands::RowAddressSet));
  WriteData(0x00);
  WriteData(0x00);
  WriteData(Width >> 8);
  WriteData(Width & 0xff);
}

void st7789::DisplayInversionOn() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayInversionOn));
  nrf_delay_ms(10);
}

void st7789::NormalModeOn() {
  WriteCommand(static_cast<uint8_t>(Commands::NormalModeOn));
  nrf_delay_ms(10);
}

void st7789::DisplayOn() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayOn));
  nrf_delay_ms(500);
}

void st7789::FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= Width) || (y >= Height)) return;
  if((x + width - 1) >= Width)  width = Width  - x;
  if((y + height - 1) >= Height) height = Height - y;

  SetAddrWindow(0+x, ST7789_ROW_OFFSET+y, x+width-1, y+height-1);

  uint8_t hi = color >> 8, lo = color;
  uint32_t c = color + (color << 16);

  nrf_gpio_pin_set(ST7735_DC_PIN);
  for(y=height+ST7789_ROW_OFFSET; y>ST7789_ROW_OFFSET; y--) {
    for(x=width; x>0; x--) {
      WriteSpi(reinterpret_cast<const uint8_t *>(&c), 4);
    }
  }
}

void st7789::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
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

void st7789::WriteToRam() {
  WriteCommand(static_cast<uint8_t>(Commands::WriteToRam));
}

void st7789::DisplayOff() {
  WriteCommand(static_cast<uint8_t>(Commands::DisplayOff));
  nrf_delay_ms(500);
}

void st7789::Uninit() {

}

void st7789::DrawPixel(uint16_t x, uint16_t y, uint32_t color) {
  if((x < 0) ||(x >= Width) || (y < 0) || (y >= Height)) return;

  SetAddrWindow(x, y, x+1, y+1);

  nrf_gpio_pin_set(ST7735_DC_PIN);
  WriteSpi(reinterpret_cast<const uint8_t *>(&color), 2);
}
