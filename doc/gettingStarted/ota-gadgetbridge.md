# Connecting to Gadgetbridge

Launch Gadgetbridge and tap on the **"+"** button on the bottom right to add a new device:

![Gadgetbridge 0](gadgetbridge0.jpg)

Wait for the scan to complete, your PineTime should be detected:

![Gadgetbridge 1](gadgetbridge1.jpg)

Tap on it. Gadgdetbridge will pair and connect to your device:

![Gadgetbridge 2](gadgetbridge2.jpg)

# Updating with Gadgetbridge

## Preparation

All releases of InfiniTime are available on the [release page of the GitHub repo](https://github.com/InfiniTimeOrg/InfiniTime/releases).

Release files are available under the *Assets* button.

You need to download the DFU of the firmware version that you'd like to install, for example `pinetime-mcuboot-app-dfu-1.6.0.zip`

## Gadgetbridge

Now that Gadgetbridge is connected to your PineTime, use a file browser application (I'm using Seafile to browse my NAS) and browse to the DFU file (image-xxx.zip) you downloaded previously. Tap on it and open it using the Gadgetbridge application/firmware installer:

![Gadgetbridge 3](gadgetbridge3.jpg)

Read carefully the warning and tap **Install**:

![Gadgetbridge 4](gadgetbridge4.jpg)

Wait for the transfer to finish. Your PineTime should reset and reboot with the new version of InfiniTime!

Don't forget to **validate** your firmware. In the InfiniTime go to the settings (swipe right, select gear icon) and Firmware option and click **validate**. Otherwise after reboot the previous firmware will be used.

![Gadgetbridge 5](gadgetbridge5.jpg)
