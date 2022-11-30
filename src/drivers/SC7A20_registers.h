#pragma once

// SC7A20 ±2G/±4G/±8G/±16G Three-axis micromachined digital accelerometer
// Hangzhou Silan Microelectronics Co., Ltd. www.silan.com.cn

// Manually assembled from an auto-translated incomplete datasheet
// This chips behaves nearly the same as a ST LIS2DH, the rest was added from the ST datasheet

// Registers

#define OUT_TEMP_L    0x0C // readonly
#define OUT_TEMP_H    0x0D // readonly
#define WHO_AM_I      0x0F // readonly
#define USER_CAL      0x13 // read-write
#define NVM_WR        0x1E // read-write
#define TEMP_CFG      0x1F // read-write
#define CTRL_REG1     0x20 // read-write. Control register 1.
#define CTRL_REG2     0x21 // read-write. Control register 2.
#define CTRL_REG3     0x22 // read-write. Control register 3.
#define CTRL_REG4     0x23 // read-write. Control register 4.
#define CTRL_REG5     0x24 // read-write. Control register 5.
#define CTRL_REG6     0x25 // read-write. Control register 6.
#define REFERENCE     0x26 // read-write
#define STATUS_REG    0x27 // read-write. Status register.
#define OUT_X_L       0x28 // readonly. X Axis accelerometer value. This value starts with 2 output in two's complement form.
#define OUT_X_H       0x29 // readonly
#define OUT_Y_L       0x2A // readonly. Y Axis accelerometer value. This value starts with 2 output in two's complement form.
#define OUT_Y_H       0x2B // readonly
#define OUT_Z_L       0x2C // readonly. Z Axis accelerometer value. This value starts with 2 output in two's complement form.
#define OUT_Z_H       0x2D // readonly
#define FIFO_CTRL_REG 0x2E // read-write
#define FIFO_SRC_REG  0x2F // readonly
#define INT1_CFG      0x30 // read-write. Interrupt 1 configuration.
#define INT1_SOURCE   0x31 // readonly. Interrupt 1 source / status.
#define INT1_THS      0x32 // read-write. Interrupt 1 treshold.
#define INT1_DURATION 0x33 // read-write. Interrupt 1 duration.
#define INT2_CFG      0x34 // read-write. Interrupt 2 configuration.
#define INT2_SOURCE   0x35 // readonly. Interrupt 2 source / status.
#define INT2_THS      0x36 // read-write. Interrupt 2 treshold.
#define INT2_DURATION 0x37 // read-write. Interrupt 1 duration.
#define CLICK_CFG     0x38 // read-write
#define CLICK_SRC     0x39 // readonly
#define CLICK_THS     0x3A // read-write
#define TIME_LIMIT    0x3B // read-write
#define TIME_LATENCY  0x3C // read-write
#define TIME_WINDOW   0x3D // read-write
#define ACT_THS       0x3E // read-write
#define ACT_DURATION  0x3F // read-write

// Control register configurations

// CTRL_REG1

#define CTRL_REG1_X_EN  (1 << 0) // XAxis enable, default is 1. (0: Xaxis disabled, 1: Xaxis enabled)
#define CTRL_REG1_Y_EN  (1 << 1) // YAxis enable, default is 1. (0: Yaxis disabled, 1: Yaxis enabled)
#define CTRL_REG1_Z_EN  (1 << 2) // ZAxis enable, default is 1. (0: Zaxis disabled, 1: Zaxis enabled)
#define CTRL_REG1_LP_EN (1 << 3) // Low power enable, the default value is 0. (0: normal working mode, 1: low power mode)
#define CTRL_REG1_ODR0  (1 << 4) // Data rate selection, default: 0000
#define CTRL_REG1_ODR1  (1 << 5)
#define CTRL_REG1_ODR2  (1 << 6)
#define CTRL_REG1_ODR3  (1 << 7)

