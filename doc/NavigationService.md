# Navigation Service
## Introduction
The navigation ble service provides 4 characteristics to allow the the watch to display navigation instructions from a companion application.  The intended purpose is when performing some outdoor activities, for example running or cycling.

The 4 characteristics are:
flag (string) - Upcoming icon name
narrative (string) - Textual description of instruction
manDist (string) - Manouvre Distance, the distance to the upcoming change
progress (uint8) - Percent complete of total route, value 0-100

## Flags
All included icons are from pure-maps, which provides the actual routing from the client. The icon names ultimately come from the mapbox project "direction-icons", See https://github.com/rinigus/pure-maps/tree/master/qml/icons/navigation .

## Narrative
This is a client supplied string describing the upcoming instruction such as "At the roundabout take the first exit".

## Man Dist
This is a short string describing the distance to the upcoming instruction such as "50 m".

## Progress
The percent complete in a uint8.  The watch displays this as an overall progress in a progress bar.
