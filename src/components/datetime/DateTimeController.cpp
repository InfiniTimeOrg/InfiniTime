#include "components/datetime/DateTimeController.h"
#include <libraries/log/nrf_log.h>
#include <systemtask/SystemTask.h>
#include <hal/nrf_rtc.h>
#include "nrf_assert.h"

using namespace Pinetime::Controllers;

namespace {
  constexpr const char* const DaysStringShort[] = {"--", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
  constexpr const char* const DaysStringShortLow[] = {"--", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  constexpr const char* const DaysString[] = {"--", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY"};
  constexpr const char* const DaysStringLow[] = {"--", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
  constexpr const char* const DaysStringShortEs[] = {"--", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB", "DOM"};
  constexpr const char* const DaysStringShortLowEs[] = {"--", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab", "Dom"};
  constexpr const char* const DaysStringEs[] = {"--", "LUNES", "MARTES", "MIERCOLES", "JUEVES", "VIERNES", "SABADO", "DOMINGO"};
  constexpr const char* const DaysStringLowEs[] = {"--", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado", "Domingo"};
  constexpr const char* const DaysStringShortPt[] = {"--", "SEG", "TER", "QUA", "QUI", "SEX", "SAB", "DOM"};
  constexpr const char* const DaysStringShortLowPt[] = {"--", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab", "Dom"};
  constexpr const char* const DaysStringPt[] = {"--", "SEGUNDA", "TERCA", "QUARTA", "QUINTA", "SEXTA", "SABADO", "DOMINGO"};
  constexpr const char* const DaysStringLowPt[] = {"--", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado", "Domingo"};
  constexpr const char* const DaysStringShortRu[] = {"--", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ", "ВС"};
  constexpr const char* const DaysStringShortLowRu[] = {"--", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
  constexpr const char* const DaysStringRu[] = {"--", "ПОНЕДЕЛЬНИК", "ВТОРНИК", "СРЕДА", "ЧЕТВЕРГ", "ПЯТНИЦА", "СУББОТА", "ВОСКРЕСЕНЬЕ"};
  constexpr const char* const DaysStringLowRu[] = {"--", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
  constexpr const char* const DaysStringShortFr[] = {"--", "LUN", "MAR", "MER", "JEU", "VEN", "SAM", "DIM"};
  constexpr const char* const DaysStringShortLowFr[] = {"--", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam", "Dim"};
  constexpr const char* const DaysStringFr[] = {"--", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI", "DIMANCHE"};
  constexpr const char* const DaysStringLowFr[] = {"--", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"};
  constexpr const char* const DaysStringShortDe[] = {"--", "MO", "DI", "MI", "DO", "FR", "SA", "SO"};
  constexpr const char* const DaysStringShortLowDe[] = {"--", "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
  constexpr const char* const DaysStringDe[] = {"--", "MONTAG", "DIENSTAG", "MITTWOCH", "DONNERSTAG", "FREITAG", "SAMSTAG", "SONNTAG"};
  constexpr const char* const DaysStringLowDe[] = {"--", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag", "Sonntag"};
  constexpr const char* const DaysStringShortIt[] = {"--", "LUN", "MAR", "MER", "GIO", "VEN", "SAB", "DOM"};
  constexpr const char* const DaysStringShortLowIt[] = {"--", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab", "Dom"};
  constexpr const char* const DaysStringIt[] = {"--", "LUNEDI", "MARTEDI", "MERCOLEDI", "GIOVEDI", "VENERDI", "SABATO", "DOMENICA"};
  constexpr const char* const DaysStringLowIt[] = {"--", "Lunedi", "Martedi", "Mercoledi", "Giovedi", "Venerdi", "Sabato", "Domenica"};
  constexpr const char* const DaysStringShortTr[] = {"--", "PZT", "SAL", "CAR", "PER", "CUM", "CMT", "PAZ"};
  constexpr const char* const DaysStringShortLowTr[] = {"--", "Pzt", "Sal", "Car", "Per", "Cum", "Cmt", "Paz"};
  constexpr const char* const DaysStringTr[] = {"--", "PAZARTESI", "SALI", "CARSAMBA", "PERSEMBE", "CUMA", "CUMARTESI", "PAZAR"};
  constexpr const char* const DaysStringLowTr[] = {"--", "Pazartesi", "Sali", "Carsamba", "Persembe", "Cuma", "Cumartesi", "Pazar"};
  constexpr const char* const DaysStringShortPl[] = {"--", "PON", "WTO", "SRO", "CZW", "PIA", "SOB", "NIE"};
  constexpr const char* const DaysStringShortLowPl[] = {"--", "Pon", "Wto", "Sro", "Czw", "Pia", "Sob", "Nie"};
  constexpr const char* const DaysStringPl[] = {"--", "PONIEDZIALEK", "WTOREK", "SRODA", "CZWARTEK", "PIATEK", "SOBOTA", "NIEDZIELA"};
  constexpr const char* const DaysStringLowPl[] = {"--", "Poniedzialek", "Wtorek", "Sroda", "Czwartek", "Piatek", "Sobota", "Niedziela"};
  constexpr const char* const DaysStringShortCa[] = {"--", "DLL", "DMT", "DMC", "DJS", "DVN", "DSB", "DGN"};
  constexpr const char* const DaysStringShortLowCa[] = {"--", "Dll", "Dmt", "Dmc", "Djs", "Dvn", "Dsb", "Dgn"};
  constexpr const char* const DaysStringCa[] = {"--", "DILLUNS", "DIMARTS", "DIMECRES", "DIJOUS", "DIVENDRES", "DISSABTE", "DIUMENGE"};
  constexpr const char* const DaysStringLowCa[] = {"--", "Dilluns", "Dimarts", "Dimecres", "Dijous", "Divendres", "Dissabte", "Diumenge"};
  constexpr const char* const DaysStringShortEu[] = {"--", "AL", "AR", "AZ", "OG", "OR", "LR", "IG"};
  constexpr const char* const DaysStringShortLowEu[] = {"--", "Al", "Ar", "Az", "Og", "Or", "Lr", "Ig"};
  constexpr const char* const DaysStringEu[] = {"--", "ASTELEHENA", "ASTEARTEA", "ASTEAZKENA", "OSTEGUNA", "OSTIRALA", "LARUNBATA", "IGANDEA"};
  constexpr const char* const DaysStringLowEu[] = {"--", "Astelehena", "Asteartea", "Asteazkena", "Osteguna", "Ostirala", "Larunbata", "Igandea"};
  constexpr const char* const MonthsString[] = {"--", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  constexpr const char* const MonthsStringLow[] =
    {"--", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  constexpr const char* const MonthsStringEs[] = {"--", "ENE", "FEB", "MAR", "ABR", "MAY", "JUN", "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"};
  constexpr const char* const MonthsStringLowEs[] =
    {"--", "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
  constexpr const char* const MonthsStringPt[] = {"--", "JAN", "FEV", "MAR", "ABR", "MAI", "JUN", "JUL", "AGO", "SET", "OUT", "NOV", "DEZ"};
  constexpr const char* const MonthsStringLowPt[] =
    {"--", "Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"};
  constexpr const char* const MonthsStringRu[] = {"--", "ЯНВ", "ФЕВ", "МАР", "АПР", "МАЙ", "ИЮН", "ИЮЛ", "АВГ", "СЕН", "ОКТ", "НОЯ", "ДЕК"};
  constexpr const char* const MonthsStringLowRu[] =
    {"--", "Янв", "Фев", "Мар", "Апр", "Май", "Июн", "Июл", "Авг", "Сен", "Окт", "Ноя", "Дек"};
  constexpr const char* const MonthsStringFr[] = {"--", "JAN", "FEV", "MAR", "AVR", "MAI", "JUN", "JUL", "AOU", "SEP", "OCT", "NOV", "DEC"};
  constexpr const char* const MonthsStringLowFr[] =
    {"--", "Jan", "Fev", "Mar", "Avr", "Mai", "Jun", "Jul", "Aou", "Sep", "Oct", "Nov", "Dec"};
  constexpr const char* const MonthsStringDe[] = {"--", "JAN", "FEB", "MAR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ"};
  constexpr const char* const MonthsStringLowDe[] =
    {"--", "Jan", "Feb", "Mar", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};
  constexpr const char* const MonthsStringIt[] = {"--", "GEN", "FEB", "MAR", "APR", "MAG", "GIU", "LUG", "AGO", "SET", "OTT", "NOV", "DIC"};
  constexpr const char* const MonthsStringLowIt[] =
    {"--", "Gen", "Feb", "Mar", "Apr", "Mag", "Giu", "Lug", "Ago", "Set", "Ott", "Nov", "Dic"};
  constexpr const char* const MonthsStringTr[] = {"--", "OCA", "SUB", "MAR", "NIS", "MAY", "HAZ", "TEM", "AGU", "EYL", "EKI", "KAS", "ARA"};
  constexpr const char* const MonthsStringLowTr[] =
    {"--", "Oca", "Sub", "Mar", "Nis", "May", "Haz", "Tem", "Agu", "Eyl", "Eki", "Kas", "Ara"};
  constexpr const char* const MonthsStringPl[] = {"--", "STY", "LUT", "MAR", "KWI", "MAJ", "CZE", "LIP", "SIE", "WRZ", "PAZ", "LIS", "GRU"};
  constexpr const char* const MonthsStringLowPl[] =
    {"--", "Sty", "Lut", "Mar", "Kwi", "Maj", "Cze", "Lip", "Sie", "Wrz", "Paz", "Lis", "Gru"};
  constexpr const char* const MonthsStringCa[] = {"--", "GEN", "FEB", "MAR", "ABR", "MAI", "JUN", "JUL", "AGO", "SET", "OCT", "NOV", "DES"};
  constexpr const char* const MonthsStringLowCa[] =
    {"--", "Gen", "Feb", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Oct", "Nov", "Des"};
  constexpr const char* const MonthsStringEu[] = {"--", "URT", "OTS", "MAR", "API", "MAI", "EKA", "UZT", "ABU", "IRA", "URR", "AZA", "ABE"};
  constexpr const char* const MonthsStringLowEu[] =
    {"--", "Urt", "Ots", "Mar", "Api", "Mai", "Eka", "Uzt", "Abu", "Ira", "Urr", "Aza", "Abe"};

  constexpr int compileTimeAtoi(const char* str) {
    int result = 0;
    while (*str >= '0' && *str <= '9') {
      result = result * 10 + *str - '0';
      str++;
    }
    return result;
  }
}

DateTime::DateTime(Controllers::Settings& settingsController) : settingsController {settingsController} {
  mutex = xSemaphoreCreateMutex();
  ASSERT(mutex != nullptr);
  xSemaphoreGive(mutex);

  // __DATE__ is a string of the format "MMM DD YYYY", so an offset of 7 gives the start of the year
  SetTime(compileTimeAtoi(&__DATE__[7]), 1, 1, 0, 0, 0);
}

void DateTime::SetCurrentTime(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  this->currentDateTime = t;
  UpdateTime(previousSystickCounter, true); // Update internal state without updating the time
  xSemaphoreGive(mutex);
}

void DateTime::SetTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  std::tm tm = {
    /* .tm_sec  = */ second,
    /* .tm_min  = */ minute,
    /* .tm_hour = */ hour,
    /* .tm_mday = */ day,
    /* .tm_mon  = */ month - 1,
    /* .tm_year = */ year - 1900,
  };

  NRF_LOG_INFO("%d %d %d ", day, month, year);
  NRF_LOG_INFO("%d %d %d ", hour, minute, second);

  tm.tm_isdst = -1; // Use DST value from local time zone

  xSemaphoreTake(mutex, portMAX_DELAY);
  currentDateTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
  UpdateTime(previousSystickCounter, true);
  xSemaphoreGive(mutex);

  if (systemTask != nullptr) {
    systemTask->PushMessage(System::Messages::OnNewTime);
  }
}

void DateTime::SetTimeZone(int8_t timezone, int8_t dst) {
  tzOffset = timezone;
  dstOffset = dst;
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> DateTime::CurrentDateTime() {
  xSemaphoreTake(mutex, portMAX_DELAY);
  UpdateTime(nrf_rtc_counter_get(portNRF_RTC_REG), false);
  xSemaphoreGive(mutex);
  return currentDateTime;
}

void DateTime::UpdateTime(uint32_t systickCounter, bool forceUpdate) {
  // Handle systick counter overflow
  uint32_t systickDelta = 0;
  if (systickCounter < previousSystickCounter) {
    systickDelta = static_cast<uint32_t>(portNRF_RTC_MAXTICKS) - previousSystickCounter;
    systickDelta += systickCounter + 1;
  } else {
    systickDelta = systickCounter - previousSystickCounter;
  }

  auto correctedDelta = systickDelta / configTICK_RATE_HZ;
  // If a second hasn't passed, there is nothing to do
  // If the time has been changed, set forceUpdate to trigger internal state updates
  if (correctedDelta == 0 && !forceUpdate) {
    return;
  }
  auto rest = systickDelta % configTICK_RATE_HZ;
  if (systickCounter >= rest) {
    previousSystickCounter = systickCounter - rest;
  } else {
    previousSystickCounter = static_cast<uint32_t>(portNRF_RTC_MAXTICKS) - (rest - systickCounter - 1);
  }

  currentDateTime += std::chrono::seconds(correctedDelta);
  uptime += std::chrono::seconds(correctedDelta);

  std::time_t currentTime = std::chrono::system_clock::to_time_t(currentDateTime);
  localTime = *std::localtime(&currentTime);

  auto minute = Minutes();
  auto hour = Hours();

  if (minute == 0 && !isHourAlreadyNotified) {
    isHourAlreadyNotified = true;
    if (systemTask != nullptr) {
      systemTask->PushMessage(System::Messages::OnNewHour);
    }
  } else if (minute != 0) {
    isHourAlreadyNotified = false;
  }

  if ((minute == 0 || minute == 30) && !isHalfHourAlreadyNotified) {
    isHalfHourAlreadyNotified = true;
    if (systemTask != nullptr) {
      systemTask->PushMessage(System::Messages::OnNewHalfHour);
    }
  } else if (minute != 0 && minute != 30) {
    isHalfHourAlreadyNotified = false;
  }

  // Notify new day to SystemTask
  if (hour == 0 and not isMidnightAlreadyNotified) {
    isMidnightAlreadyNotified = true;
    if (systemTask != nullptr)
      systemTask->PushMessage(System::Messages::OnNewDay);
  } else if (hour != 0) {
    isMidnightAlreadyNotified = false;
  }
}

const char* DateTime::MonthShortToString() const {
  const auto month = static_cast<uint8_t>(Month());
  switch (settingsController.GetLanguage()) {
    case Controllers::Settings::Language::Spanish:
      return MonthsStringEs[month];
    case Controllers::Settings::Language::Portuguese:
      return MonthsStringPt[month];
    case Controllers::Settings::Language::Russian:
      return MonthsStringRu[month];
    case Controllers::Settings::Language::French:
      return MonthsStringFr[month];
    case Controllers::Settings::Language::German:
      return MonthsStringDe[month];
    case Controllers::Settings::Language::Italian:
      return MonthsStringIt[month];
    case Controllers::Settings::Language::Turkish:
      return MonthsStringTr[month];
    case Controllers::Settings::Language::Polish:
      return MonthsStringPl[month];
    case Controllers::Settings::Language::Catalan:
      return MonthsStringCa[month];
    case Controllers::Settings::Language::Basque:
      return MonthsStringEu[month];
    default:
      return MonthsString[month];
  }
}

const char* DateTime::DayOfWeekShortToString() const {
  const auto day = static_cast<uint8_t>(DayOfWeek());
  switch (settingsController.GetLanguage()) {
    case Controllers::Settings::Language::Spanish:
      return DaysStringShortEs[day];
    case Controllers::Settings::Language::Portuguese:
      return DaysStringShortPt[day];
    case Controllers::Settings::Language::Russian:
      return DaysStringShortRu[day];
    case Controllers::Settings::Language::French:
      return DaysStringShortFr[day];
    case Controllers::Settings::Language::German:
      return DaysStringShortDe[day];
    case Controllers::Settings::Language::Italian:
      return DaysStringShortIt[day];
    case Controllers::Settings::Language::Turkish:
      return DaysStringShortTr[day];
    case Controllers::Settings::Language::Polish:
      return DaysStringShortPl[day];
    case Controllers::Settings::Language::Catalan:
      return DaysStringShortCa[day];
    case Controllers::Settings::Language::Basque:
      return DaysStringShortEu[day];
    default:
      return DaysStringShort[day];
  }
}

const char* DateTime::DayOfWeekToString() const {
  const auto day = static_cast<uint8_t>(DayOfWeek());
  switch (settingsController.GetLanguage()) {
    case Controllers::Settings::Language::Spanish:
      return DaysStringEs[day];
    case Controllers::Settings::Language::Portuguese:
      return DaysStringPt[day];
    case Controllers::Settings::Language::Russian:
      return DaysStringRu[day];
    case Controllers::Settings::Language::French:
      return DaysStringFr[day];
    case Controllers::Settings::Language::German:
      return DaysStringDe[day];
    case Controllers::Settings::Language::Italian:
      return DaysStringIt[day];
    case Controllers::Settings::Language::Turkish:
      return DaysStringTr[day];
    case Controllers::Settings::Language::Polish:
      return DaysStringPl[day];
    case Controllers::Settings::Language::Catalan:
      return DaysStringCa[day];
    case Controllers::Settings::Language::Basque:
      return DaysStringEu[day];
    default:
      return DaysString[day];
  }
}

const char* DateTime::MonthShortToStringLow(Months month) {
  return MonthShortToStringLow(month, Controllers::Settings::Language::English);
}

const char* DateTime::MonthShortToStringLow(Months month, Controllers::Settings::Language language) {
  const auto monthIndex = static_cast<uint8_t>(month);
  switch (language) {
    case Controllers::Settings::Language::Spanish:
      return MonthsStringLowEs[monthIndex];
    case Controllers::Settings::Language::Portuguese:
      return MonthsStringLowPt[monthIndex];
    case Controllers::Settings::Language::Russian:
      return MonthsStringLowRu[monthIndex];
    case Controllers::Settings::Language::French:
      return MonthsStringLowFr[monthIndex];
    case Controllers::Settings::Language::German:
      return MonthsStringLowDe[monthIndex];
    case Controllers::Settings::Language::Italian:
      return MonthsStringLowIt[monthIndex];
    case Controllers::Settings::Language::Turkish:
      return MonthsStringLowTr[monthIndex];
    case Controllers::Settings::Language::Polish:
      return MonthsStringLowPl[monthIndex];
    case Controllers::Settings::Language::Catalan:
      return MonthsStringLowCa[monthIndex];
    case Controllers::Settings::Language::Basque:
      return MonthsStringLowEu[monthIndex];
    default:
      return MonthsStringLow[monthIndex];
  }
}

const char* DateTime::DayOfWeekShortToStringLow(Days day) {
  return DayOfWeekShortToStringLow(day, Controllers::Settings::Language::English);
}

const char* DateTime::DayOfWeekShortToStringLow(Days day, Controllers::Settings::Language language) {
  const auto dayIndex = static_cast<uint8_t>(day);
  switch (language) {
    case Controllers::Settings::Language::Spanish:
      return DaysStringShortLowEs[dayIndex];
    case Controllers::Settings::Language::Portuguese:
      return DaysStringShortLowPt[dayIndex];
    case Controllers::Settings::Language::Russian:
      return DaysStringShortLowRu[dayIndex];
    case Controllers::Settings::Language::French:
      return DaysStringShortLowFr[dayIndex];
    case Controllers::Settings::Language::German:
      return DaysStringShortLowDe[dayIndex];
    case Controllers::Settings::Language::Italian:
      return DaysStringShortLowIt[dayIndex];
    case Controllers::Settings::Language::Turkish:
      return DaysStringShortLowTr[dayIndex];
    case Controllers::Settings::Language::Polish:
      return DaysStringShortLowPl[dayIndex];
    case Controllers::Settings::Language::Catalan:
      return DaysStringShortLowCa[dayIndex];
    case Controllers::Settings::Language::Basque:
      return DaysStringShortLowEu[dayIndex];
    default:
      return DaysStringShortLow[dayIndex];
  }
}

const char* DateTime::DayOfWeekToStringLow(Days day) {
  return DayOfWeekToStringLow(day, Controllers::Settings::Language::English);
}

const char* DateTime::DayOfWeekToStringLow(Days day, Controllers::Settings::Language language) {
  const auto dayIndex = static_cast<uint8_t>(day);
  switch (language) {
    case Controllers::Settings::Language::Spanish:
      return DaysStringLowEs[dayIndex];
    case Controllers::Settings::Language::Portuguese:
      return DaysStringLowPt[dayIndex];
    case Controllers::Settings::Language::Russian:
      return DaysStringLowRu[dayIndex];
    case Controllers::Settings::Language::French:
      return DaysStringLowFr[dayIndex];
    case Controllers::Settings::Language::German:
      return DaysStringLowDe[dayIndex];
    case Controllers::Settings::Language::Italian:
      return DaysStringLowIt[dayIndex];
    case Controllers::Settings::Language::Turkish:
      return DaysStringLowTr[dayIndex];
    case Controllers::Settings::Language::Polish:
      return DaysStringLowPl[dayIndex];
    case Controllers::Settings::Language::Catalan:
      return DaysStringLowCa[dayIndex];
    case Controllers::Settings::Language::Basque:
      return DaysStringLowEu[dayIndex];
    default:
      return DaysStringLow[dayIndex];
  }
}

void DateTime::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

using ClockType = Pinetime::Controllers::Settings::ClockType;

std::string DateTime::FormattedTime() {
  auto hour = Hours();
  auto minute = Minutes();
  // Return time as a string in 12- or 24-hour format
  char buff[9];
  if (settingsController.GetClockType() == ClockType::H12) {
    uint8_t hour12;
    const char* amPmStr;
    if (hour < 12) {
      hour12 = (hour == 0) ? 12 : hour;
      amPmStr = "AM";
    } else {
      hour12 = (hour == 12) ? 12 : hour - 12;
      amPmStr = "PM";
    }
    snprintf(buff, sizeof(buff), "%i:%02i %s", hour12, minute, amPmStr);
  } else {
    snprintf(buff, sizeof(buff), "%02i:%02i", hour, minute);
  }
  return std::string(buff);
}
