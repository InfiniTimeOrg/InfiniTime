#include "./lv_i18n.h"

////////////////////////////////////////////////////////////////////////////////
// Define plural operands
// http://unicode.org/reports/tr35/tr35-numbers.html#Operands

// Integer version, simplified

#define UNUSED(x) (void) (x)

static inline uint32_t op_n(int32_t val) {
  return (uint32_t) (val < 0 ? -val : val);
}
static inline uint32_t op_i(uint32_t val) {
  return val;
}
// always zero, when decimal part not exists.
static inline uint32_t op_v(uint32_t val) {
  UNUSED(val);
  return 0;
}
static inline uint32_t op_w(uint32_t val) {
  UNUSED(val);
  return 0;
}
static inline uint32_t op_f(uint32_t val) {
  UNUSED(val);
  return 0;
}
static inline uint32_t op_t(uint32_t val) {
  UNUSED(val);
  return 0;
}

static lv_i18n_phrase_t en_singulars[] = {
  {"Reset FW", "Reset"},
  {NULL, NULL} // End mark
};

static uint8_t en_plural_fn(int32_t num) {
  uint32_t n = op_n(num);
  UNUSED(n);
  uint32_t i = op_i(n);
  UNUSED(i);
  uint32_t v = op_v(n);
  UNUSED(v);

  if ((i == 1 && v == 0))
    return LV_I18N_PLURAL_TYPE_ONE;
  return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t en_lang = {.locale_name = "en",
                                       .singulars = en_singulars,

                                       .locale_plural_fn = en_plural_fn};

static lv_i18n_phrase_t de_singulars[] = {
  {"Display", "Anzeige"},
  {"Wake Up", "Aufwecken"},
  {"Display timeout", "Anzeigedauer"},
  {"Bluetooth", "Bluetooth"},
  {" Enabled", " Aktiv"},
  {" Disabled", " Inaktiv"},
  {"Chimes", "Glockenschlag"},
  {"Off", "Aus"},
  {"Every hour", "Stuendlich"},
  {"Every 30 mins", "Halbstuendl."},
  {"Time format", "Zeitformat"},
  {"Watch face", "Ziffernblatt"},
  {"Steps", "Schritte"},
  {"Set date", "Datum setzen"},
  {"Set time", "Zeit setzen"},
  {"Battery", "Batterie"},
  {"Shake Calib.", "Schuettel Kal."},
  {"Firmware", "Firmware"},
  {"About", "Ueber"},
  {"Set current date", "Datum setzen"},
  {"Set", "Setzen"},
  {"Set current time", "Zeit setzen"},
  {"Wake Sensitivity", "Empfindlichkeit"},
  {"Calibrate", "Kalibrieren"},
  {"Shake!", "Schuettel!"},
  {"Ready!", "Fertig!"},
  {"Daily steps goal", "Taegliches\nSchrittziel"},
  {"12-hour", "12-Stunden"},
  {"24-hour", "24-Stunden"},
  {"Single Tap", "Einfachtap"},
  {"Double Tap", "Doppeltap"},
  {"Raise Wrist", "Hand heben"},
  {"Shake Wake", "Schuetteln"},
  {"--", "--"},
  {"MON", "Mo."},
  {"TUE", "Di."},
  {"WED", "Mi."},
  {"THU", "Do."},
  {"FRI", "Fr."},
  {"SAT", "Sa."},
  {"SUN", "So."},
  {"Mon", "Mo."},
  {"Tue", "Di."},
  {"Wed", "Mi."},
  {"Thu", "Do."},
  {"Fri", "Fr."},
  {"Sat", "Sa."},
  {"Sun", "So."},
  {"JAN", "JAN"},
  {"FEB", "FEB"},
  {"MAR", "MRZ"},
  {"APR", "APR"},
  {"MAY", "MAI"},
  {"JUN", "JUN"},
  {"JUL", "JUL"},
  {"AUG", "AUG"},
  {"SEP", "SEP"},
  {"OCT", "OKT"},
  {"NOV", "NOV"},
  {"DEC", "DEZ"},
  {"Jan", "Jan"},
  {"Feb", "Feb"},
  {"Mar", "Mrz"},
  {"Apr", "Apr"},
  {"May", "Mai"},
  {"Jun", "Jun"},
  {"Jul", "Jul"},
  {"Aug", "Aug"},
  {"Sep", "Sep"},
  {"Oct", "Okt"},
  {"Nov", "Nov"},
  {"Dec", "Dez"},
  {"Reading Battery status", "Lese Batteriestatus"},
  {"volts", "Volt"},
  {"Charging", "Lade"},
  {"Fully charged", "Geladen"},
  {"Battery low", "Batteriespannung niedrig"},
  {"Discharging", "Entlade"},
  {"Firmware update", "Firmware Update"},
  {"Waiting...", "Warte..."},
  {"Notification", "Benachrichtigung"},
  {"No notification to display", "Keine Benachrichtigung"},
  {"Incoming call from", "Anruf von:"},
  {"Goal", "Ziel"},
  {"Reset", "Reset"},
  {"Trip", "Trip"},
  {"Start", "Start"},
  {"Pause", "Pause"},
  {"Waiting for", "Warte auf"},
  {"track information..", "Titelinformation"},
  {"Time to\nalarm:\n%2lu Days\n%2lu Hours\n%2lu Minutes\n%2lu Seconds",
   "Zeit bis\nAlarm:\n%2lu Tage\n%2lu Stunden\n%2lu Minuten\n%2lu Sekunden"},
  {"Alarm\nis not\nset.", "Alarm\nist nicht\ngesetzt."},
  {"ONCE", "EINML"},
  {"DAILY", "TAEGL"},
  {"MON-FRI", "MO-FR"},
  {"Version", "Version"},
  {"ShortRef", "ShortRef"},
  {"You have already\n#00ff00 validated# this firmware#", "Du hast diese\nFirmware bereits\n#00ff00 validiert#"},
  {"Please #00ff00 Validate# this version or\n#ff0000 Reset# to rollback to the previous version.",
   "Bitte #00ff00 validiere# diese Version oder\nsetze sie #ff0000 zurueck# auf die vorherige Version."},
  {"Validate", "Valide"},
  {"Not enough data,\nplease wait...", "Nicht genug Daten,\nbitte warten..."},
  {"No touch detected", "Keine Beruehrung erkannt"},
  {"Measuring...", "Messe..."},
  {"Stopped", "Gestoppt"},
  {"Heart rate BPM", "Puls BPM"},
  {"Stop", "Stopp"},
  {"Steps ---", "Schritte ---"},
  {"Artist Name", "Künstler"},
  {"This is a very long getTrack name", "Das ist ein langer Titel"},
  {"Digital face", "Digitaluhr"},
  {"Terminal", "Konsole"},
  {"Analog face", "Analoguhr"},
  {"Reset FW", "Zurueck"},
  {NULL, NULL} // End mark
};

static uint8_t de_plural_fn(int32_t num) {
  uint32_t n = op_n(num);
  UNUSED(n);
  uint32_t i = op_i(n);
  UNUSED(i);
  uint32_t v = op_v(n);
  UNUSED(v);

  if ((i == 1 && v == 0))
    return LV_I18N_PLURAL_TYPE_ONE;
  return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t de_lang = {.locale_name = "de",
                                       .singulars = de_singulars,

                                       .locale_plural_fn = de_plural_fn};

const lv_i18n_language_pack_t lv_i18n_language_pack[] = {
  &en_lang,
  &de_lang,
  NULL // End mark
};

////////////////////////////////////////////////////////////////////////////////

// Internal state
static const lv_i18n_language_pack_t* current_lang_pack;
static const lv_i18n_lang_t* current_lang;

/**
 * Reset internal state. For testing.
 */
void __lv_i18n_reset(void) {
  current_lang_pack = NULL;
  current_lang = NULL;
}

/**
 * Set the languages for internationalization
 * @param langs pointer to the array of languages. (Last element has to be `NULL`)
 */
int lv_i18n_init(const lv_i18n_language_pack_t* langs) {
  if (langs == NULL)
    return -1;
  if (langs[0] == NULL)
    return -1;

  current_lang_pack = langs;
  current_lang = langs[0]; /*Automatically select the first language*/
  return 0;
}

/**
 * Change the localization (language)
 * @param l_name name of the translation locale to use. E.g. "en-GB"
 */
int lv_i18n_set_locale(const char* l_name) {
  if (current_lang_pack == NULL)
    return -1;

  uint16_t i;

  for (i = 0; current_lang_pack[i] != NULL; i++) {
    // Found -> finish
    if (strcmp(current_lang_pack[i]->locale_name, l_name) == 0) {
      current_lang = current_lang_pack[i];
      return 0;
    }
  }

  return -1;
}

static const char* __lv_i18n_get_text_core(lv_i18n_phrase_t* trans, const char* msg_id) {
  uint16_t i;
  for (i = 0; trans[i].msg_id != NULL; i++) {
    if (strcmp(trans[i].msg_id, msg_id) == 0) {
      /*The msg_id has found. Check the translation*/
      if (trans[i].translation)
        return trans[i].translation;
    }
  }

  return NULL;
}

/**
 * Get the translation from a message ID
 * @param msg_id message ID
 * @return the translation of `msg_id` on the set local
 */
const char* lv_i18n_get_text(const char* msg_id) {
  if (current_lang == NULL)
    return msg_id;

  const lv_i18n_lang_t* lang = current_lang;
  const void* txt;

  // Search in current locale
  if (lang->singulars != NULL) {
    txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
    if (txt != NULL)
      return txt;
  }

  // Try to fallback
  if (lang == current_lang_pack[0])
    return msg_id;
  lang = current_lang_pack[0];

  // Repeat search for default locale
  if (lang->singulars != NULL) {
    txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
    if (txt != NULL)
      return txt;
  }

  return msg_id;
}

/**
 * Get the translation from a message ID and apply the language's plural rule to get correct form
 * @param msg_id message ID
 * @param num an integer to select the correct plural form
 * @return the translation of `msg_id` on the set local
 */
const char* lv_i18n_get_text_plural(const char* msg_id, int32_t num) {
  if (current_lang == NULL)
    return msg_id;

  const lv_i18n_lang_t* lang = current_lang;
  const void* txt;
  lv_i18n_plural_type_t ptype;

  // Search in current locale
  if (lang->locale_plural_fn != NULL) {
    ptype = lang->locale_plural_fn(num);

    if (lang->plurals[ptype] != NULL) {
      txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
      if (txt != NULL)
        return txt;
    }
  }

  // Try to fallback
  if (lang == current_lang_pack[0])
    return msg_id;
  lang = current_lang_pack[0];

  // Repeat search for default locale
  if (lang->locale_plural_fn != NULL) {
    ptype = lang->locale_plural_fn(num);

    if (lang->plurals[ptype] != NULL) {
      txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
      if (txt != NULL)
        return txt;
    }
  }

  return msg_id;
}

/**
 * Get the name of the currently used locale.
 * @return name of the currently used locale. E.g. "en-GB"
 */
const char* lv_i18n_get_current_locale(void) {
  if (!current_lang)
    return NULL;
  return current_lang->locale_name;
}
