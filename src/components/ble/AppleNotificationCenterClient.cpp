#include "components/ble/AppleNotificationCenterClient.h"
#include "components/ble/NotificationManager.h"
#include "systemtask/SystemTask.h"
#include <algorithm>

using namespace Pinetime::Controllers;

int OnDiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error* error, const struct ble_gatt_svc* service, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnDiscoveryEvent(conn_handle, error, service);
}

int OnANCSCharacteristicDiscoveredCallback(uint16_t conn_handle,
                                           const struct ble_gatt_error* error,
                                           const struct ble_gatt_chr* chr,
                                           void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnCharacteristicsDiscoveryEvent(conn_handle, error, chr);
}

int OnANCSDescriptorDiscoveryEventCallback(uint16_t conn_handle,
                                           const struct ble_gatt_error* error,
                                           uint16_t chr_val_handle,
                                           const struct ble_gatt_dsc* dsc,
                                           void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnDescriptorDiscoveryEventCallback(conn_handle, error, chr_val_handle, dsc);
}

int NewAlertSubcribeCallback(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnNewAlertSubcribe(conn_handle, error, attr);
}

int OnControlPointWriteCallback(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnControlPointWrite(conn_handle, error, attr);
}

AppleNotificationCenterClient::AppleNotificationCenterClient(Pinetime::System::SystemTask& systemTask,
                                                             Pinetime::Controllers::NotificationManager& notificationManager)
  : systemTask {systemTask}, notificationManager {notificationManager} {
}

bool AppleNotificationCenterClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_svc* service) {
  if (service == nullptr && error->status == BLE_HS_EDONE) {
    if (isDiscovered) {
      NRF_LOG_INFO("ANCS Discovery found, starting characteristics discovery");
      ble_gattc_disc_all_chrs(connectionHandle, ancsStartHandle, ancsEndHandle, OnANCSCharacteristicDiscoveredCallback, this);
    } else {
      NRF_LOG_INFO("ANCS not found");
      MaybeFinishDiscovery(connectionHandle);
    }
    return true;
  }

  if (service != nullptr && ble_uuid_cmp(&ancsUuid.u, &service->uuid.u) == 0) {
    NRF_LOG_INFO("ANCS discovered : 0x%x - 0x%x", service->start_handle, service->end_handle);

    ancsStartHandle = service->start_handle;
    ancsEndHandle = service->end_handle;
    isDiscovered = true;
  }
  return false;
}

