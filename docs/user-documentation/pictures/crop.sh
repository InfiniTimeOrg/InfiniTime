#!/bin/bash
#convert
for image in *.png; do
        convert  "$image" -crop 240x240+0+29  "$image"
done
exit 0 
