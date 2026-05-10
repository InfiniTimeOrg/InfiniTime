# Localization

InfiniTime has a small static localization layer for user interface strings. It is designed for firmware use:

- strings are addressed by `Pinetime::Applications::Localization::StringId`
- the selected language is `Pinetime::Applications::Localization::Language`
- translations are stored in fixed `constexpr` tables in `src/displayapp/localization/Localization.cpp`
- callers use `Translate(language, stringId)` to get a `const char*`
- no dynamic allocation is used by the localization layer

The currently supported languages are English and Spanish.

Spanish uses accented characters such as `á`, `é`, `í`, `ó`, `ú`, `ñ`, `¿` and `¡`. The built-in UI font range is configured in:

```text
src/displayapp/fonts/fonts.json
```

`jetbrains_mono_bold_20` includes the Latin-1 range so normal UI labels can render Spanish text correctly on device.

## Where Strings Are Defined

String IDs and supported languages are declared in:

```text
src/displayapp/localization/Localization.h
```

English and Spanish string tables are defined in:

```text
src/displayapp/localization/Localization.cpp
```

The settings controller stores the selected language in the persisted settings data. English is the default.

## Adding a New Language

To add a language:

1. Add a value to the `Language` enum before `Count`.
2. Add a translation table in `Localization.cpp`.
3. Add that table to the `translations` array in the same enum order.
4. Add the language to the selector options in `src/displayapp/screens/settings/SettingLanguage.cpp`.
5. Add translations for every existing `StringId`.
6. Make sure any new characters required by the language are present in the active UI font ranges.

The translation tables use `std::array<const char*, StringCount>`. This makes missing entries fail at compile time when the table size no longer matches the number of string IDs.

## Settings Menu Usage

The Settings app stores menu entries as string IDs instead of hardcoded labels. `src/displayapp/screens/List.cpp` translates those IDs with the current language when the list screen is created.

The language selector is implemented in:

```text
src/displayapp/screens/settings/SettingLanguage.cpp
```

It uses the existing `CheckboxList` pattern and saves the selected language through the settings controller.
