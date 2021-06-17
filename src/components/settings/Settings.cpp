#include "Settings.h"
#include <cstdlib>
#include <cstring>

using namespace Pinetime::Controllers;

Settings::Settings(Pinetime::Controllers::FS& fs) : fs {fs} {
}

void Settings::Init() {

  // Load default settings from Flash
  LoadSettingsFromFile();
}

void Settings::SaveSettings() {

  // verify if is necessary to save
  if (settingsChanged) {
    SaveSettingsToFile();
  }
  settingsChanged = false;
}

void Settings::LoadSettingsFromFile() {
  SettingsData bufferSettings;

  fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDWR | LFS_O_CREAT);
  fs.FileRead(&settingsFile, (uint8_t *)&bufferSettings, sizeof(settings));
  fs.FileClose(&settingsFile);
  if ( bufferSettings.version == settingsVersion ) {
    std::memcpy((void *)&settings, (void *)&bufferSettings, sizeof(settings));
  }
}

void Settings::SaveSettingsToFile() {
  fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDWR | LFS_O_CREAT);
  fs.FileWrite(&settingsFile, (uint8_t *)&settings, sizeof(settings));
  fs.FileClose(&settingsFile);
}
