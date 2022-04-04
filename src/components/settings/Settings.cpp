#include "components/settings/Settings.h"
#include "libs/QCBOR/inc/qcbor/qcbor.h"
#include <qcbor/qcbor_spiffy_decode.h>
#include <qcbor/qcbor_encode.h>
#include <qcbor/UsefulBuf.h>
#include <cstdlib>
#include <cstring>

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
  lfs_info cborFileInfo;
  if (fs.Stat("/settings.cbor", &cborFileInfo) != LFS_ERR_OK) {
    return;
  }

  uint8_t* cborBuf = new uint8_t[cborFileInfo.size]();

  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/settings.cbor", LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  fs.FileRead(&settingsFile, cborBuf, cborFileInfo.size);
  fs.FileClose(&settingsFile);

  SettingsData bufferSettings;
  QCBORDecodeContext decodeCtx;
  UsefulBufC encodedCbor = {cborBuf, cborFileInfo.size};

  QCBORDecode_Init(&decodeCtx, encodedCbor, QCBOR_DECODE_MODE_NORMAL);

  QCBORDecode_EnterMap(&decodeCtx, nullptr);

  uint64_t tmpStepsGoal = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "StepsGoal", &tmpStepsGoal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.stepsGoal = tmpStepsGoal;

  uint64_t tmpScreenTimeout = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ScreenTimeout", &tmpScreenTimeout);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.screenTimeOut = tmpScreenTimeout;

  uint64_t tmpClockType = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ClockType", &tmpClockType);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.clockType = static_cast<ClockType>(tmpClockType);

  uint64_t tmpNotifStatus = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "NotificationStatus", &tmpNotifStatus);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.notificationStatus = static_cast<Notification>(tmpNotifStatus);

  uint64_t tmpClockFace = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ClockFace", &tmpClockFace);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.clockFace = tmpClockFace;

  uint64_t tmpChimesOption = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ChimesOption", &tmpChimesOption);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.chimesOption = static_cast<ChimesOption>(tmpChimesOption);


  // Enter PTS colors map
  QCBORDecode_EnterMapFromMapSZ(&decodeCtx, "PTS");
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }

  uint64_t tmpColorTime = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorTime", &tmpColorTime);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.PTS.ColorTime = static_cast<Colors>(tmpColorTime);

  uint64_t tmpColorBar = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorBar", &tmpColorBar);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.PTS.ColorBar = static_cast<Colors>(tmpColorBar);

  uint64_t tmpColorBG = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorBG", &tmpColorBG);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.PTS.ColorBG = static_cast<Colors>(tmpColorBG);

  QCBORDecode_ExitMap(&decodeCtx);

  uint64_t tmpWakeUpMode = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "WakeupMode", &tmpWakeUpMode);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.wakeUpMode = std::bitset<4>(tmpWakeUpMode);

  uint64_t tmpShakeWakeThreshold = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ShakeWakeThreshold", &tmpShakeWakeThreshold);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.shakeWakeThreshold = tmpShakeWakeThreshold;

  uint64_t tmpBrightLevel = 0;
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "BrightLevel", &tmpBrightLevel);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    delete[] cborBuf;
    return;
  }
  bufferSettings.brightLevel = static_cast<Pinetime::Controllers::BrightnessController::Levels>(tmpBrightLevel);

  settings = bufferSettings;
  CleanUpQcbor(&decodeCtx);
  delete[] cborBuf;
}

void Settings::SaveSettingsToFile() {
  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/settings.cbor", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    return;
  }

  UsefulBufC encodedSize;
  encodedSize = Encode({NULL, SIZE_MAX});

  UsefulBuf encodedCborBuf;
  encodedCborBuf.len = encodedSize.len;
  encodedCborBuf.ptr = new uint8_t[encodedSize.len]();

  UsefulBufC encodedCbor = Encode(encodedCborBuf);

  fs.FileWrite(&settingsFile, (uint8_t*)(encodedCbor.ptr), encodedCbor.len);
  fs.FileClose(&settingsFile);

  delete[] (uint8_t*)(encodedCbor.ptr);
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