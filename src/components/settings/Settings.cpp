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
  MigrateToCBOR();
  
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
  
  // To avoid creating a uint64_t every time a value needs to be
  // decoded, this variable will be reused for all uint64_t decoding.
  uint64_t tmpVal = 0;
  
  QCBORDecode_Init(&decodeCtx, encodedCbor, QCBOR_DECODE_MODE_NORMAL);
  
  QCBORDecode_EnterMap(&decodeCtx, nullptr);
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "StepsGoal", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.stepsGoal = tmpVal;
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ScreenTimeout", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.screenTimeOut = tmpVal;
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ClockType", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are only two possible clock types, ensure the value is within bounds.
  if (tmpVal < 2) {
    bufferSettings.clockType = static_cast<ClockType>(tmpVal);
  }
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "NotificationStatus", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are only three possible notification statuses, ensure the value is within bounds.
  if (tmpVal < 3) {
    bufferSettings.notificationStatus = static_cast<Notification>(tmpVal);
  }
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ClockFace", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are only four possible clock faces, ensure the value is within bounds.
  if (tmpVal < 6) {
    bufferSettings.clockFace = tmpVal;
  }
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ChimesOption", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are only three possible chimes options, ensure the value is within bounds.
  if (tmpVal < 3) {
    bufferSettings.chimesOption = static_cast<ChimesOption>(tmpVal);
  }
  
  // Enter PTS colors map
  QCBORDecode_EnterMapFromMapSZ(&decodeCtx, "PTS");
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorTime", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are eighteen possible colors, ensure the value is within bounds.
  if (tmpVal < 18) {
    bufferSettings.PTS.ColorTime = static_cast<Colors>(tmpVal);
  }
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorBar", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are eighteen possible colors, ensure the value is within bounds.
  if (tmpVal < 18) {
    bufferSettings.PTS.ColorBar = static_cast<Colors>(tmpVal);
  }
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorBG", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are eighteen possible colors, ensure the value is within bounds.
  if (tmpVal < 18) {
    bufferSettings.PTS.ColorBG = static_cast<Colors>(tmpVal);
  }
    
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "GaugeStyle", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are eighteen possible colors, ensure the value is within bounds.
  if (tmpVal < 3) {
    bufferSettings.PTS.gaugeStyle = static_cast<PTSGaugeStyle>(tmpVal);
  }

  // Exit PTS colors map
  QCBORDecode_ExitMap(&decodeCtx);
  
  // Enter Infineat settings map
  QCBORDecode_EnterMapFromMapSZ(&decodeCtx, "Infineat");
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  
  bool tmpShowSideCover = true;
  QCBORDecode_GetBoolInMapSZ(&decodeCtx, "ShowSideCover", &tmpShowSideCover);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.watchFaceInfineat.showSideCover = tmpShowSideCover;
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ColorIndex", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.watchFaceInfineat.colorIndex = tmpVal;
  
  // Exit Infineat settings map
  QCBORDecode_ExitMap(&decodeCtx);
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "WakeupMode", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.wakeUpMode = std::bitset<4>(tmpVal);
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "ShakeWakeThreshold", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  bufferSettings.shakeWakeThreshold = tmpVal;
  
  QCBORDecode_GetUInt64InMapSZ(&decodeCtx, "BrightLevel", &tmpVal);
  if (QCBORDecode_GetError(&decodeCtx) != QCBOR_SUCCESS) {
    CleanUpQcbor(&decodeCtx);
    return;
  }
  // There are three possible brightness levels, ensure the value is within bounds.
  if (tmpVal < 3) {
    bufferSettings.brightLevel = static_cast<Controllers::BrightnessController::Levels>(tmpVal);
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
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "GaugeStyle", static_cast<uint64_t>(settings.PTS.gaugeStyle));
  QCBOREncode_CloseMap(&encodeCtx);
  
  QCBOREncode_OpenMapInMap(&encodeCtx, "Infineat");
  QCBOREncode_AddBoolToMap(&encodeCtx, "ShowSideCover", settings.watchFaceInfineat.showSideCover);
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ColorIndex", settings.watchFaceInfineat.colorIndex);
  QCBOREncode_CloseMap(&encodeCtx);
  
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "WakeupMode", settings.wakeUpMode.to_ulong());
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "ShakeWakeThreshold", settings.shakeWakeThreshold);
  QCBOREncode_AddUInt64ToMap(&encodeCtx, "BrightLevel", static_cast<uint64_t>(settings.brightLevel));
  
  QCBOREncode_CloseMap(&encodeCtx);

  UsefulBufC encodedCbor;
  QCBOREncode_Finish(&encodeCtx, &encodedCbor);

  return encodedCbor;
}

void Settings::MigrateToCBOR() {
  lfs_info oldSettingsInfo;
  if (fs.Stat("/settings.dat", &oldSettingsInfo) != LFS_ERR_OK) {
    return;
  }
  
  SettingsData bufferSettings;
  lfs_file_t settingsFile;
  if (fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  fs.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(bufferSettings));
  fs.FileClose(&settingsFile);
  if ( bufferSettings.version == settingsVersion ) {
    settings = bufferSettings;
  }
  
  SaveSettingsToFile();
  fs.FileDelete("/settings.dat");
}

void Settings::CleanUpQcbor(QCBORDecodeContext* decodeCtx) {
  QCBORDecode_ExitMap(decodeCtx);
  QCBORDecode_Finish(decodeCtx);
}