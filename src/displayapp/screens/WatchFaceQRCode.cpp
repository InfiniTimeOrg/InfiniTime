#include "displayapp/screens/WatchFaceQRCode.h"

using namespace Pinetime::Applications::Screens;

WatchFaceQRCode::WatchFaceQRCode(Components::LittleVgl& lvgl,
                                 Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 Controllers::Settings& settingsController,
                                 Controllers::MotorController& motor)
  : lvgl{lvgl},
    dateTimeController{dateTimeController},
    batteryController{batteryController},
    bleController{bleController},
    settingsController{settingsController},
    motor{motor} {

  altTextIndex = 0;
  altTextIndexUpdated = false;
  altTextLastChangedTime = xTaskGetTickCount();

  // White background for entire screen
  lv_obj_t* whiteBorder = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(whiteBorder, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_size(whiteBorder, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_style_local_radius(whiteBorder, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

  // Gray placeholder to show before the qr code gets fully drawn
  lv_obj_t* grayPlaceholder = lv_obj_create(whiteBorder, nullptr);
  lv_obj_set_style_local_bg_color(grayPlaceholder, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_size(grayPlaceholder,
                  (lv_coord_t) ((LV_HOR_RES * 33. + quietZoneSize) / (33. + 2 * quietZoneSize)),
                  (lv_coord_t) ((LV_VER_RES * 33. + quietZoneSize) / (33. + 2 * quietZoneSize)));
  lv_obj_align(grayPlaceholder, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_radius(grayPlaceholder, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

  // Populate buffers for qr code drawing
  const int bufSize = (int) (ceil(LV_HOR_RES / (33. + 2 * quietZoneSize)) * ceil(LV_VER_RES / (33. + 2 * quietZoneSize)));
  whiteBuffer = std::unique_ptr<lv_color_t[]>(new lv_color_t[bufSize]);
  lv_color_fill(whiteBuffer.get(), LV_COLOR_WHITE, bufSize);
  blackBuffer = std::unique_ptr<lv_color_t[]>(new lv_color_t[bufSize]);
  lv_color_fill(blackBuffer.get(), LV_COLOR_BLACK, bufSize);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

WatchFaceQRCode::~WatchFaceQRCode() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

/// Long tap to move to next alternate text
bool WatchFaceQRCode::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::LongTap) {
    altTextIndex = (altTextIndex + 1) % (altTexts.size() + 1);
    altTextLastChangedTime = xTaskGetTickCount();
    altTextIndexUpdated = true;
    BuzzBinary(altTextIndex);
    return true;
  }
  return false;
}

/// Reset back to normal time QR code if displaying alternate text
bool WatchFaceQRCode::OnButtonPushed() {
  if (altTextIndex != 0) {
    altTextIndex = 0;
    altTextIndexUpdated = true;
    BuzzBinary(0);
    return true;
  }
  return false;
}

void WatchFaceQRCode::Refresh() {
  bool timeTextNeedsRefresh = false;

  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    timeTextNeedsRefresh = true;
    // Needs to be called to reset isUpdated flag on powerPresent, else screen updates twice on object creation
    powerPresent.IsUpdated();
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    timeTextNeedsRefresh = true;
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    timeTextNeedsRefresh = true;
  }

  if (altTextIndex != 0 && xTaskGetTickCount() - altTextLastChangedTime >= altTextTimeout) {
    altTextIndex = 0;
    altTextIndexUpdated = true;
    BuzzBinary(0);
  }

  if ((timeTextNeedsRefresh && altTextIndex == 0) || altTextIndexUpdated) {
    altTextIndexUpdated = false;
    RefreshQRCode();
  }
}

/// Buzzes the provided number out in binary. Long buzz = 1, short buzz = 0.
void WatchFaceQRCode::BuzzBinary(const uint8_t number) const {
  static constexpr uint8_t longBuzzLenMS = 50;
  static constexpr uint8_t shortBuzzLenMS = 20;
  static constexpr uint8_t pauseLenMS = 250;

  if (number == 0) {
    motor.RunForDuration(shortBuzzLenMS);
    return;
  }

  uint8_t mask = 0x01;
  while (mask << 1 <= number && mask < 0x80) {
    mask <<= 1;
  }

  while (mask > 0) {
    if ((bool) (number & mask)) {
      motor.RunForDuration(longBuzzLenMS);
    } else {
      motor.RunForDuration(shortBuzzLenMS);
    }
    vTaskDelay(pdMS_TO_TICKS(pauseLenMS));
    mask >>= 1;
  }
}

/// Create and return the time text to be displayed as the main watchface.
// ReSharper disable once CppDFAUnreachableFunctionCall
std::string WatchFaceQRCode::MakeQRTimeText() const {
  /*
  - FORMAT:
  {
  "time": "12:30 PM",
  "date": "11 Jan 2025",
  "bt"  :  false,
  "batt": "+100"
  }
  - NOTES:
  Time respects 12/24 hour format
  Battery level is a string, and has a plus in front of it when charging
  */

  auto textToEncode = std::unique_ptr<char[]>(new char[80]);
  int textLength = 0;
  int printReturn = 0;

  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    printReturn = snprintf(&textToEncode[textLength],
                           80 - textLength,
                           "{\n\"time\": \"%i:%.2i %cM\",\n",
                           dateTimeController.Hours() > 12 ? dateTimeController.Hours() - 12 : dateTimeController.Hours(),
                           dateTimeController.Minutes(),
                           dateTimeController.Hours() > 12 ? 'P' : 'A');
  } else {
    printReturn = snprintf(&textToEncode[textLength],
                           80 - textLength,
                           "{\n\"time\": \"%i:%.2i\",\n",
                           dateTimeController.Hours(),
                           dateTimeController.Minutes());
  }

  if (printReturn < 0) {
    return "{\n\"error\": \"Failed to encode watch: TIME\",\n\"apology\": \"Sowwy :3\"\n}";
  }
  textLength += printReturn;

  printReturn = snprintf(&textToEncode[textLength],
                         80 - textLength,
                         "\"date\": \"%i %s %.4i\",\n\"bt\"  :  %s,\n",
                         dateTimeController.Day(),
                         Controllers::DateTime::MonthShortToStringLow(dateTimeController.Month()),
                         dateTimeController.Year(),
                         bleController.IsConnected() ? "true" : "false");

  if (printReturn < 0) {
    return "{\n\"error\": \"Failed to encode watch: DATE/BT\",\n\"apology\": \"Sowwy :3\"\n}";
  }
  textLength += printReturn;

  if (batteryController.IsPowerPresent()) {
    printReturn = snprintf(&textToEncode[textLength],
                           80 - textLength,
                           "\"batt\": \"+%i\"\n}",
                           batteryController.PercentRemaining());
  } else {
    printReturn = snprintf(&textToEncode[textLength],
                           80 - textLength,
                           "\"batt\": \"%i\"\n}",
                           batteryController.PercentRemaining());
  }

  if (printReturn < 0) {
    return "{\n\"error\": \"Failed to encode watch: BATTERY\",\n\"apology\": \"Sowwy :3\"\n}";
  }

  return std::string(textToEncode.get());
}

/// Refresh the entire QR code being displayed, including reevaluating text.
void WatchFaceQRCode::RefreshQRCode() const {
  std::string workingText;
  if (altTextIndex == 0) {
    workingText = MakeQRTimeText();
  } else {
    workingText = altTexts[altTextIndex - 1];
  }

  const std::shared_ptr<BitByteArray2D> qrCodeImg = QRCodeGenerator::GenerateCode(workingText);

  lv_area_t area;
  for (int x = 0; x < 33; x++) {
    for (int y = 0; y < 33; y++) {
      area.x1 = (lv_coord_t) (LV_HOR_RES * (x + quietZoneSize) / (33. + 2 * quietZoneSize));
      area.y1 = (lv_coord_t) (LV_VER_RES * (y + quietZoneSize) / (33. + 2 * quietZoneSize));
      area.x2 = (lv_coord_t) (LV_HOR_RES * (x + quietZoneSize + 1) / (33. + 2 * quietZoneSize) - 1);
      area.y2 = (lv_coord_t) (LV_VER_RES * (y + quietZoneSize + 1) / (33. + 2 * quietZoneSize) - 1);
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);

      if ((int) qrCodeImg->getBit2D(x, y) == 0) {
        lvgl.FlushDisplay(&area, whiteBuffer.get());
      } else {
        lvgl.FlushDisplay(&area, blackBuffer.get());
      }
    }
  }
}

BitByteArray::BitByteArray(const uint16_t sizeBytes)
  : size{sizeBytes} {
  this->byteArray = std::unique_ptr<uint8_t[]>(new uint8_t[sizeBytes]);
  std::fill_n(byteArray.get(), sizeBytes, 0);
}

/// Size of array in bytes
uint16_t BitByteArray::length() const {
  return size;
}

uint8_t& BitByteArray::operator[](const uint16_t index) {
  assert(index < this->size);
  return byteArray[index];
}

uint8_t BitByteArray::getByte(const uint16_t index) const {
  assert(index < this->size);
  return byteArray[index];
}

void BitByteArray::setByte(const uint16_t index, const uint8_t value) {
  assert(index < this->size);
  byteArray[index] = value;
}

bool BitByteArray::getBit(const uint16_t index) const {
  assert(index < this->size * 8);
  return (bool) (byteArray[index / 8] & (0x80 >> index % 8));
}

void BitByteArray::setBit(const uint16_t index, const bool value) {
  assert(index < this->size * 8);
  if (!value) {
    byteArray[index / 8] &= 0xFF - (0x80 >> index % 8);
  } else {
    byteArray[index / 8] |= 0x80 >> index % 8;
  }
}

uint8_t BitByteArray::getNybble(const uint16_t index) const {
  assert(index < this->size * 2);
  if (index % 2 == 1) {
    return byteArray[index / 2] & 0x0F;
  }
  return byteArray[index / 2] & 0xF0;
}

void BitByteArray::setNybble(const uint16_t index, const uint8_t value) {
  assert(index < this->size * 2);
  if (index % 2 == 1) {
    byteArray[index / 2] = (byteArray[index / 2] & 0xF0) | (value & 0x0F);
  } else {
    byteArray[index / 2] = (byteArray[index / 2] & 0x0F) | ((value & 0x0F) << 4);
  }
}

BitByteArray2D::BitByteArray2D(const uint16_t width, const uint16_t height)
  : BitByteArray(width * height),
    width{width},
    height{height} {
}

bool BitByteArray2D::getBit2D(const uint16_t indexX, const uint16_t indexY) const {
  assert(indexX < width);
  assert(indexY < height);
  return getBit((indexY * width) + indexX);
}

void BitByteArray2D::setBit2D(const uint16_t indexX, const uint16_t indexY, const bool value) {
  assert(indexX < width);
  assert(indexY < height);
  setBit((indexY * width) + indexX, value);
}

/// Fills the area with the given value. Inclusive on all sides.
void BitByteArray2D::FillBits(const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2, const bool value) {
  assert(x1 <= x2);
  assert(x2 < width);
  assert(y1 <= y2);
  assert(y2 < height);
  for (uint16_t xCoord = x1; xCoord <= x2; xCoord++) {
    for (uint16_t yCoord = y1; yCoord <= y2; yCoord++) {
      setBit((yCoord * width) + xCoord, value);
    }
  }
}


/**
 * @param text The text to convert to a QR code. Must be encoded in ISO-8859-1 (which is ASCII compatible).
 * @return a 33x33 BitByteArray2D representing the modules of the code. 0 is white, 1 is black.
 */
std::shared_ptr<BitByteArray2D> QRCodeGenerator::GenerateCode(const std::string& text) {
  std::string workingText;
  if (text.length() > 78) {
    workingText = "{\n\"error\": \"DATA TOO LONG\",\n\"apology\": \"Sowwy :3\"\n}";
  } else {
    workingText = text;
  }

  const std::shared_ptr<BitByteArray> codeData = GenerateCodeData(workingText);
  AppendCodeEC(codeData);
  return GenerateCodeImage(codeData);
}

/**
 * @param text The text to encode. Must be encoded in ISO-8859-1 and not be longer than 78 bytes.
 * @return a 101-length BitByteArray (80 bytes populated) containing the base data, to append EC data to.
 */
std::shared_ptr<BitByteArray> QRCodeGenerator::GenerateCodeData(const std::string& text) {
  auto data = std::make_shared<BitByteArray>(101);

  // QR code type and length
  data->setNybble(0, 0b0100);
  data->setNybble(1, ((uint8_t) text.length() & 0xF0) >> 4);
  data->setNybble(2, (uint8_t) text.length() & 0x0F);

  // Insert user provided text
  uint16_t nybbleIndex = 3;
  for (int i = 0; i < std::min((int) text.length(), 78); i++) {
    data->setNybble(nybbleIndex, (text[i] & 0xF0) >> 4);
    data->setNybble(nybbleIndex + 1, text[i] & 0x0F);
    nybbleIndex += 2;
  }

  // Pad remainder with alternating 0xEC and 0x11 bytes
  uint16_t byteIndex = (nybbleIndex + 1) / 2;
  while (true) {
    if (byteIndex >= 80) {
      break;
    }
    data->setByte(byteIndex, 0xEC);
    byteIndex++;
    if (byteIndex >= 80) {
      break;
    }
    data->setByte(byteIndex, 0x11);
    byteIndex++;
  }

  return data;
}


/**
 * Modifies the passed BitByteArray in-place to add 20 bytes of Reed-Solomon error correction at the end.
 * @param baseData A BitByteArray with <=80 populated bytes and size >=100 bytes.
 */
void QRCodeGenerator::AppendCodeEC(const std::shared_ptr<BitByteArray> baseData) {
  // https://www.thonky.com/qr-code-tutorial/error-correction-coding

  BitByteArray dataPolynomialInt = BitByteArray(100);

  // Copy initial polynomial state to new BitByteArray for holding the long division state
  for (int i = 0; i < 80; i++) {
    dataPolynomialInt[i] = baseData->getByte(i);
  }

  // Polynomial long division: dataPolynomialInt / generatorPolynomialAlpha
  for (uint16_t i = 0; i < 80; i++) {
    // if got extra 0 in the lead, pass over it
    if (dataPolynomialInt[i] == 0) {
      continue;
    }
    // multiplier to put onto generator polynomial
    const uint16_t multiplierAlpha = intToAlpha[dataPolynomialInt[i]];
    // add multiplied generator polynomial to the message polynomial (by xor)
    for (uint16_t j = 0; j < 21; j++) {
      dataPolynomialInt[i + j] ^= alphaToInt[(generatorPolynomialAlpha[j] + multiplierAlpha) % 255];
    }
  }

  // dataPolynomialInt now only has 20 digits at the tail containing the EC data, copy it over
  for (int i = 80; i < 100; i++) {
    baseData->setByte(i, dataPolynomialInt.getByte(i));
  }
}

/**
 * @param codeData a >=101 byte BitByteArray containing the base data + error correction data to put in the code.
 * @return a 33x33 BitByteArray2D representing the modules of the code. 0 is white, 1 is black.
 */
std::shared_ptr<BitByteArray2D> QRCodeGenerator::GenerateCodeImage(const std::shared_ptr<BitByteArray> codeData) {
  auto canvas = std::make_shared<BitByteArray2D>(33, 33);

  // Finder patterns
  // top left
  canvas->FillBits(0, 0, 7, 7, false);
  canvas->FillBits(0, 0, 6, 6, true);
  canvas->FillBits(1, 1, 5, 5, false);
  canvas->FillBits(2, 2, 4, 4, true);
  // top right
  canvas->FillBits(25, 0, 32, 7, false);
  canvas->FillBits(26, 0, 32, 6, true);
  canvas->FillBits(27, 1, 31, 5, false);
  canvas->FillBits(28, 2, 30, 4, true);
  // bottom left
  canvas->FillBits(0, 25, 7, 32, false);
  canvas->FillBits(0, 26, 6, 32, true);
  canvas->FillBits(1, 27, 5, 31, false);
  canvas->FillBits(2, 28, 4, 30, true);
  // alignment pattern
  canvas->FillBits(24, 24, 28, 28, true);
  canvas->FillBits(25, 25, 27, 27, false);
  canvas->setBit2D(26, 26, true);

  // Timing patterns
  for (uint16_t x = 6; x < 25; x++) {
    canvas->setBit2D(x, 6, x % 2 == 0);
  }
  for (uint16_t y = 6; y < 25; y++) {
    canvas->setBit2D(6, y, y % 2 == 0);
  }

  // Dark module
  canvas->setBit2D(8, 25, true);

  // Version information
  BitByteArray versionInfo = BitByteArray(2);
  versionInfo[0] = 0xE5;
  versionInfo[1] = 0xE6;
  // Version info around top left finder pattern
  uint8_t versionIdx = 0;
  for (uint8_t x = 0; x < 6; x++) {
    canvas->setBit2D(x, 8, versionInfo.getBit(versionIdx));
    versionIdx++;
  }
  canvas->setBit2D(7, 8, versionInfo.getBit(6));
  canvas->setBit2D(8, 8, versionInfo.getBit(7));
  canvas->setBit2D(8, 7, versionInfo.getBit(8));
  versionIdx += 3;
  for (int8_t y = 5; y > -1; y--) {
    canvas->setBit2D(8, y, versionInfo.getBit(versionIdx));
    versionIdx++;
  }
  // Version info around bottom left and top right finder patterns
  versionIdx = 0;
  for (uint8_t y = 32; y > 25; y--) {
    canvas->setBit2D(8, y, versionInfo.getBit(versionIdx));
    versionIdx++;
  }
  for (uint8_t x = 25; x < 33; x++) {
    canvas->setBit2D(x, 8, versionInfo.getBit(versionIdx));
    versionIdx++;
  }

  // Add data into the code
  BitByteArray2D mask = BitByteArray2D(33, 33);
  mask.FillBits(0, 0, 32, 32, true);
  mask.FillBits(0, 0, 8, 8, false);
  mask.FillBits(0, 25, 8, 32, false);
  mask.FillBits(25, 0, 32, 8, false);
  mask.FillBits(9, 6, 24, 6, false);
  mask.FillBits(6, 9, 6, 24, false);
  mask.FillBits(24, 24, 28, 28, false);

  uint16_t dataIdx = 0;
  int8_t wideColX = 32;
  while (wideColX > 0) {
    if ((wideColX > 5 && (wideColX / 2) % 2 == 0) || (wideColX <= 5 && wideColX == 3)) {
      // going up
      for (int8_t y = 32; y > -1; y--) {
        for (int8_t x = wideColX; x > wideColX - 2; x--) {
          if (!mask.getBit2D(x, y)) {
            continue;
          }
          canvas->setBit2D(x, y, (int) codeData->getBit(dataIdx) == y % 2);
          dataIdx++;
        }
      }
    } else {
      // going down
      for (int8_t y = 0; y < 33; y++) {
        for (int8_t x = wideColX; x > wideColX - 2; x--) {
          if (!mask.getBit2D(x, y)) {
            continue;
          }
          canvas->setBit2D(x, y, (int) codeData->getBit(dataIdx) == y % 2);
          dataIdx++;
        }
      }
    }
    wideColX -= 2;
    if (wideColX == 6) {
      wideColX--;
    }
  }

  return canvas;
}