#define CTRL_REG1_ODR_POWERDOWN (0 << 4) // Power down mode
#define CTRL_REG1_ODR_1HZ       (1 << 4) // Normal / Low power mode (1 Hz)
#define CTRL_REG1_ODR_10HZ      (2 << 4) // Normal / Low power mode (10 Hz)
#define CTRL_REG1_ODR_25HZ      (3 << 4) // Normal / Low power mode (25 Hz)
#define CTRL_REG1_ODR_50HZ      (4 << 4) // Normal / Low power mode (50 Hz)
#define CTRL_REG1_ODR_100HZ     (5 << 4) // Normal / Low power mode (100 Hz)
#define CTRL_REG1_ODR_200HZ     (6 << 4) // Normal / Low power mode (200 Hz)
#define CTRL_REG1_ODR_400HZ     (7 << 4) // Normal / Low power mode (400 Hz)
#define CTRL_REG1_ODR_1_6KHZ    (8 << 4) // Low power mode (1.6 KHz)
#define CTRL_REG1_ODR_1_25KHZ   (9 << 4) // normal working mode (1.25 kHz) / Low power mode (5KHz)

// CTRL_REG2

#define CTRL_REG2_HPIS1   (1 << 0) // Interrupt 1 AOIFunction high pass filter enable. (0: Filter disabled; 1: filter enable)
#define CTRL_REG2_HPIS2   (1 << 1) // Interrupt 2 AOIFunction high pass filter enable. (0: Filter disabled; 1: filter enable)
#define CTRL_REG2_HPCLICK (1 << 2) // CLICK Function high pass filter enable. (0: Filter disabled; 1: filter enable)
// Data filtering options. Defaults: 0. (0: skip internal filtering; 1: The data after internal filtering is output to the data register or
// FIFO)
#define CTRL_REG2_FDS   (1 << 3)
#define CTRL_REG2_HPCF1 (1 << 4) // High pass cutoff frequency selection
#define CTRL_REG2_HPCF2 (1 << 5)
#define CTRL_REG2_HPM0  (1 << 6) // High pass mode selection, Default 00
#define CTRL_REG2_HPM1  (1 << 7)

#define CTRL_REG2_HPM_NORMAL_HPFILTER_AUTORESET (0 << 6) // Normal mode (read high-pass filter resets automatically)
#define CTRL_REG2_HPM_FILTER_REF                (1 << 6) // Filter reference signal
#define CTRL_REG2_HPM_NORMAL                    (2 << 6) // Normal mode
#define CTRL_REG2_HPM_INTERRUPT_AUTORESET       (3 << 6) // Interrupt event auto reset

// CTRL_REG3

#define CTRL_REG3_I1_OVERRUN (1 << 1) // FIFO overflow interrupt at INT1 superior. Defaults: 0 (0: prohibit; 1: Enable)
#define CTRL_REG3_I1_WTM     (1 << 2) // FIFO watermark breaks at INT1 superior. Defaults:0 (0: prohibit; 1:Enable)
#define CTRL_REG3_I1_DRDY2   (1 << 3) // DRDY2 interrupted at INT1 superior. Defaults: 0 (0: prohibit; 1: Enable)
#define CTRL_REG3_I1_DRDY1   (1 << 4) // DRDY1 interrupted at INT1 superior. Defaults: 0 (0: prohibit; 1: Enable)
#define CTRL_REG3_I1_AOI2    (1 << 5) // Enable interrupt function 2 on interrupt pin 1. Defaults: 0 (0: prohibit; 1: Enable)
#define CTRL_REG3_I1_AOI1    (1 << 6) // Enable interrupt function 1 on interrupt pin 1. Defaults: 0 (0: prohibit; 1: Enable)
#define CTRL_REG3_I1_CLICK   (1 << 7) // CLICK interrupted at INT1 superior. Defaults:0 (0: prohibit; 1: Enable)

// CTRL_REG4

