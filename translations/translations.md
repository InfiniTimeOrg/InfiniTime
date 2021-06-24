InfiniTime uses [lv_i18n](https://github.com/lvgl/lv_i18n) for translations.

When translations need updating (typically after new features are added with user-readable text): 
1. Ensure `_("some_key")` or `_p("some_key")` is used instead of directly using a string for user text.
2. Run `translations/extract.sh` to extract all keys into the translation files in this folder.
3. Update the translation files as appropriate.
4. Run `translations/compile.sh` to generate header and implementation files containing all the translations.
5. Commit the result.

To add a new language, add a new .yml file with the ISO locale code as the only key, e.g:
```
es-ES: ~
```
then run `translations/extract.sh` to get the keys added to the file

