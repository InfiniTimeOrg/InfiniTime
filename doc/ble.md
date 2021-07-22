# Bluetooth Low-Energy : 
## Introduction
This page describes the BLE implementation and API built in this firmware.

**Note** : I'm a beginner in BLE related technologies and the information of this document reflect my current knowledge and understanding of the BLE stack. These informations might be erroneous or incomplete. Feel free to submit a PR if you think you can improve these.  

## BLE Connection
When starting the firmware start a BLE advertising : it sends small messages that can be received by any *central* device in range. This allows the device to announce its presence to other devices.

A companion application (running on a PC, RaspberryPi, smartphone) which received this avertising packet can request a connection to the device. This connection procedure allows the 2 devices to negotiate communication parameters, security keys,...

When the connection is established, the pinetime will try to discover services running on the companion application. For now **CTS** (**C**urrent **T**ime **S**ervice) and **ANS** (**A**lert **N**otification **S**ervice) are supported.

If **CTS** is detected, it'll request the current time to the companion application. If **ANS** is detected, it will listen to new notifications coming from the companion application.

![BLE connection sequence diagram](ble/connection_sequence.png "BLE connection sequence diagram")

## BLE UUIDs
When possible, InfiniTime tries to implement BLE services defined by the BLE specification. 

When the service does not exist in the BLE specification, InfiniTime implement custom services. As all BLE services, custom services are identified by a UUID. Here is how to define the UUID of custom services in InfiniTime:

```
 - Base UUID :           xxxxxxxx-78fc-48fe-8e23-433b3a1942d0
 - Service UUID :        SSSS0000-78fc-48fe-8e23-433b3a1942d0 where SSSS is the service ID
 - Characteristic UUID : SSSSCCCC-78fc-48fe-8e23-433b3a1942d0 where CCCC is the characteristic ID for the service SSSS and is different than 0
```

The following custom services are implemented in InfiniTime:

 - Since InfiniTime 0.8:
   ```
    * Music Service :             00000000-78fc-48fe-8e23-433b3a1942d0
   ```
   
 - Since InfiniTime 0.11:
   ```
     * Navigation Service :        00010000-78fc-48fe-8e23-433b3a1942d0
   ```

## BLE services
[List of standard BLE services](https://www.bluetooth.com/specifications/gatt/services/)

### CTS
[Current Time Service](https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.current_time.xml)

### ANS
[Alert Notification Service](https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.alert_notification.xml)

![ANS sequence diagram](./ble/ans_sequence.png "ANS sequence diagram")