#define CTRL_REG4_SIM (1 << 0) // SPI Serial interface mode configuration. Defaults: 0 (0: 4line interface; 1: 3line interface)
#define CTRL_REG4_ST0 (1 << 1) // Self-test enabled. Defaults: 00.
#define CTRL_REG4_ST1 (1 << 2)
#define CTRL_REG4_HR  (1 << 3) // High precision output mode selection. Defaults: (0: high precision prohibited; 1: high precision enable)
#define CTRL_REG4_FS0 (1 << 4) // Full range selection. Defaults: 00.
#define CTRL_REG4_FS1 (1 << 5)
// Big endian/little endian data selection. Defaults:0 (0: The low byte data is at the low address; 1: high byte data at low address)
#define CTRL_REG4_BLE (1 << 6)
// Block data update. Defaults: 0. (0: continuous update; 1: The output data register is not updated until MSB and LSB is read)
#define CTRL_REG4_BDU (1 << 7)

#define CTRL_REG4_ST_NORMAL (0 << 1)
#define CTRL_REG4_ST_TEST0  (1 << 1)
#define CTRL_REG4_ST_TEST1  (2 << 1)

#define CTRL_REG4_FS_2G  (0 << 4)
#define CTRL_REG4_FS_4G  (1 << 4)
#define CTRL_REG4_FS_8G  (2 << 4)
#define CTRL_REG4_FS_16G (3 << 4)

// CTRL_REG5

// 4D enable: in INT2 enable on pin 4D detection, while taking the interrupt2 in the configuration register 6D set 1.
#define CTRL_REG5_D4D_INT2 (1 << 0)
// Latch Interrupt2 The interrupt response specified on the configuration register. Interrupt by read 2 The configuration
// register can clear the corresponding interrupt latch signal. Defaults: 0 (0: Do not latch the interrupt signal; 1: Latch interrupt
// signal)
#define CTRL_REG5_LIR_INT2 (1 << 1)
// 4D enable: in INT1 enable on pin 4D detection, while taking the interrupt1 in the configuration register 6D set 1.
#define CTRL_REG5_D4D_INT1 (1 << 2)
// Latch Interrupt1 The interrupt response specified on the configuration register. Interrupt by read 1 The configuration
// register can clear the corresponding interrupt latch signal. Defaults: 0 (0: Do not latch the interrupt signal; 1: Latch interrupt
// signal)
#define CTRL_REG5_LIR_INT1 (1 << 3)
// FIFO Enable. Defaults: 0. (0: FIFO prohibit; 1: FIFO Enable)
#define CTRL_REG5_FIFO_EN (1 << 6)
// Override trim value. Defaults: 0. (0: normal mode; 1: Overload trim value)
#define CTRL_REG5_BOOT (1 << 7)

// CTRL_REG6

#define CTRL_REG6_H_LACTIVE (1 << 1) // 0: High level trigger interrupt; 1: Low level trigger interrupt
#define CTRL_REG6_BOOT_I2   (1 << 4) // BOOT status is INT2 superior. Defaults: 0 (0: prohibit; 1: Enable)
#define CTRL_REG6_I2_AOI2   (1 << 5) // Enable interrupt function 2 on interrupt pin 2. Defaults: 0 (0: prohibit; 1: Enable)
#define CTRL_REG6_I2_AOI1   (1 << 6) // Enable interrupt function 1 on interrupt pin 2. Defaults: 0 (0: prohibit; 1: Enable)
#define CTRL_REG6_I2_CLICK  (1 << 7) // CLICK interrupted at INT2 superior. Defaults: 0. (0: prohibit; 1: Enable)

// Status register

// STATUS_REG

