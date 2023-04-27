# Bluetooth Low-Energy :

## Introduction

This page describes the BLE implementation and API built in this firmware.

---

### Table of Contents

- [BLE Connection](#ble-connection)
- [BLE FS](#ble-fs)
- [BLE UUIDs](#ble-uuids)
- [BLE Services](#ble-services)
  - [CTS](#cts)
  - [ANS](#ans)
- [Getting Information](#getting-information)
  - [Firmware Version](#firmware-version)
  - [Battery Level](#battery-level)
  - [Heart Rate](#heart-rate)
- [Notifications](#notifications)
  - [New Alert](#new-alert)
  - [Notification Event](#notification-event)
- [Firmware Upgrades](#firmware-upgrades)
  - [Step one](#step-one)
  - [Step two](#step-two)
  - [Step three](#step-three)
  - [Step four](#step-four)
  - [Step five](#step-five)
  - [Step six](#step-six)
  - [Step seven](#step-seven)
  - [Step eight](#step-eight)
  - [Step nine](#step-nine)
- [Music Control](#music-control)
  - [Events](#events)
  - [Status](#status)
  - [Artist, Track, and Album](#artist-track-and-album)
- [Time](#time)

---

## BLE Connection

When starting, the firmware starts BLE advertising. It sends small messages that can be received by any *central* device in range. This allows the device to announce its presence to other devices.

A companion application (running on a PC, Raspberry Pi, smartphone, etc.) which receives this advertising packet can request a connection to the device. This connection procedure allows the 2 devices to negotiate communication parameters, security keys, etc.

When the connection is established, the PineTime will try to discover services running on the companion application. For now **CTS** (**C**urrent **T**ime **S**ervice) and **ANS** (**A**lert **N**otification **S**ervice) are supported.

If **CTS** is detected, it'll request the current time to the companion application. If **ANS** is detected, it will listen to new notifications coming from the companion application.

![BLE connection sequence diagram](ble/connection_sequence.png "BLE connection sequence diagram")

---

## BLE FS

The documentation for BLE FS can be found here:
[BLEFS.md](./BLEFS.md)

---

## BLE UUIDs

When possible, InfiniTime tries to implement BLE services defined by the BLE specification.

When the service does not exist in the BLE specification, InfiniTime implements custom services. Custom services are identified by a UUID, as are all BLE services. Here is how to define the UUID of custom services in InfiniTime:

```
 - Base UUID :           xxxxxxxx-78fc-48fe-8e23-433b3a1942d0
 - Service UUID :        SSSS0000-78fc-48fe-8e23-433b3a1942d0 where SSSS is the service ID
 - Characteristic UUID : SSSSCCCC-78fc-48fe-8e23-433b3a1942d0 where CCCC is the characteristic ID for the service SSSS and is different than 0
```

The following custom services are implemented in InfiniTime:

- Since InfiniTime 0.8:

  - Music Service : `00000000-78fc-48fe-8e23-433b3a1942d0`

- Since InfiniTime 0.11:

  - [Navigation Service](NavigationService.md) : `00010000-78fc-48fe-8e23-433b3a1942d0`

- Since InfiniTime 0.13

  - Call characteristic (extension to the Alert Notification Service): `00020001-78fc-48fe-8e23-433b3a1942d0`

- Since InfiniTime 1.7:

  - [Motion Service](MotionService.md): `00030000-78fc-48fe-8e23-433b3a1942d0`

- Since InfiniTime 1.8:

  - [Weather Service](/src/components/ble/weather/WeatherService.h): `00040000-78fc-48fe-8e23-433b3a1942d0`


- Development:
    * Running characteristic (extension to Heart Rate Service): 00050001-78fc-48fe-8e23-433b3a1942d0
---

## BLE services

[List of standard BLE services](https://www.bluetooth.com/specifications/gatt/services/)

### CTS

[Current Time Service](https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.current_time.xml)

### ANS

[Alert Notification Service](https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.alert_notification.xml)

![ANS sequence diagram](./ble/ans_sequence.png "ANS sequence diagram")

---

### Getting Information

The InfiniTime firmware exposes some information about itself through BLE. The BLE characteristic UUIDs for this information are as follows:

- Firmware Version: `00002a26-0000-1000-8000-00805f9b34fb`
- Battery Level: `00002a19-0000-1000-8000-00805f9b34fb`
- Heart Rate: `00002a37-0000-1000-8000-00805f9b34fb`

#### Firmware Version

Reading a value from the firmware version characteristic will yield a UTF-8 encoded string containing the version of InfiniTime being run on the device. Example: `1.6.0`.

#### Battery Level

Reading from the battery level characteristic yields a single byte of data. This byte can be converted to an unsigned 8-bit integer which will be the battery percentage. This characteristic allows notifications for updates as the value changes.

#### Heart Rate

Reading from the heart rate characteristic yields two bytes of data. I am not sure of the function of the first byte. It appears to always be zero. The second byte can be converted to an unsigned 8-bit integer which is the current heart rate. This characteristic also allows notifications for updates as the value changes.

---

### Notifications

InfiniTime uses the Alert Notification Service (ANS) for notifications. The relevant UUIDs are as follows:

- New Alert: `00002a46-0000-1000-8000-00805f9b34fb`
- Notification Event: `00020001-78fc-48fe-8e23-433b3a1942d0`

#### New Alert

The new alert characteristic allows sending new notifications to InfiniTime. It requires the following format:

```
<category><amount>\x00<\x00-separated data>
```

For example, here is what a normal notification looks like in Golang (language of `itd`):

```go
// \x00 is the category for simple alert, and there is one new notification, hence \x01.
"\x00\x01\x00Test Title\x00Test Body"
```

A call notification looks like so:

```go
// \x03 is the category for calls, and there is one new call notification, hence \x01.
"\x03\x01\x00Mary"
```

The `\x00` stands for hexadecimal `00` which means null.

Here is the list of categories and commands:

- Simple Alert: `0`
- Email: `1`
- News: `2`
- Call Notification: `3`
- Missed Call: `4`
- SMS/MMS: `5`
- Voicemail: `6`
- Schedule: `7`
- High Prioritized Alert: `8`
- Instant Message: `9`
- All Alerts: `0xFF`

These lists and information were retrieved from the following pages in the Nordic docs:

- [Alert Notification Service Client](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.2.0%2Fgroup__ble__ans__c.html)
- [Alert Notification Application](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v13.0.0%2Fble_sdk_app_alert_notification.html)

#### Notification Event

A call notification in InfiniTime contains three buttons. Decline, Accept, and Mute. The notification event characteristic contains the button tapped by the user on a call notification. This characteristic only allows notify, **not** read.

Enabling notifications from this characteristic, you get a single byte whenever the user taps a button on the call notification. This byte is an unsigned 8-bit integer that signifies one of the buttons. The numbers are as follows:

- 0: Declined
- 1: Accepted
- 2: Muted

---

### Firmware Upgrades

Firmware upgrades in InfiniTime are probably the most complex of the BLE operations. It is a nine step process requiring multiple commands be sent to multiple characteristics. The relevant UUIDs are as follows:

- Control Point: `00001531-1212-efde-1523-785feabcd123`
- Packet: `00001532-1212-efde-1523-785feabcd123`

A DFU upgrade archive for InfiniTime consists of multiple files. The most important being the .bin and .dat files. The first is the actual firmware, while the second is a packet that initializes DFU. Both are needed for a DFU upgrade.

The first thing to do is to enable notifications on the control point characteristic. This will be needed for verifying that the proper responses are being sent back from InfiniTime.

#### Step one

For the first step, write `0x01`, `0x04` to the control point characteristic. This will signal InfiniTime that a DFU upgrade is to be started.

#### Step two

In step two, send the total size in bytes of the firmware file to the packet characteristic. This value should be an unsigned 32-bit integer encoded as little-endian. In front of this integer should be 8 null bytes. This is because there are three items that can be updated and each 4 bytes is for one of those. The last four are for the InfiniTime application, so those are the ones that need to be set.

#### Step three

Before running step three, wait for a response from the control point. This response should be `0x10`, `0x01`, `0x01` which indicates a successful DFU start. In step three, send `0x02`, `0x00` to the control point. This will signal InfiniTime to expect the init packet on the packet characteristic.

#### Step four

The previous step prepared InfiniTime for this one. In this step, send the contents of the .dat init packet file to the packet characteristic. After this, send `0x02`, `0x01` indicating that the packet has been sent.

#### Step five

Before running this step, wait to receive `0x10`, `0x02`, `0x01` which indicates that the packet has been received. During this step, send the packet receipt interval to the control point. The firmware file will be sent in segments of 20 bytes each. The packet receipt interval indicates how many segments should be received before sending a receipt containing the amount of bytes received so that it can be confirmed to be the same as the amount sent. This is very useful for detecting packet loss. `itd` uses `0x08`, `0x0A` which indicates 10 segments.

#### Step six

In step six, write `0x03` to the control point, indicating that the firmware will be sent next on the packet characteristic.

#### Step seven

This step is the most difficult. Here, the actual firmware is sent to InfiniTime.

As mentioned before, the firmware file must be split up into segments of 20 bytes each and sent to the packet characteristic one by one. Every 10 segments (or whatever you have set the interval to), check for a response starting with `0x11`. The rest of the response will be the amount of bytes received encoded as a little-endian unsigned 32-bit integer. Confirm that this matches the amount of bytes sent, and then continue sending more segments.

#### Step eight

Before running this step, wait to receive `0x10`, `0x03`, `0x01` which indicates a successful receipt of the firmware image. In this step, write `0x04` to the control point to signal InfiniTime to validate the image it has received.

#### Step nine

Before running this step, wait to receive `0x10`, `0x04`, `0x01` which indicates that the image has been validated. In this step, send `0x05` to the control point as a command with no response. This signals InfiniTime to activate the new firmware and reboot.

Once all of these steps are complete, the DFU is complete. Don't forget to validate the firmware in the settings.

---

### Music Control

InfiniTime contains a music controller app which is meant to control the music playback and volume through the companion.

The following UUIDs are relevant to this:

- Events: `00000001-78fc-48fe-8e23-433b3a1942d0`
- Status: `00000002-78fc-48fe-8e23-433b3a1942d0`
- Artist: `00000003-78fc-48fe-8e23-433b3a1942d0`
- Track: `00000004-78fc-48fe-8e23-433b3a1942d0`
- Album: `00000005-78fc-48fe-8e23-433b3a1942d0`

#### Events

The events characteristic is meant to respond to user input in the music controller app.

Enabling notifications on this characteristic gives you a single byte upon any event. This byte can be converted to an unsigned 8-bit integer which corresponds to each possible event. Here are the events:

- App Opened: `0xe0`
- Play: `0x00`
- Pause: `0x01`
- Next: `0x03`
- Previous: `0x04`
- Volume up: `0x05`
- Volume down: `0x06`

#### Status

The status characteristic allows setting the playing status of music. Send `0x01` to the status characteristic for playing, and `0x00` for paused.

#### Artist, Track, and Album

These characteristics all work the same way. Simply send a UTF-8 encoded string to the relevant characteristic in order to set the value in the app.

---

### Time

InfiniTime allows setting its time via the Current Time Service (CTS)

The UUID for the current time characteristic is: `00002a2b-0000-1000-8000-00805f9b34fb`

This characteristic expects a particular format:

- Year (`uint16`)
- Month (`uint8`)
- Day (`uint8`)
- Hour (`uint8`)
- Minute (`uint8`)
- Second (`uint8`)
- Weekday (`uint8`)
- Microsecond divided by `1e6*256` (`uint8`)
- Binary 0001 (`uint8`)

Write all of these together, encoded as little-endian, to the current time characteristic.
