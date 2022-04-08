#include "components/settings/Settings.h"
#include "libs/QCBOR/inc/qcbor/qcbor.h"
#include <qcbor/qcbor_spiffy_decode.h>
#include <qcbor/qcbor_encode.h>
#include <qcbor/UsefulBuf.h>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace Pinetime::Controllers;

Settings::Settings(Pinetime::Controllers::FS& fs) : fs {fs} {
}

void CleanUpQcbor(QCBORDecodeContext* decodeCtx) {
  QCBORDecode_ExitMap(decodeCtx);
  QCBORDecode_Finish(decodeCtx);
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
  MigrateSettingsToCBOR();

  lfs_info cborFileInfo;
  if (fs.Stat("/settings.cbor", &cborFileInfo) != LFS_ERR_OK) {
    return;
  }

  std::vector<uint8_t> cborBuf(cborFileInfo.size);

  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/settings.cbor", LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  fs.FileRead(&settingsFile, cborBuf.data(), cborFileInfo.size);
  fs.FileClose(&settingsFile);

  SettingsData bufferSettings;
  QCBORDecodeContext decodeCtx;
  UsefulBufC encodedCbor = {cborBuf.data(), cborFileInfo.size};

  QCBORDecode_Init(&decodeCtx, encodedCbor, QCBOR_DECODE_MODE_NORMAL);

  QCBORDecode_EnterMap(&decodeCtx, nullptr);

  uint64_t tmpStepsGoal = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "StepsGoal", &tmpStepsGoal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.stepsGoal = tmpStepsGoal;

  uint64_t tmpScreenTimeout = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ScreenTimeout", &tmpScreenTimeout);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.screenTimeOut = tmpScreenTimeout;

  uint64_t tmpClockType = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ClockType", &tmpClockType);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are only two possible clock types, ensure the value is within bounds.
  if (tmpClockType < 2) {
    bufferSettings.clockType = static_cast<ClockType>(tmpClockType);
  }

  uint64_t tmpNotifStatus = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "NotificationStatus", &tmpNotifStatus);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are only two possible notification statuses, ensure the value is within bounds.
  if (tmpNotifStatus < 2) {
    bufferSettings.notificationStatus = static_cast<Notification>(tmpNotifStatus);
  }

  uint64_t tmpClockFace = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ClockFace", &tmpClockFace);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are only four possible clock faces, ensure the value is within bounds.
  if (tmpClockFace < 4) {
    bufferSettings.clockFace = tmpClockFace;
  }

  uint64_t tmpChimesOption = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ChimesOption", &tmpChimesOption);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are only three possible chimes options, ensure the value is within bounds.
  if (tmpChimesOption < 3) {
    bufferSettings.chimesOption = static_cast<ChimesOption>(tmpChimesOption);
  }


  // Enter PTS colors map
  QCBORDecode_EnterMapFromMapSZ(&decodeCtx, "PTS");
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }

  uint64_t tmpColorTime = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorTime", &tmpColorTime);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are sixteen possible colors, ensure the value is within bounds.
  if (tmpColorTime < 16) {
    bufferSettings.PTS.ColorTime = static_cast<Colors>(tmpColorTime);
  }

  uint64_t tmpColorBar = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorBar", &tmpColorBar);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are sixteen possible colors, ensure the value is within bounds.
  if (tmpColorBar < 16) {
    bufferSettings.PTS.ColorBar = static_cast<Colors>(tmpColorBar);
  }

  uint64_t tmpColorBG = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorBG", &tmpColorBG);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are sixteen possible colors, ensure the value is within bounds.
  if (tmpColorBG < 16) {
    bufferSettings.PTS.ColorBG = static_cast<Colors>(tmpColorBG);
  }

  // Exit PTS colors map
  QCBORDecode_ExitMap(&decodeCtx);

  uint64_t tmpWakeUpMode = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "WakeupMode", &tmpWakeUpMode);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.wakeUpMode = std::bitset<4>(tmpWakeUpMode);

  uint64_t tmpShakeWakeThreshold = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ShakeWakeThreshold", &tmpShakeWakeThreshold);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.shakeWakeThreshold = tmpShakeWakeThreshold;

  uint64_t tmpBrightLevel = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "BrightLevel", &tmpBrightLevel);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are three possible brightness levels, ensure the value is within bounds.
  if (tmpBrightLevel < 3) {
    bufferSettings.brightLevel = static_cast<Pinetime::Controllers::BrightnessController::Levels>(tmpBrightLevel);
  }

  settings = bufferSettings;
  CleanUpQcbor(&decodeCtx);
}

void Settings::SaveSettingsToFile() {
  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/settings.cbor", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    return;
  }

  UsefulBufC encodedSize;
  encodedSize = Encode({NULL, SIZE_MAX});

  std::vector<uint8_t> cborBuf(encodedSize.len);

  UsefulBuf encodedCborBuf;
  encodedCborBuf.len = encodedSize.len;
  encodedCborBuf.ptr = cborBuf.data();

  UsefulBufC encodedCbor = Encode(encodedCborBuf);

  fs.FileWrite(&settingsFile, cborBuf.data(), encodedSize.len);
  fs.FileClose(&settingsFile);
}

UsefulBufC Settings::Encode(UsefulBuf buffer) {
  QCBOREncodeContext encodeCtx;
  QCBOREncode_Init(&encodeCtx, buffer);

  QCBOREncode_OpenMap(&encodeCtx);

  QCBOREncode_AddUInt64ToMap(&encodeCtx, "StepsGoal", settings.stepsGoal);
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ScreenTimeout", settings.screenTimeOut);
  
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ClockType", static_cast<uint64_t>(settings.clockType));
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "NotificationStatus", static_cast<uint64_t>(settings.notificationStatus));

  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ClockFace", settings.clockFace);
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ChimesOption", static_cast<uint64_t>(settings.chimesOption));

  QCBOREncode_OpenMapInMap(&encodeCtx, "PTS");
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ColorTime", static_cast<uint64_t>(settings.PTS.ColorTime));
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ColorBar", static_cast<uint64_t>(settings.PTS.ColorBar));
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ColorBG", static_cast<uint64_t>(settings.PTS.ColorBG));
  QCBOREncode_CloseMap(&encodeCtx);

  QCBOREncode_AddUInt64ToMap(&encodeCtx, "WakeupMode", settings.wakeUpMode.to_ulong());
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ShakeWakeThreshold", settings.shakeWakeThreshold);
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "BrightLevel", static_cast<uint64_t>(settings.brightLevel));

  QCBOREncode_CloseMap(&encodeCtx);

  UsefulBufC encodedCbor;
  QCBOREncode_Finish(&encodeCtx, &encodedCbor);

  return encodedCbor;
}

void Settings::MigrateSettingsToCBOR() {
  lfs_info oldSettingsInfo;
  if (fs.Stat("/settings.dat", &oldSettingsInfo) != LFS_ERR_OK) {
    return;
  }

  SettingsData bufferSettings;
  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  fs.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
  fs.FileClose(&settingsFile);
  if ( bufferSettings.version == settingsVersion ) {
    settings = bufferSettings;
  }

  SaveSettingsToFile();

  fs.FileDelete("/settings.dat");
}