// X New data for the axis arrives. Defaults: 0 (0: X The new data of the axis has not been converted; 1: X Axis new data conversion
// completed)
#define STATUS_REG_XDA (1 << 0)
// Y New data for the axis arrives. Defaults: 0 (0: Y The new data of the axis has not been converted; 1: Y Axis new data conversion
// completed)
#define STATUS_REG_YDA (1 << 1)
// Z New data for the axis arrives. Defaults: 0 (0: Z The new data of the axis has not been converted; 1: Z Axis new data conversion
// completed)
#define STATUS_REG_ZDA (1 << 2)
// X, Y and Z The new data of the three axes are all converted. Defaults: 0 (0: The data of at least one of the three axes has
// not been converted; 1: The new data of the three axes are all converted)
#define STATUS_REG_ZYXDA (1 << 3)
// X The new data of the axis has overwritten the old data. Defaults: 0. (0: X The new data of the axis has not overwritten the
// old data; 1: X The new data of the axis overwrites the old data)
#define STATUS_REG_XOR (1 << 4)
// Y The new data of the axis has overwritten the old data. Defaults: 0. (0: Y The new data of the axis has not overwritten the
// old data; 1: Y The new data of the axis overwrites the old data)
#define STATUS_REG_YOR (1 << 5)
// Z The new data of the axis has overwritten the old data. Defaults: 0. (0: Z The new data of the axis has not overwritten the
// old data; 1: Z The new data of the axis overwrites the old data)
#define STATUS_REG_ZOR (1 << 6)
// X, Y and Z At least one of the new data on the three axes has overwritten the old data. Defaults: 0 .(0: The new data of none
// of the three axes overwrites the old data; 1: The new data of at least one of the three axes has overwritten the old data)
#define STATUS_REG_ZYXOR (1 << 7)

// FIFO configuration

// FIFO_CTRL_REG

#define FIFO_CTRL_REG_BYPASS         (0 << 6) // Bypass mode
#define FIFO_CTRL_REG_FIFO           (1 << 6) // FIFO mode
#define FIFO_CTRL_REG_STREAM         (2 << 6) // Stream mode
#define FIFO_CTRL_REG_STREAM_TO_FIFO (3 << 6) // Stream to FIFO mode

// FIFO_SRC_REG

#define FIFO_SRC_REG_FSS_MASK 0x1F

#define FIFO_SRC_REG_FSS0 (1 << 0) // Contains the current number of unread samples stored in the FIFO buffer
#define FIFO_SRC_REG_FSS1 (1 << 1)
#define FIFO_SRC_REG_FSS2 (1 << 2)
#define FIFO_SRC_REG_FSS3 (1 << 3)
#define FIFO_SRC_REG_FSS4 (1 << 4)
// Set high when all FIFO samples have been read and the FIFO is empty
#define FIFO_SRC_REG_EMPTY (1 << 5)
// Set high when the FIFO buffer is full, which means that the FIFO buffer contains 32 unread samples
#define FIFO_SRC_REG_OVRN_FIFO (1 << 6)
#define FIFO_SRC_REG_WTM       (1 << 7) // Set high when FIFO content exceeds watermark level

// Interrupt configuration

// INT1_CFG

// X Axis low event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT1_CFG_XLIE_XDOWNE (1 << 0)
// X Axis high event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT1_CFG_XHIE_XUPE (1 << 1)
// Y Axis low event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT1_CFG_YLIE_YDOWNE (1 << 2)
// Y Axis high event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT1_CFG_YHIE_YUPE (1 << 3)
// Z Axis low event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT1_CFG_ZLIE_XDOWNE (1 << 4)
// Z Axis high event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT1_CFG_ZHIE_ZUPE (1 << 5)
// 6 The direction detection function is enabled. Defaults: 0. Refer to "Interrupt Mode"
#define INT1_CFG_6D (1 << 6)
// and/or interrupt events. Defaults: 0. Refer to "Interrupt Mode"
#define INT1_CFG_AOI (1 << 7)

#define INT1_CFG_6D_AOI_OR_INT_EVENT  (0 << 6) // or interrupt event
#define INT1_CFG_6D_AOI_6D_MOTION     (1 << 6) // 6 direction motion recognition
#define INT1_CFG_6D_AOI_AND_INT_EVENT (2 << 6) // and interrupt event
#define INT1_CFG_6D_AOI_6D_POSITION   (3 << 6) // 6 direction position detection

// INT1_SOURCE

