# Connecting to Gadgetbridge

Launch Gadgetbridge and tap on the menu button in the top left:

![Gadgetbridge 0](gadgetbridge0.jpg)

Press the "Connect new device" button:

![Gadgetbridge 1](gadgetbridge1.jpg)

Your PineTime should appear on the list. Tap on it.

Tap on it. Gadgdetbridge will pair and connect to your device:

![Gadgetbridge 2](gadgetbridge2.jpg)

# Updating firmware or resources with Gadgetbridge

Now that Gadgetbridge is connected to your PineTime, press the three dots on the device card:

![Gadgetbridge 3](gadgetbridge3.jpg)

Now press the "File Installer" button:

![Gadgetbridge 4](gadgetbridge4.jpg)

Select the firmware (`pinetime-mcuboot-app-dfu-x.x.x.zip`) or resources (`infinitime-resources-x.x.x.zip`) you downloaded from the [Releases tab](https://github.com/InfiniTimeOrg/InfiniTime/releases/latest):

![Gadgetbridge 5](gadgetbridge5.jpg)

Wait for the transfer to finish.

### When updating firmware

There will be a progress bar on both the watch and the phone. Your PineTime should reboot with the new version of InfiniTime!

Don't forget to **validate** your firmware. In the InfiniTime go to the settings (swipe right, select gear icon) and scroll to the Firmware option and click **validate**. Otherwise, after reboot the previous firmware will be used.

![Validate](validate.png)

### When updating resources

There will be a progress bar only on the phone, watch screen will turn on and stay on during the update. InfiniTime does not need to be rebooted after updating the resources.