int AppleNotificationCenterClient::OnCharacteristicsDiscoveryEvent(uint16_t connectionHandle,
                                                                   const ble_gatt_error* error,
                                                                   const ble_gatt_chr* characteristic) {
  if (error->status != 0 && error->status != BLE_HS_EDONE) {
    NRF_LOG_INFO("ANCS Characteristic discovery ERROR");
    MaybeFinishDiscovery(connectionHandle);
    return 0;
  }

  if (characteristic == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("ANCS Characteristic discovery complete");
    if (isCharacteristicDiscovered) {
      ble_gattc_disc_all_dscs(connectionHandle, notificationSourceHandle, ancsEndHandle, OnANCSDescriptorDiscoveryEventCallback, this);
    }
    if (isDataCharacteristicDiscovered) {
      ble_gattc_disc_all_dscs(connectionHandle, dataSourceHandle, ancsEndHandle, OnANCSDescriptorDiscoveryEventCallback, this);
    }
    if (isCharacteristicDiscovered == isControlCharacteristicDiscovered && isCharacteristicDiscovered == isDataCharacteristicDiscovered) {
      MaybeFinishDiscovery(connectionHandle);
    }
  } else if (characteristic != nullptr) {
    if (ble_uuid_cmp(&notificationSourceChar.u, &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANCS Characteristic discovered: Notification Source");
      notificationSourceHandle = characteristic->val_handle;
      isCharacteristicDiscovered = true;
    } else if (ble_uuid_cmp(&controlPointChar.u, &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANCS Characteristic discovered: Control Point");
      controlPointHandle = characteristic->val_handle;
      isControlCharacteristicDiscovered = true;
    } else if (ble_uuid_cmp(&dataSourceChar.u, &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANCS Characteristic discovered: Data Source");
      dataSourceHandle = characteristic->val_handle;
      isDataCharacteristicDiscovered = true;
    }
  }
  return 0;
}

int AppleNotificationCenterClient::OnDescriptorDiscoveryEventCallback(uint16_t connectionHandle,
                                                                      const ble_gatt_error* error,
                                                                      uint16_t characteristicValueHandle,
                                                                      const ble_gatt_dsc* descriptor) {
  if (error->status == 0) {
    if (characteristicValueHandle == notificationSourceHandle && ble_uuid_cmp(&notificationSourceChar.u, &descriptor->uuid.u)) {
      if (notificationSourceDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        notificationSourceDescriptorHandle = descriptor->handle;
        isDescriptorFound = true;
        uint8_t value[2] {1, 0};
        ble_gattc_write_flat(connectionHandle, notificationSourceDescriptorHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
        ble_gattc_write_flat(connectionHandle, ancsEndHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
      }
    } else if (characteristicValueHandle == controlPointHandle && ble_uuid_cmp(&controlPointChar.u, &descriptor->uuid.u)) {
      if (controlPointDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        controlPointDescriptorHandle = descriptor->handle;
        isControlDescriptorFound = true;
      }
    } else if (characteristicValueHandle == dataSourceHandle && ble_uuid_cmp(&dataSourceChar.u, &descriptor->uuid.u)) {
      if (dataSourceDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        dataSourceDescriptorHandle = descriptor->handle;
        isDataDescriptorFound = true;
        uint8_t value[2] {1, 0};
        ble_gattc_write_flat(connectionHandle, dataSourceDescriptorHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
      }
    }
  } else {
    if (error->status != BLE_HS_EDONE) {
      char errorStr[55];
      snprintf(errorStr, sizeof(errorStr), "ANCS Descriptor discovery ERROR: %d", error->status);
      NRF_LOG_INFO(errorStr);
    }
    if (isDescriptorFound == isDataDescriptorFound)
      MaybeFinishDiscovery(connectionHandle);
  }
  return 0;
}

int AppleNotificationCenterClient::OnNewAlertSubcribe(uint16_t connectionHandle,
                                                      const ble_gatt_error* error,
                                                      ble_gatt_attr* /*attribute*/) {
  if (error->status == 0) {
    NRF_LOG_INFO("ANCS New alert subscribe OK");

    // Mark subscriptions complete only after both CCCDs are known
    if (notificationSourceDescriptorHandle != 0 && dataSourceDescriptorHandle != 0) {
      subscriptionsDone = true;
    }
  } else {
    NRF_LOG_INFO("ANCS New alert subscribe ERROR");
  }
  if (isDescriptorFound == isControlDescriptorFound && isDescriptorFound == isDataDescriptorFound)
    MaybeFinishDiscovery(connectionHandle);

  return 0;
}

int AppleNotificationCenterClient::OnControlPointWrite(uint16_t /*connectionHandle*/,
                                                       const ble_gatt_error* error,
                                                       ble_gatt_attr* /*attribute*/) {
  if (error->status == 0) {
    NRF_LOG_INFO("ANCS Control Point write OK");
  } else {
    char errorStr[55];
    snprintf(errorStr, sizeof(errorStr), "ANCS Control Point ERROR: %d", error->status);
    NRF_LOG_INFO(errorStr);
  }
  return 0;
}

void AppleNotificationCenterClient::MaybeFinishDiscovery(uint16_t connectionHandle) {
  if (isCharacteristicDiscovered && isControlCharacteristicDiscovered && isDataCharacteristicDiscovered && isDescriptorFound &&
      isControlDescriptorFound && isDataDescriptorFound && subscriptionsDone) {
    onServiceDiscovered(connectionHandle);
  }
}

void AppleNotificationCenterClient::OnNotification(ble_gap_event* event) {
  if (event->notify_rx.attr_handle == notificationSourceHandle || event->notify_rx.attr_handle == notificationSourceDescriptorHandle) {
    NRF_LOG_INFO("ANCS Notification received");

    AncsNotification ancsNotif;

    os_mbuf_copydata(event->notify_rx.om, 0, 1, &ancsNotif.eventId);
    os_mbuf_copydata(event->notify_rx.om, 1, 1, &ancsNotif.eventFlags);
    os_mbuf_copydata(event->notify_rx.om, 2, 1, &ancsNotif.category);
    // Can be used to see how many grouped notifications are present
    // os_mbuf_copydata(event->notify_rx.om, 3, 1, &categoryCount);
    os_mbuf_copydata(event->notify_rx.om, 4, 4, &ancsNotif.uuid);

    // bool silent = (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::Silent)) != 0;
    //  bool important = eventFlags & static_cast<uint8_t>(EventFlags::Important);
    // bool preExisting = (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::PreExisting)) != 0;
    //  bool positiveAction = eventFlags & static_cast<uint8_t>(EventFlags::PositiveAction);
    //  bool negativeAction = eventFlags & static_cast<uint8_t>(EventFlags::NegativeAction);

    // If notification was removed, we remove it from the notifications map
    if (ancsNotif.eventId == static_cast<uint8_t>(EventIds::Removed) && notifications.contains(ancsNotif.uuid)) {
      notifications.erase(ancsNotif.uuid);
      NRF_LOG_INFO("ANCS Notification removed: %d", ancsNotif.uuid);
      return;
    }

    // If the notification is pre-existing, or if it is a silent notification, we do not add it to the list
    if (notifications.contains(ancsNotif.uuid) || (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::Silent)) != 0 ||
        (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::PreExisting)) != 0) {
      return;
    }

    // If new notification, add it to the notifications
    if (ancsNotif.eventId == static_cast<uint8_t>(EventIds::Added) &&
        (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::Silent)) == 0) {
      notifications.insert({ancsNotif.uuid, ancsNotif});
    } else {
      // If the notification is not added, we ignore it
      NRF_LOG_INFO("ANCS Notification not added, ignoring: %d", ancsNotif.uuid);
      return;
    }

    // The 6 is from TotalNbNotifications in NotificationManager.h + 1
    while (notifications.size() > 100) {
      notifications.erase(notifications.begin());
    }

    // if (notifications.contains(ancsNotif.uuid)) {
    //   notifications[ancsNotif.uuid] = ancsNotif;
    // } else {
    //   notifications.insert({ancsNotif.uuid, ancsNotif});
    // }

    // Request ANCS more info
    // The +4 is for the "..." at the end of the string
    uint8_t titleSize = maxTitleSize + 4;
    uint8_t subTitleSize = maxSubtitleSize + 4;
    uint8_t messageSize = maxMessageSize + 4;
    BYTE request[14];
    request[0] = 0x00; // Command ID: Get Notification Attributes
    request[1] = static_cast<uint8_t>(ancsNotif.uuid & 0xFF);
    request[2] = static_cast<uint8_t>((ancsNotif.uuid >> 8) & 0xFF);
    request[3] = static_cast<uint8_t>((ancsNotif.uuid >> 16) & 0xFF);
    request[4] = static_cast<uint8_t>((ancsNotif.uuid >> 24) & 0xFF);
    request[5] = 0x01; // Attribute ID: Title
    // request[6] = 0x00;
    request[6] = static_cast<uint8_t>(titleSize & 0xFF);
    request[7] = static_cast<uint8_t>((titleSize >> 8) & 0xFF);
    request[8] = 0x02; // Attribute ID: Subtitle
    request[9] = static_cast<uint8_t>(subTitleSize & 0xFF);
    request[10] = static_cast<uint8_t>((subTitleSize >> 8) & 0xFF);
    request[11] = 0x03; // Attribute ID: Message
    request[12] = static_cast<uint8_t>(messageSize & 0xFF);
    request[13] = static_cast<uint8_t>((messageSize >> 8) & 0xFF);

    ble_gattc_write_flat(event->notify_rx.conn_handle, controlPointHandle, request, sizeof(request), OnControlPointWriteCallback, this);
  } else if (event->notify_rx.attr_handle == dataSourceHandle || event->notify_rx.attr_handle == dataSourceDescriptorHandle) {
    uint16_t titleSize;
    uint16_t subTitleSize;
    uint16_t messageSize;
    uint32_t notificationUid;

    os_mbuf_copydata(event->notify_rx.om, 1, 4, &notificationUid);
    os_mbuf_copydata(event->notify_rx.om, 6, 2, &titleSize);
    os_mbuf_copydata(event->notify_rx.om, 8 + titleSize + 1, 2, &subTitleSize);
    os_mbuf_copydata(event->notify_rx.om, 8 + titleSize + 1 + 2 + subTitleSize + 1, 2, &messageSize);

    AncsNotification ancsNotif;
    ancsNotif.uuid = 0;

    // Check if the notification is in the session
    if (notifications.contains(notificationUid)) {
      if (notifications[notificationUid].isProcessed) {
        // If the notification is already processed, we ignore it
        NRF_LOG_INFO("Notification with UID %d already processed, ignoring", notificationUid);
        return;
      }
    } else {
      // If the notification is not in the session, we ignore it
      NRF_LOG_INFO("Notification with UID %d not found in session, ignoring", notificationUid);
      return;
    }

    if (notifications.contains(notificationUid)) {
      ancsNotif = notifications[notificationUid];
    } else {
      // If the Notification source didn't add it earlier, then don't process it
      NRF_LOG_INFO("Notification with UID %d not found in notifications map, ignoring datasource", notificationUid);
      return;
    }

    std::string decodedTitle = DecodeUtf8String(event->notify_rx.om, titleSize, 8);

    std::string decodedSubTitle = DecodeUtf8String(event->notify_rx.om, subTitleSize, 8 + titleSize + 1 + 2);

    std::string decodedMessage = DecodeUtf8String(event->notify_rx.om, messageSize, 8 + titleSize + 1 + 2 + subTitleSize + 1 + 2);

    // Debug event ids ands flags by putting them at front of message (in int format)
    // decodedMessage = std::to_string(ancsNotif.uuid) + " " + decodedMessage;

    NRF_LOG_INFO("Decoded Title: %s", decodedTitle.c_str());
    NRF_LOG_INFO("Decoded SubTitle: %s", decodedSubTitle.c_str());

    bool incomingCall = ancsNotif.uuid != 0 && ancsNotif.category == static_cast<uint8_t>(Categories::IncomingCall);

    if (!incomingCall) {
      if (titleSize >= maxTitleSize) {
        decodedTitle.resize(maxTitleSize - 3);
        decodedTitle += "...";
        if (!decodedSubTitle.empty()) {
          decodedTitle += " - ";
        } else {
          decodedTitle += "-";
        }
      } else {
        if (!decodedSubTitle.empty()) {
          decodedTitle += " - ";
        } else {
          decodedTitle += "-";
        }
      }

      if (subTitleSize > maxSubtitleSize) {
        decodedSubTitle.resize(maxSubtitleSize - 3);
        decodedSubTitle += "...";
      }
    }

    titleSize = static_cast<uint16_t>(decodedTitle.size());
    subTitleSize = static_cast<uint16_t>(decodedSubTitle.size());
    messageSize = static_cast<uint16_t>(decodedMessage.size());

    NotificationManager::Notification notif;
    notif.ancsUid = notificationUid;
    std::string notifStr;

    if (incomingCall) {
      notifStr += "Incoming Call:";
      notifStr += decodedTitle;
      notifStr += "\n";
      notifStr += decodedSubTitle;
    } else {
      notifStr += decodedTitle;
      if (!decodedSubTitle.empty()) {
        notifStr += decodedSubTitle + ":";
      }
      notifStr += decodedMessage;
    }

    // Adjust notification if too long
    if (notifStr.size() > NotificationManager::MessageSize) {
      notifStr.resize(97);
      notifStr += "...";
    }

    notif.message = std::array<char, NotificationManager::MessageSize + 1> {};
    std::strncpy(notif.message.data(), notifStr.c_str(), notif.message.size() - 1);

    if (incomingCall) {
      notif.message[13] = '\0'; // Separate Title and Message
    } else if (!decodedSubTitle.empty()) {
      notif.message[titleSize + subTitleSize] = '\0'; // Separate Title and Message
    } else {
      notif.message[titleSize - 1] = '\0'; // Separate Title and Message
    }

    notif.message[notif.message.size() - 1] = '\0'; // Ensure null-termination
    notif.size = std::min(std::strlen(notifStr.c_str()), notif.message.size());
    if (incomingCall) {
      notif.category = Pinetime::Controllers::NotificationManager::Categories::IncomingCall;
    } else {
      notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
    }
    notificationManager.Push(std::move(notif));

    // Only ping the system task if the notification was added and ignore pre-existing notifications
    if (ancsNotif.isProcessed == false && (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::Silent)) == 0 &&
        (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::PreExisting)) == 0) {
      systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
    }

    // Mark the notification as processed in the session
    notifications[notificationUid].isProcessed = true;
  }
}

void AppleNotificationCenterClient::AcceptIncomingCall(uint32_t uuid) {
  if (notifications.contains(uuid)) {
    const AncsNotification ancsNotif = notifications[uuid];
    if (ancsNotif.category != static_cast<uint8_t>(Categories::IncomingCall)) {
      return;
    }
  } else {
    return;
  }

  uint8_t value[6];
  value[0] = 0x02; // Command ID: Perform Notification Action
  value[1] = static_cast<uint8_t>((uuid & 0xFF));
  value[2] = static_cast<uint8_t>((uuid >> 8) & 0xFF);
  value[3] = static_cast<uint8_t>((uuid >> 16) & 0xFF);
  value[4] = static_cast<uint8_t>((uuid >> 24) & 0xFF);
  value[5] = 0x00; // Action ID: Positive Action

  ble_gattc_write_flat(systemTask.nimble().connHandle(), controlPointHandle, value, sizeof(value), OnControlPointWriteCallback, this);
}

void AppleNotificationCenterClient::RejectIncomingCall(uint32_t uuid) {
  AncsNotification ancsNotif;
  if (notifications.contains(uuid)) {
    ancsNotif = notifications[uuid];
    if (ancsNotif.category != static_cast<uint8_t>(Categories::IncomingCall)) {
      return;
    }
  } else {
    return;
  }

  uint8_t value[6];
  value[0] = 0x02; // Command ID: Perform Notification Action
  value[1] = static_cast<uint8_t>(uuid & 0xFF);
  value[2] = static_cast<uint8_t>((uuid >> 8) & 0xFF);
  value[3] = static_cast<uint8_t>((uuid >> 16) & 0xFF);
  value[4] = static_cast<uint8_t>((uuid >> 24) & 0xFF);
  value[5] = 0x01; // Action ID: Negative Action

  ble_gattc_write_flat(systemTask.nimble().connHandle(), controlPointHandle, value, sizeof(value), OnControlPointWriteCallback, this);
}

void AppleNotificationCenterClient::Reset() {
  ancsStartHandle = 0;
  ancsEndHandle = 0;
  gattStartHandle = 0;
  gattEndHandle = 0;
  serviceChangedHandle = 0;
  serviceChangedDescriptorHandle = 0;
  notificationSourceHandle = 0;
  notificationSourceDescriptorHandle = 0;
  controlPointHandle = 0;
  controlPointDescriptorHandle = 0;
  dataSourceHandle = 0;
  dataSourceDescriptorHandle = 0;
  isGattDiscovered = false;
  isGattCharacteristicDiscovered = false;
  isGattDescriptorFound = false;
  isDiscovered = false;
  isCharacteristicDiscovered = false;
  isDescriptorFound = false;
  isControlCharacteristicDiscovered = false;
  isControlDescriptorFound = false;
  isDataCharacteristicDiscovered = false;
  isDataDescriptorFound = false;
  subscriptionsDone = false;

  notifications.clear();
}

void AppleNotificationCenterClient::Discover(uint16_t connectionHandle, std::function<void(uint16_t)> onServiceDiscovered) {
  NRF_LOG_INFO("[ANCS] Starting discovery");
  this->onServiceDiscovered = onServiceDiscovered;
  ble_gattc_disc_svc_by_uuid(connectionHandle, &ancsUuid.u, OnDiscoveryEventCallback, this);
}

// This function is used for debugging purposes to log a message and push a notification
// Used to test BLE debugging on production devices
void AppleNotificationCenterClient::DebugNotification(const char* msg) const {
  NRF_LOG_INFO("[ANCS DEBUG] %s", msg);

  NotificationManager::Notification notif;
  std::strncpy(notif.message.data(), msg, notif.message.size() - 1);
  notif.message[notif.message.size() - 1] = '\0'; // Ensure null-termination
  notif.size = std::min(std::strlen(msg), notif.message.size());
  notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
  notificationManager.Push(std::move(notif));

  systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
}

std::string AppleNotificationCenterClient::DecodeUtf8String(os_mbuf* om, uint16_t size, uint16_t offset) {
  std::string decoded;
  decoded.reserve(size);

  auto isInFontDefinition = [](uint32_t codepoint) -> bool {
    // Check if the codepoint falls into the specified font ranges or is explicitly listed
    return (codepoint >= 0x20 && codepoint <= 0x7E) ||   // Printable ASCII
           (codepoint >= 0x410 && codepoint <= 0x44F) || // Cyrillic
           codepoint == 0xB0;
  };

  for (uint16_t i = 0; i < size;) {
    uint8_t byte = 0;
    if (os_mbuf_copydata(om, offset + i, 1, &byte) != 0) {
      break; // Handle error in copying data (e.g., log or terminate processing)
    }

    if (byte <= 0x7F) { // Single-byte UTF-8 (ASCII)
      if (isInFontDefinition(byte)) {
        decoded.push_back(static_cast<char>(byte));
      } else {
        decoded.append("�"); // Replace unsupported
      }
      ++i;
    } else { // Multi-byte UTF-8
      // Determine sequence length based on leading byte
      int sequenceLength = 0;
      if ((byte & 0xE0) == 0xC0) {
        sequenceLength = 2; // 2-byte sequence
      } else if ((byte & 0xF0) == 0xE0) {
        sequenceLength = 3; // 3-byte sequence
      } else if ((byte & 0xF8) == 0xF0) {
        sequenceLength = 4; // 4-byte sequence
      }

      if (i + sequenceLength > size) {
        decoded.append("�"); // Incomplete sequence, replace
        break;
      }

      // Read the full sequence
      std::string utf8Char;
      bool validSequence = true;
      uint32_t codepoint = 0;

      for (int j = 0; j < sequenceLength; ++j) {
        uint8_t nextByte = 0;
        os_mbuf_copydata(om, offset + i + j, 1, &nextByte);
        utf8Char.push_back(static_cast<char>(nextByte));

        if (j == 0) {
          // Leading byte contributes significant bits
          if (sequenceLength == 2) {
            codepoint = nextByte & 0x1F;
          } else if (sequenceLength == 3) {
            codepoint = nextByte & 0x0F;
          } else if (sequenceLength == 4) {
            codepoint = nextByte & 0x07;
          }
        } else {
          // Continuation bytes contribute lower bits
          if ((nextByte & 0xC0) != 0x80) {
            validSequence = false; // Invalid UTF-8 continuation byte
            break;
          }
          codepoint = (codepoint << 6) | (nextByte & 0x3F);
        }
      }

      if (validSequence && isInFontDefinition(codepoint)) {
        decoded.append(utf8Char); // Append valid UTF-8 character
      } else {
        decoded.append("�"); // Replace unsupported
      }
      i += sequenceLength;
    }
  }

  return decoded;
}