#define INT1_SOURCE_XL (1 << 0) // X axis low. Defaults: 0. (0: no interruption, 1: X Axis low event has been generated)
#define INT1_SOURCE_XH (1 << 1) // X axis high. Defaults: 0. (0: no interruption, 1: X Axis high event has been generated)
#define INT1_SOURCE_YL (1 << 2) // Y axis low. Defaults: 0. (0: no interruption, 1: Y Axis low event has been generated)
#define INT1_SOURCE_YH (1 << 3) // Y axis high. Defaults: 0. (0: no interruption, 1: Y Axis high event has been generated)
#define INT1_SOURCE_ZL (1 << 4) // Z axis low. Defaults: 0. (0: no interruption, 1: Z Axis low event has been generated)
#define INT1_SOURCE_ZH (1 << 5) // Z axis high. Defaults: 0. (0: no interruption, 1: Z Axis high event has been generated)
// Interrupt activation. Defaults: 0. (0: Interrupt is not generated; 1: one or more interrupts have been generated)
#define INT1_SOURCE_IA (1 << 6)

// INT1_THS

#define INT1_THS_TH0 (1 << 0) // Interrupt 1 threshold. Defaults: 000 0000. Units: 16mg @ FS=2g, 32mg @ FS=4g, 64mg @ FS=8g, 128mg @ FS=16g
#define INT1_THS_TH1 (1 << 1)
#define INT1_THS_TH2 (1 << 2)
#define INT1_THS_TH3 (1 << 3)
#define INT1_THS_TH4 (1 << 4)
#define INT1_THS_TH5 (1 << 5)
#define INT1_THS_TH6 (1 << 6)

// INT2_DURATION

// Duration count value. Defaults: 000 0000. set recognized interrupt The minimum duration of the event. The maximum time and
// time step of the duration register is ODR for the clock.
#define INT1_DURATION_D0 (1 << 0)
#define INT1_DURATION_D1 (1 << 1)
#define INT1_DURATION_D2 (1 << 2)
#define INT1_DURATION_D3 (1 << 3)
#define INT1_DURATION_D4 (1 << 4)
#define INT1_DURATION_D5 (1 << 5)
#define INT1_DURATION_D6 (1 << 6)

// INT2_CFG
// X Axis low event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT2_CFG_XLIE_XDOWNE (1 << 0)
// X Axis high event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT2_CFG_XHIE_XUPE (1 << 1)
// Y Axis low event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT2_CFG_YLIE_YDOWNE (1 << 2)
// Y Axis high event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT2_CFG_YHIE_YUPE (1 << 3)
// Z Axis low event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT2_CFG_ZLIE_XDOWNE (1 << 4)
// Z Axis high event interrupt or Z Axis Orientation Detection Interrupt Enable. Defaults: 0 (0: disable interrupt; 1: enable interrupt)
#define INT2_CFG_ZHIE_ZUPE (1 << 5)
// 6 The direction detection function is enabled. Defaults: 0. Refer to "Interrupt Mode"
#define INT2_CFG_6D (1 << 6)
// and/or interrupt events. Defaults: 0. Refer to "Interrupt Mode"
#define INT2_CFG_AOI (1 << 7)

#define INT2_CFG_6D_AOI_OR_INT_EVENT  (0 << 6) // or interrupt event
#define INT2_CFG_6D_AOI_6D_MOTION     (1 << 6) // 6 direction motion recognition
#define INT2_CFG_6D_AOI_AND_INT_EVENT (2 << 6) // and interrupt event
#define INT2_CFG_6D_AOI_6D_POSITION   (3 << 6) // 6 direction position detection

// INT2_SOURCE

#define INT2_SOURCE_XL (1 << 0) // X axis low. Defaults: 0. (0: no interruption, 1: X Axis low event has been generated)
#define INT2_SOURCE_XH (1 << 1) // X axis high. Defaults: 0. (0: no interruption, 1: X Axis high event has been generated)
#define INT2_SOURCE_YL (1 << 2) // Y axis low. Defaults: 0. (0: no interruption, 1: Y Axis low event has been generated)
#define INT2_SOURCE_YH (1 << 3) // Y axis high. Defaults: 0. (0: no interruption, 1: Y Axis high event has been generated)
#define INT2_SOURCE_ZL (1 << 4) // Z axis low. Defaults: 0. (0: no interruption, 1: Z Axis low event has been generated)
#define INT2_SOURCE_ZH (1 << 5) // Z axis high. Defaults: 0. (0: no interruption, 1: Z Axis high event has been generated)
// Interrupt activation. Defaults: 0. (0: Interrupt is not generated; 1: one or more interrupts have been generated)
#define INT2_SOURCE_IA (1 << 6)

