#include "components/firmwarevalidator/FirmwareValidator.h"

#include <hal/nrf_rtc.h>
#include "drivers/InternalFlash.h"

using namespace Pinetime::Controllers;

FirmwareValidator::FirmwareValidator(Pinetime::Controllers::FS& fs) : fs {fs} {
}

bool FirmwareValidator::IsValidated() const {
  auto* imageOkPtr = reinterpret_cast<uint32_t*>(validBitAdress);
  return (*imageOkPtr) == validBitValue;
}

void FirmwareValidator::Validate() {
  if (!IsValidated()) {
    Pinetime::Drivers::InternalFlash::WriteWord(validBitAdress, validBitValue);

    const char* thisVersion = Version::GitCommitHash();
    lfs_file_t autovalidateFile;
    if (fs.FileOpen(&autovalidateFile, "/.system/autovalidator.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
      return;
    }
    fs.FileWrite(&autovalidateFile, reinterpret_cast<const uint8_t*>(thisVersion), commitHashBufferLength);
    fs.FileClose(&autovalidateFile);
  }
}

void FirmwareValidator::Reset() {
  NVIC_SystemReset();
}

// make sure to call this after fs.Init() was already called or the fs read will
// fail and nothing happens here.
void FirmwareValidator::AutoValidate() {
  const std::string thisVersion = Version::GitCommitHash();
  char validVersion[commitHashBufferLength];
  lfs_file_t autovalidateFile;
  if (fs.FileOpen(&autovalidateFile, "/.system/autovalidator.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  fs.FileRead(&autovalidateFile, reinterpret_cast<uint8_t*>(validVersion), commitHashBufferLength);
  fs.FileClose(&autovalidateFile);
  if (thisVersion.compare(validVersion) == 0) {
    Validate();
  }
}
