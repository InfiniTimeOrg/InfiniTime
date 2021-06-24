#!/usr/bin/env bash
for f in translations/*.yml
do
 npx lv_i18n extract -s 'src/**/*.+(c|cpp|h|hpp)' -t "$f"
done