// INT2_THS

#define INT2_THS_TH0 (1 << 0) // Interrupt 1 threshold. Defaults: 000 0000. Units: 16mg @ FS=2g, 32mg @ FS=4g, 64mg @ FS=8g, 128mg @ FS=16g
#define INT2_THS_TH1 (1 << 1)
#define INT2_THS_TH2 (1 << 2)
#define INT2_THS_TH3 (1 << 3)
#define INT2_THS_TH4 (1 << 4)
#define INT2_THS_TH5 (1 << 5)
#define INT2_THS_TH6 (1 << 6)

// INT2_DURATION

// Duration count value. Defaults: 000 0000. set recognized interrupt The minimum duration of the event. The maximum time and
// time step of the duration register is ODR for the clock.
#define INT2_DURATION_D0 (1 << 0)
#define INT2_DURATION_D1 (1 << 1)
#define INT2_DURATION_D2 (1 << 2)
#define INT2_DURATION_D3 (1 << 3)
#define INT2_DURATION_D4 (1 << 4)
#define INT2_DURATION_D5 (1 << 5)
#define INT2_DURATION_D6 (1 << 6)

// Click configuration

// CLICK_CFG

// Enable interrupt single tap on X axis. Default value: 0. (0: disable interrupt request; 1: enable interrupt request on
// measured accel. value higher than preset threshold)
#define CLICK_CFG_XS (1 << 0)
// Enable interrupt double tap on X axis. Default value: 0. (0: disable interrupt request; 1: enable interrupt request on
// measured accel. value higher than preset threshold)
#define CLICK_CFG_XD (1 << 1)
// Enable interrupt single tap on Y axis. Default value: 0. (0: disable interrupt request; 1: enable interrupt request on
// measured accel. value higher than preset threshold)
#define CLICK_CFG_YS (1 << 2)
// Enable interrupt double tap on Y axis. Default value: 0. (0: disable interrupt request; 1: enable interrupt request on
// measured accel. value higher than preset threshold)
#define CLICK_CFG_YD (1 << 3)
// Enable interrupt single tap on Z axis. Default value: 0. (0: disable interrupt request; 1: enable interrupt request on
// measured accel. value higher than preset threshold)
#define CLICK_CFG_ZS (1 << 4)
// Enable interrupt double tap on Z axis. Default value: 0. (0: disable interrupt request; 1: enable interrupt request on
// measured accel. value higher than preset threshold)
#define CLICK_CFG_ZD (1 << 5)

// CLICK_SRC

#define CLICK_SRC_X    (1 << 0) // X Click-Click detection. Default value: 0. (0: no interrupt, 1: X High event has occurred)
#define CLICK_SRC_Y    (1 << 1) // Y Click-Click detection. Default value: 0. (0: no interrupt, 1: Y High event has occurred)
#define CLICK_SRC_Z    (1 << 2) // Z Click-Click detection. Default value: 0. (0: no interrupt, 1: Z High event has occurred)
#define CLICK_SRC_SIGN (1 << 3) // Click-Click Sign. 0: positive detection, 1: negative detection
// Single Click-Click enable. Default value: 0. (0:Single Click-Click detection disable, 1: single Click-Click detection enable)
#define CLICK_SRC_SCLICK (1 << 4)
// Single Click-Click enable. Default value: 0. (0:Single Click-Click detection disable, 1: single Click-Click detection enable)
#define CLICK_SRC_DCLICK (1 << 5)
// Interrupt active. Default value: 0. (0: no interrupt has been generated; 1: one or more interrupts have been generated)
#define CLICK_SRC_IA (1 << 6)

