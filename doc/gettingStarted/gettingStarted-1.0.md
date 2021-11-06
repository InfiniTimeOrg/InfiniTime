# Getting started with InfiniTime 1.0.0

On April 22 2021, InfiniTime and Pine64 [announced the release of InfiniTime 1.0.0](https://www.pine64.org/2021/04/22/its-time-infinitime-1-0/) and the availability of PineTime smartwatches as *enthusiast grade end-user product*. This page aims to guide you with your first step with your new PineTime.

It is highly recommended to update the firmware to the latest version when you receive your watch and when a new InfiniTime version is released. More information on updating the firmware [here](/doc/gettingStarted/updating-software.md).

## InfiniTime 1.0.0 quick user guide

### Setting the time

By default, InfiniTime starts on the digital watchface. It'll probably display the epoch time (1 Jan 1970, 00:00).

You can sync the time using companion apps.

 - Gadgetbridge automatically synchronizes the time when you connect it to your watch. More information on Gadgetbridge [here](/doc/gettingStarted/ota-gadgetbridge.md)
 - You can use NRFConnect to [sync the time](/doc/gettingStarted/time-nrfconnect.md)
 - Sync the time with your browser https://hubmartin.github.io/WebBLEWatch/
 - Since InfiniTime 1.7.0, you can set the time in the settings without needing to use a companion app

## Navigation in the menu

![Quick actions](quickactions.jpg)
![Settings](settings.jpg)
![Application menu](appmenu.jpg)

 - Swipe **down** to display the notification panel. Notification sent by your companion app will be displayed in this panel.
 - Swipe **up** to display the application menus. Apps (stopwatch, music, step, games,...) can be started from this menu.
 - Swipe **right** to display the Quick Actions menu. This menu allows you to
    - Set the brightness of the display
    - Start the **flashlight** app
    - Enable/disable vibrations on notifications (Do Not Disturb mode)
    - Enter the **settings** menu
       - Swipe up and down to see all options

### Bootloader

Most of the time, the bootloader just runs without your intervention (update and load the firmware).

However, you can enable 2 functionalities using the push button:

 - Push the button until the pine cone is drawn in **blue** to force the rollback of the previous version of the firmware, even if you've already validated the updated one
 - Push the button until the pine cone is drawn in **red** to load the recovery firmware. This recovery firmware only provides BLE connectivity and OTA functionality.

More info about the bootloader in [its project page](https://github.com/JF002/pinetime-mcuboot-bootloader/blob/master/README.md).
