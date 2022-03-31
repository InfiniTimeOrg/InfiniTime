#include "components/settings/Settings.h"
#include <cstdlib>
#include <cstring>
#include "displayapp/lv_pinetime_theme.h"
#include "displayapp/Colors.h"

using namespace Pinetime::Controllers;

Settings::Settings(Pinetime::Controllers::FS& fs) : fs {fs} {
}

void Settings::Init() {

  // Load default settings from Flash
  LoadSettingsFromFile();

  pt_update_theme(Pinetime::Applications::Convert(settings.colorScheme.primary, settings.colorScheme.primaryTint), 
                  Pinetime::Applications::Convert(settings.colorScheme.secondary, settings.colorScheme.secondaryTint),
                  Pinetime::Applications::Convert(settings.colorScheme.surface, settings.colorScheme.surfaceTint),
                  Pinetime::Applications::Convert(settings.colorScheme.background, settings.colorScheme.backgroundTint));

}

Settings::ColorScheme Settings::getColorScheme() const{
  return settings.colorScheme;
}

void Settings::setColorScheme(ColorScheme newScheme)
{
  settings.colorScheme = newScheme;
  settingsChanged = true;
  
  pt_update_theme(Pinetime::Applications::Convert(settings.colorScheme.primary, settings.colorScheme.primaryTint), 
                  Pinetime::Applications::Convert(settings.colorScheme.secondary, settings.colorScheme.secondaryTint),
                  Pinetime::Applications::Convert(settings.colorScheme.surface, settings.colorScheme.surfaceTint),
                  Pinetime::Applications::Convert(settings.colorScheme.background, settings.colorScheme.backgroundTint));
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
  lfs_file_t settingsFile;

  if ( fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  fs.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
  fs.FileClose(&settingsFile);
  if ( bufferSettings.version == settingsVersion ) {
    settings = bufferSettings;
  }
}

void Settings::SaveSettingsToFile() {
  lfs_file_t settingsFile;

  if ( fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    return;
  }
  fs.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
  fs.FileClose(&settingsFile);
}
