#include "Settings.h"
#include <cstdlib>
#include <cstring>

using namespace Pinetime::Controllers;

struct SettingsHeader {
  uint8_t isActive; // 0xF1 = Block is active, 0xF0 = Block is inactive
  uint16_t version; // Current version, to verify if the saved data is for the current Version
};

#define HEADER_SIZE sizeof(SettingsHeader)

Settings::Settings(Pinetime::Drivers::SpiNorFlash& spiNorFlash) : spiNorFlash {spiNorFlash} {
}

void Settings::Init() {

  // Load default settings from Flash
  LoadSettingsFromFlash();
}

void Settings::SaveSettings() {

  // verify if is necessary to save
  if (settingsChanged) {
    SaveSettingsToFlash();
  }
  settingsChanged = false;
}

bool Settings::FindHeader() {
  SettingsHeader settingsHeader;
  uint8_t bufferHead[sizeof(settingsHeader)];

  for (uint8_t block = 0; block < 10; block++) {

    spiNorFlash.Read(settingsBaseAddr + (block * 0x1000), bufferHead, sizeof(settingsHeader));
    std::memcpy(&settingsHeader, bufferHead, sizeof(settingsHeader));
    if (settingsHeader.isActive == 0xF1 && settingsHeader.version == settingsVersion) {
      settingsFlashBlock = block;
      return true;
    }
  }
  return false;
}

void Settings::ReadSettingsData() {
  uint8_t bufferSettings[sizeof(settings)];
  spiNorFlash.Read(settingsBaseAddr + (settingsFlashBlock * 0x1000) + HEADER_SIZE, bufferSettings, sizeof(settings));
  std::memcpy(&settings, bufferSettings, sizeof(settings));
}

void Settings::EraseBlock() {

  spiNorFlash.SectorErase(settingsBaseAddr + (settingsFlashBlock * 0x1000));
}

void Settings::SetHeader(bool state) {
  SettingsHeader settingsHeader;
  uint8_t bufferHead[sizeof(settingsHeader)];
  settingsHeader.isActive = state ? 0xF1 : 0xF0;
  settingsHeader.version = settingsVersion;

  std::memcpy(bufferHead, &settingsHeader, sizeof(settingsHeader));
  spiNorFlash.Write(settingsBaseAddr + (settingsFlashBlock * 0x1000), bufferHead, sizeof(settingsHeader));
}

void Settings::SaveSettingsData() {
  uint8_t bufferSettings[sizeof(settings)];
  std::memcpy(bufferSettings, &settings, sizeof(settings));
  spiNorFlash.Write(settingsBaseAddr + (settingsFlashBlock * 0x1000) + HEADER_SIZE, bufferSettings, sizeof(settings));
}

void Settings::LoadSettingsFromFlash() {

  if (settingsFlashBlock == 99) {
    // Find current Block, if can't find use default settings and set block to 0 ans save !
    if (FindHeader()) {
      ReadSettingsData();
    } else {
      SaveSettingsToFlash();
    }
  } else {
    // Read Settings from flash...
    // never used :)
    ReadSettingsData();
  }
}

void Settings::SaveSettingsToFlash() {

  // calculate where to save...
  // mark current to inactive
  // erase the new location and save
  // set settingsFlashBlock

  // if first time hever, only saves to block 0 and set settingsFlashBlock

  if (settingsFlashBlock != 99) {
    SetHeader(false);
  }

  settingsFlashBlock++;
  if (settingsFlashBlock > 9)
    settingsFlashBlock = 0;

  EraseBlock();
  SetHeader(true);
  SaveSettingsData();
}
