#pragma once

#include <cstdint>
#include "Identity.h"

#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include "components/ble/AlertNotificationClient.h"
#include "components/ble/AlertNotificationService.h"
#include "components/ble/BatteryInformationService.h"
#include "components/ble/CurrentTimeClient.h"
#include "components/ble/CurrentTimeService.h"
#include "components/ble/DeviceInformationService.h"
#include "components/ble/DfuService.h"
#include "components/ble/FSService.h"
#include "components/ble/HeartRateService.h"
#include "components/ble/ImmediateAlertService.h"
#include "components/ble/IntrusionLog.h"
#include "components/ble/TrackerDetector.h"
#include "components/ble/MusicService.h"
#include "components/ble/NavigationService.h"
#include "components/ble/ServiceDiscovery.h"
#include "components/ble/MotionService.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/fs/FS.h"

namespace Pinetime {
  namespace Drivers {
    class SpiNorFlash;
  }

  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class Ble;
    class DateTime;
    class NotificationManager;

    class NimbleController {

    public:
      NimbleController(Pinetime::System::SystemTask& systemTask,
                       Ble& bleController,
                       DateTime& dateTimeController,
                       NotificationManager& notificationManager,
                       Battery& batteryController,
                       Pinetime::Drivers::SpiNorFlash& spiNorFlash,
                       HeartRateController& heartRateController,
                       MotionController& motionController,
                       FS& fs);
      void Init();
      void StartAdvertising();
      int OnGAPEvent(ble_gap_event* event);
      void StartDiscovery();

      Pinetime::Controllers::MusicService& music() {
        return musicService;
      };

      Pinetime::Controllers::NavigationService& navigation() {
        return navService;
      };

      Pinetime::Controllers::AlertNotificationService& alertService() {
        return anService;
      };

      Pinetime::Controllers::SimpleWeatherService& weather() {
        return weatherService;
      };

      Pinetime::Controllers::IntrusionLog& intrusionLog() {
        return intrusionLogger;
      };

      // Classifies the current connection for the intrusion log, returns true if it should raise an alert
      bool CheckIntrusion();

      // Tracker detection. Automatic scans run in the background when enabled, a live scan
      // (continuous, for hunting a tag down by signal strength) runs while the Trackers app is open.
      static constexpr size_t MaxTrackers = TrackerDetector::MaxTrackers;

      bool IsTrackerScanEnabled() const {
        return trackerScanEnabled;
      }

      void SetTrackerScanEnabled(bool enabled);
      void StartTrackerScan(bool live);
      void StopTrackerScan();

      bool IsTrackerScanning() const {
        return trackerScanActive;
      }

      size_t TrackerSnapshot(std::array<TrackerDetector::Tracker, MaxTrackers>& out);

      TrackerDetector::Tracker LastTrackerAlert() const {
        return lastTrackerAlert;
      }

      // Drops trackers that went away and raises an alert for one that's following
      void EvaluateTrackers();
      void SaveTrackerSettings();

      bool TrackerSettingsDirty() const {
        return trackerSettingsDirty;
      }

      int OnTrackerScanEvent(ble_gap_event* event);

      uint16_t connHandle();
      void NotifyBatteryLevel(uint8_t level);

      void RestartFastAdv() {
        fastAdvCount = 0;
      };

      void EnableRadio();
      void DisableRadio();

    private:
      void PersistBond(struct ble_gap_conn_desc& desc);
      void RestoreBond();
      bool HasBond();

      static constexpr const char* deviceName = Pinetime::Identity::handle;
      Pinetime::System::SystemTask& systemTask;
      Ble& bleController;
      DateTime& dateTimeController;
      Pinetime::Drivers::SpiNorFlash& spiNorFlash;
      FS& fs;
      DfuService dfuService;

      DeviceInformationService deviceInformationService;
      CurrentTimeClient currentTimeClient;
      AlertNotificationService anService;
      AlertNotificationClient alertNotificationClient;
      CurrentTimeService currentTimeService;
      MusicService musicService;
      SimpleWeatherService weatherService;
      NavigationService navService;
      BatteryInformationService batteryInformationService;
      ImmediateAlertService immediateAlertService;
      HeartRateService heartRateService;
      MotionService motionService;
      FSService fsService;
      ServiceDiscovery serviceDiscovery;
      IntrusionLog intrusionLogger;

      void LoadTrackerSettings();
      TrackerDetector trackerDetector;
      TrackerDetector::Tracker lastTrackerAlert {};
      bool trackerScanEnabled = false;
      bool trackerSettingsDirty = false;
      bool trackerScanActive = false;
      bool trackerScanLive = false;

      uint8_t addrType;
      uint16_t connectionHandle = BLE_HS_CONN_HANDLE_NONE;
      uint8_t fastAdvCount = 0;
      uint8_t bondId[16] = {0};
    };

    static NimbleController* nptr;
  }
}
