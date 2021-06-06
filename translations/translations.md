InfiniTime uses [lv_i18n](https://github.com/lvgl/lv_i18n) for translations.

When translations need updating (typically after new features are added with user-readable text): 
1. Ensure `_("some_key")` or `_p("some_key")` instead of directly using a string.
2. Run `npx lv_i18n extract -s 'src/**/*.+(c|cpp|h|hpp)' -t 'translations/*.yml'` to extract all keys into the translation files in this folder.
3. Update the translation files as appropriate.
4. Run `npx lv_i18n compile -t 'translations/*.yml' -o 'src/lv_i18n'` to generate header and implementation files containing all the translations.
5. Commit the result.


