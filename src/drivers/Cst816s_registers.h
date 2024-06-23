#pragma once

// CST816(S) / CST716 High-performance self-capacitive touch chip
// Hynitron, www.hynitron.com

// Manually assembled from auto-translated incomplete datasheets and documents.

// The device contains an embedded firmware, behaviour may differ.
// Code fragments on how to update the internal firmware exist, but not much is know about
// the internal architecture. No verified compatible binary firmware has been found yet.

// The "S" suffix stands for speed, i.e. double the sample rate (>100 Hz instead of ~50 Hz (?)).

// The CST816S has an auto-sleep low-power mode, from which it can wake via a touch event.
// In low-power sleep, the I2C interface is inactive.
// It sends a gesture event while the gesture is still being performed.

// The CST716 has no auto-sleep functionality, and sends a gesture event after the gesture has been completed.
// Its I2C interface is always active, as it stays in normal mode.

// Both chips are able to enter a deep-sleep mode, from which they can only wake via a signal on the reset pin.

// Registers

// * = Works only on chips which have not been preconfigured (fused) in the factory.
// CST716 are probably always fused, default: reporting mode.
// CST816S can be reconfigureable, but can be fused as well. Some fused ones report with a CST716 chip ID.

// ** = Auto-sleep related registers. Only on the CST816S.

#define DEV_MODE   0x00 // readonly. Always 0.
#define GESTURE_ID 0x01 // readonly. Which gesture was detected.
#define TD_STATUS  0x02 // readonly. Number of touch points. (0 - 2).

#define P1_X_POS_H 0x03 // readonly. Point 1: X touch event flag + X touch coordinate MSB.
#define P1_X_POS_L 0x04 // readonly. Point 1: X touch coordinate LSB.
#define P1_Y_POS_H 0x05 // readonly. Point 1: Y touch event flag + Y touch coordinate MSB.
#define P1_Y_POS_L 0x06 // readonly. Point 1: Y touch coordinate LSB.
#define P1_WEIGHT  0x07 // readonly. Point 1: Touch weight.
#define P1_MISC    0x08 // readonly. Point 1: Touch area.

#define P2_X_POS_H 0x09 // readonly. Point 2: X touch event flag + X touch coordinate MSB.
#define P2_X_POS_L 0x0A // readonly. Point 2: X touch coordinate LSB.
#define P2_Y_POS_H 0x0B // readonly. Point 2: Y touch event flag + Y touch coordinate MSB.
#define P2_Y_POS_L 0x0C // readonly. Point 2: Y touch coordinate LSB.
#define P2_WEIGHT  0x0D // readonly. Point 2: Touch weight.
#define P2_MISC    0x0E // readonly. Point 2: Touch area.

#define BPC0_H 0xB0 // unknown. BPC0 value MSB.
#define BPC0_L 0xB1 // unknown. BPC0 value LSB.
#define BPC1_H 0xB2 // unknown. BPC1 value MSB.
#define BPC1_L 0xB3 // unknown. BPC1 value LSB.

#define CHIP_ID    0xA7 // readonly. Chip / firmware type ID.
#define PROJ_ID    0xA8 // readonly. Vendor / project ID.
#define FW_VERSION 0xA9 // readonly. Firmware version.

#define MOTION_MASK     0xEC // read-write*. Motion configuration.
#define IRQ_PULSE_WIDTH 0xED // read-write. Interrupt low pulse output width. Unit 0.1ms, possible value: 1～200. The default value is 10.
// read-write. Normal fast detection cycle. This value affects LpAutoWakeTime and AutoSleepTime. Unit 10ms, possible value: 1～30. The
// default value is 1.
#define NOR_SCAN_PER 0xEE
// read-write. Gesture detection sliding partition angle control. Angle=tan(c)*10. c is the angle based on the positive direction of
// the x-axis.
#define MOTION_S1_ANGLE 0xEF

