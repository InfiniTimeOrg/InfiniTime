#! /bin/bash

source $(dirname $0)/../target.sh

gscript=/tmp/init$$.gdb

cat > $gscript <<EOF
target remote localhost:2331
symbol-file outdir/$BOARD/zephyr.elf
# symbol-file ../zephyr/samples/shell/outdir/$BOARD/zephyr.elf
# dir apps/boot/src
# dir libs/bootutil/src
# dir hw/mcu/stm/stm32f4xx/src
b main
# b __reset
# b bootutil_img_validate
# b cmp_rsasig
# b bootutil_verify_sig
# b mbedtls_rsa_public
# b boot_calloc
mon reset 2
layout src
focus cmd
EOF

$gdbexe -x $gscript
rm $gscript