// CLICK_THS

#define CLICK_THS_TH0 (1 << 0) // Click-Click threshold. Default value: 000 0000
#define CLICK_THS_TH1 (1 << 1)
#define CLICK_THS_TH2 (1 << 2)
#define CLICK_THS_TH3 (1 << 3)
#define CLICK_THS_TH4 (1 << 4)
#define CLICK_THS_TH5 (1 << 5)
#define CLICK_THS_TH6 (1 << 6)
// If the LIR_Click bit is not set, the interrupt is kept high for the duration of the latency window. If the LIR_Click bit is
// set, the interrupt is kept high until CLICK_SRC (39h) is read
#define CLICK_THS_LIR_CLICK (1 << 7)

// TIME_LIMIT

#define CLICK_TIME_LIMIT_TLI0 (1 << 0) // Click-Click Time Limit. Default value: 000 0000
#define CLICK_TIME_LIMIT_TLI1 (1 << 1)
#define CLICK_TIME_LIMIT_TLI2 (1 << 2)
#define CLICK_TIME_LIMIT_TLI3 (1 << 3)
#define CLICK_TIME_LIMIT_TLI4 (1 << 4)
#define CLICK_TIME_LIMIT_TLI5 (1 << 5)
#define CLICK_TIME_LIMIT_TLI6 (1 << 6)

// TIME_LATENCY

#define CLICK_TIME_LATENCY_TLA0 (1 << 0) // Click-Click Time Latency. Default value: 000 0000
#define CLICK_TIME_LATENCY_TLA1 (1 << 1)
#define CLICK_TIME_LATENCY_TLA2 (1 << 2)
#define CLICK_TIME_LATENCY_TLA3 (1 << 3)
#define CLICK_TIME_LATENCY_TLA4 (1 << 4)
#define CLICK_TIME_LATENCY_TLA5 (1 << 5)
#define CLICK_TIME_LATENCY_TLA6 (1 << 6)
#define CLICK_TIME_LATENCY_TLA7 (1 << 7)

// TIME_WINDOW

#define CLICK_TIME_WINDOW_TW0 (1 << 0) // Click-Click Time Window
#define CLICK_TIME_WINDOW_TW1 (1 << 1)
#define CLICK_TIME_WINDOW_TW2 (1 << 2)
#define CLICK_TIME_WINDOW_TW3 (1 << 3)
#define CLICK_TIME_WINDOW_TW4 (1 << 4)
#define CLICK_TIME_WINDOW_TW5 (1 << 5)
#define CLICK_TIME_WINDOW_TW6 (1 << 6)
#define CLICK_TIME_WINDOW_TW7 (1 << 7)

// Active configuration

// ACT_THS

// Sleep to wake, return to Sleep activation threshold in Low power mode (1LSb = 16mg @FS=2g, 1LSb = 32 mg @FS=4g, 1LSb = 62 mg @FS=8g, 1LSb
// = 186 mg @FS=16g)
#define ACT_THS_ACTH0 (1 << 0)
#define ACT_THS_ACTH1 (1 << 1)
#define ACT_THS_ACTH2 (1 << 2)
#define ACT_THS_ACTH3 (1 << 3)
#define ACT_THS_ACTH4 (1 << 4)
#define ACT_THS_ACTH5 (1 << 5)
#define ACT_THS_ACTH6 (1 << 6)

// ACT_DUR

#define ACT_DUR_ACTD0 (1 << 0) // Sleep to Wake, Return to Sleep duration (1LSb = (8*1[LSb]+1)/ODR)
#define ACT_DUR_ACTD1 (1 << 1)
#define ACT_DUR_ACTD2 (1 << 2)
#define ACT_DUR_ACTD3 (1 << 3)
#define ACT_DUR_ACTD4 (1 << 4)
#define ACT_DUR_ACTD5 (1 << 5)
#define ACT_DUR_ACTD6 (1 << 6)
#define ACT_DUR_ACTD7 (1 << 7)
