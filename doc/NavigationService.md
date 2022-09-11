# Navigation Service

## Introduction

The navigation ble service provides 4 characteristics to allow the watch to display navigation instructions from a companion application. This service is intended to be used when performing some outdoor activities, for example running or cycling.

The 4 characteristics are:
flag (string) - Upcoming icon name
narrative (string) - Textual description of instruction
manDist (string) - Manouvre Distance, the distance to the upcoming change
progress (uint8) - Percent complete of total route, value 0-100

## Service

The service UUID is 00010000-78fc-48fe-8e23-433b3a1942d0

## Characteristics

## Flags (UUID 00010001-78fc-48fe-8e23-433b3a1942d0)

All included icons are from pure-maps, which provides the actual routing from the client. The icon names ultimately come from the mapbox project "direction-icons", See https://github.com/rinigus/pure-maps/tree/master/qml/icons/navigation See the end of this document for the full list of supported icon names.

## Narrative (UUID 00010002-78fc-48fe-8e23-433b3a1942d0)

This is a client supplied string describing the upcoming instruction such as "At the roundabout take the first exit".

## Man Dist (UUID 00010003-78fc-48fe-8e23-433b3a1942d0)

This is a short string describing the distance to the upcoming instruction such as "50 m".

## Progress (UUID 00010004-78fc-48fe-8e23-433b3a1942d0)

The percent complete in a uint8. The watch displays this as an overall progress in a progress bar.

## Full icon list

- arrive
- arrive-left
- arrive-right
- arrive-straight
- close
- continue
- continue-left
- continue-right
- continue-slight-left
- continue-slight-right
- continue-straight
- continue-uturn
- depart
- depart-left
- depart-right
- depart-straight
- end-of-road-left
- end-of-road-right
- ferry
- flag
- fork
- fork-left
- fork-right
- fork-slight-left
- fork-slight-right
- fork-straight
- invalid
- invalid-left
- invalid-right
- invalid-slight-left
- invalid-slight-right
- invalid-straight
- invalid-uturn
- merge-left
- merge-right
- merge-slight-left
- merge-slight-right
- merge-straight
- new-name-left
- new-name-right
- new-name-sharp-left
- new-name-sharp-right
- new-name-slight-left
- new-name-slight-right
- new-name-straight
- notification-left
- notification-right
- notification-sharp-left
- notification-sharp-right
- notification-slight-left
- notification-slight-right
- notification-straight
- off-ramp-left
- off-ramp-right
- off-ramp-sharp-left
- off-ramp-sharp-right
- off-ramp-slight-left
- off-ramp-slight-right
- off-ramp-straight
- on-ramp-left
- on-ramp-right
- on-ramp-sharp-left
- on-ramp-sharp-right
- on-ramp-slight-left
- on-ramp-slight-right
- on-ramp-straight
- rotary
- rotary-left
- rotary-right
- rotary-sharp-left
- rotary-sharp-right
- rotary-slight-left
- rotary-slight-right
- rotary-straight
- roundabout
- roundabout-left
- roundabout-right
- roundabout-sharp-left
- roundabout-sharp-right
- roundabout-slight-left
- roundabout-slight-right
- roundabout-straight
- turn-left
- turn-right
- turn-sharp-left
- turn-sharp-right
- turn-slight-left
- turn-slight-right
- turn-stright
- updown
- uturn