#define LP_SCAN_RAW1_H 0xF0 // readonly**. Low power scan, MSB of the reference value of channel 1.
#define LP_SCAN_RAW1_L 0xF1 // readonly**. Low power scan, LSB of the reference value of channel 1.
#define LP_SCAN_RAW2_H 0xF2 // readonly**. Low power scan, MSB of the reference value of channel 2.
#define LP_SCAN_RAW2_L 0xF3 // readonly**. Low power scan, LSB of the reference value of channel 2.
// read-write**. Automatic recalibration cycle in low power mode. The unit is 1 minute, and the possible value is 1 to 5. The default
// value is 5.
#define LP_AUTO_WAKE_TIME 0xF4
// read-write**. Low-power scan wake-up threshold. The smaller the more sensitive. Possible values: 1 to 255. The default value
// is 48.
#define LP_SCAN_TH 0xF5
// read-write**. Low power scan range. The larger the more sensitive, the higher the power consumption. Possible values: 0, 1, 2, 3.
// The default value is 3.
#define LP_SCAN_WIN 0xF6
// read-write**. Low power scan frequency. The smaller the more sensitive. Possible values: 1 to 255. The default value is 7.
#define LP_SCAN_FREQ  0xF7
#define LP_SCAN_I_DAC 0xF8 // read-write**. Low power scan current. The smaller the more sensitive. Possible values: 1 to 255.
// read-write**. Automatically enter low power mode when there is no touch for x seconds. The unit is 1s, the default value is 2.
#define AUTO_SLEEP_TIME 0xF9
// read-write*. Interrupt configuration. 0x60 = report mode, 0x11 = gesture mode, 0x71 = both.
#define IRQ_CTL 0xFA
// read-write. Automatic reset (cancel) when there is a touch but no valid gesture within x seconds. The unit is 1s. When it is 0,
// this function is not enabled. Default is 5.
#define AUTO_RESET 0xFB
// read-write. Automatic reset (cancel) after long press for x seconds. The unit is 1s. When it is 0, this function is not enabled.
// Default is 10.
#define LONG_PRESS_TIME 0xFC
// read-write. Pin IO configuration.
#define IO_CTL 0xFD
// read-write**. The default is 0, enabling automatic entry into low power mode. When it is a non-zero value, automatic entry into
// low power mode is disabled.
#define DIS_AUTO_SLEEP 0xFE

#define PWR_MODE_CST816S 0xE5 // read-write. Power state for the CST816S.
#define PWR_MODE_CST716  0xA5 // read-write. Power state for the CST716.

// Data fields

// GESTURE_ID

#define GESTURE_ID_NONE        0x00
#define GESTURE_ID_SLIDE_DOWN  0x01
#define GESTURE_ID_SLIDE_UP    0x02
#define GESTURE_ID_SLIDE_LEFT  0x03
#define GESTURE_ID_SLIDE_RIGHT 0x04
#define GESTURE_ID_SINGLE_TAP  0x05
#define GESTURE_ID_DOUBLE_TAP  0x0B
#define GESTURE_ID_LONG_PRESS  0x0C

// TD_STATUS

#define TD_STATUS_MASK 0x0F

// *_POS_H

#define POS_H_POS_MASK 0x0F

#define POS_H_EVENT_MASK 0xC0
#define POS_H_EVENT0     (1 << 6)
#define POS_H_EVENT1     (1 << 7)

#define POS_H_EVENT_DOWN    (0 << 6)
#define POS_H_EVENT_UP      (1 << 6)
#define POS_H_EVENT_CONTACT (2 << 6)
#define POS_H_EVENT_NONE    (3 << 6)

// CHIP_ID

#define CHIP_ID_CST816S 0xB4
#define CHIP_ID_CST716  0x20

// Control register configurations

// MOTION_MASK

#define MOTION_MASK_EN_DCLICK (1 << 0) // Enable double click action
#define MOTION_MASK_EN_CON_UD (1 << 1) // Enable continuous up-down sliding action
#define MOTION_MASK_EN_CON_LR (1 << 2) // Enable continuous left-right sliding action

// IRQ_CTL

#define IRQ_CTL_ONCE_WLP  (1 << 0) // The long press gesture only emits a low pulse signal.
#define IRQ_CTL_EN_MOTION (1 << 4) // When a gesture is detected, a low pulse is emitted.
#define IRQ_CTL_EN_CHANGE (1 << 5) // A low pulse is emitted when a touch state change is detected.
#define IRQ_CTL_EN_TOUCH  (1 << 6) // Periodically emit low pulses when a touch is detected.
#define IRQ_CTL_EN_TEST   (1 << 7) // Interrupt pin test, automatically and periodically send low pulses after enabling.

// IO_CTL

#define IO_CTL_EN_1V8   (1 << 0) // I2C and IRQ pin voltage level selection, the default is VDD level. 0: VDD, 1: 1.8V.
#define IO_CTL_IIC_OD   (1 << 1) // I2C pin drive mode, the default is resistor pull-up. 0: Resistor pull-up 1: OD.
#define IO_CTL_SOFT_RST (1 << 2) // Enable soft reset by pulling down the IRQ pin. 0: Soft reset is disabled. 1: Enable soft reset.

// PWR_MODE_*

#define PWR_MODE_ACTIVE     0x00 // Normal state of execution
#define PWR_MODE_DEEP_SLEEP 0x03 // Deep-sleep mode. Touch to wakeup and I2C are disabled.
