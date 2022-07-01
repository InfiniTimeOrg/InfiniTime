/**
 * Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
 *
 * BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file       bma4_defs.h
 * @date       2020-05-08
 * @version    V2.14.13
 *
 */

/*! \file bma4_defs.h
 * \brief Sensor Driver for BMA4 family of sensors
 */
#ifndef BMA4_DEFS_H__
#define BMA4_DEFS_H__

/*********************************************************************/
/**\ header files */
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#endif

/*********************************************************************/
/* macro definitions */

#ifdef __KERNEL__

#if (!defined(UINT8_C) && !defined(INT8_C))
#define INT8_C(x)                                 S8_C(x)
#define UINT8_C(x)                                U8_C(x)
#endif

#if (!defined(UINT16_C) && !defined(INT16_C))
#define INT16_C(x)                                S16_C(x)
#define UINT16_C(x)                               U16_C(x)
#endif

#if (!defined(INT32_C) && !defined(UINT32_C))
#define INT32_C(x)                                S32_C(x)
#define UINT32_C(x)                               U32_C(x)
#endif

#if (!defined(INT64_C) && !defined(UINT64_C))
#define INT64_C(x)                                S64_C(x)
#define UINT64_C(x)                               U64_C(x)
#endif

#else /* __KERNEL__ */

#if (!defined(UINT8_C) && !defined(INT8_C))
#define INT8_C(x)                                 (x)
#define UINT8_C(x)                                (x##U)
#endif

#if (!defined(UINT16_C) && !defined(INT16_C))
#define INT16_C(x)                                (x)
#define UINT16_C(x)                               (x##U)
#endif

#if (!defined(INT32_C) && !defined(UINT32_C))
#define INT32_C(x)                                (x)
#define UINT32_C(x)                               (x##U)
#endif

#if (!defined(INT64_C) && !defined(UINT64_C))
#define INT64_C(x)                                (x##LL)
#define UINT64_C(x)                               (x##ULL)
#endif

#endif /* __KERNEL__ */

/**\name CHIP ID ADDRESS*/
#define BMA4_CHIP_ID_ADDR                         UINT8_C(0x00)

/**\name ERROR STATUS*/
#define BMA4_ERROR_ADDR                           UINT8_C(0X02)

/**\name STATUS REGISTER FOR SENSOR STATUS FLAG*/
#define BMA4_STATUS_ADDR                          UINT8_C(0X03)

/**\name AUX/ACCEL DATA BASE ADDRESS REGISTERS*/
#define BMA4_DATA_0_ADDR                          UINT8_C(0X0A)
#define BMA4_DATA_8_ADDR                          UINT8_C(0X12)

/**\name SENSOR TIME REGISTERS*/
#define BMA4_SENSORTIME_0_ADDR                    UINT8_C(0X18)

/**\name INTERRUPT/FEATURE STATUS REGISTERS*/
#define BMA4_INT_STAT_0_ADDR                      UINT8_C(0X1C)

/**\name INTERRUPT/FEATURE STATUS REGISTERS*/
#define BMA4_INT_STAT_1_ADDR                      UINT8_C(0X1D)

/**\name TEMPERATURE REGISTERS*/
#define BMA4_TEMPERATURE_ADDR                     UINT8_C(0X22)

/**\name FIFO REGISTERS*/
#define BMA4_FIFO_LENGTH_0_ADDR                   UINT8_C(0X24)
#define BMA4_FIFO_DATA_ADDR                       UINT8_C(0X26)

/**\name ACCEL CONFIG REGISTERS*/
#define BMA4_ACCEL_CONFIG_ADDR                    UINT8_C(0X40)

/**\name ACCEL RANGE ADDRESS*/
#define BMA4_ACCEL_RANGE_ADDR                     UINT8_C(0X41)

/**\name AUX CONFIG REGISTERS*/
#define BMA4_AUX_CONFIG_ADDR                      UINT8_C(0X44)

/**\name FIFO DOWN SAMPLING REGISTER ADDRESS FOR ACCEL*/
#define BMA4_FIFO_DOWN_ADDR                       UINT8_C(0X45)

/**\name FIFO WATERMARK REGISTER ADDRESS*/
#define BMA4_FIFO_WTM_0_ADDR                      UINT8_C(0X46)

/**\name FIFO CONFIG REGISTERS*/
#define BMA4_FIFO_CONFIG_0_ADDR                   UINT8_C(0X48)
#define BMA4_FIFO_CONFIG_1_ADDR                   UINT8_C(0X49)

/**\name MAG INTERFACE REGISTERS*/
#define BMA4_AUX_DEV_ID_ADDR                      UINT8_C(0X4B)
#define BMA4_AUX_IF_CONF_ADDR                     UINT8_C(0X4C)
#define BMA4_AUX_RD_ADDR                          UINT8_C(0X4D)
#define BMA4_AUX_WR_ADDR                          UINT8_C(0X4E)
#define BMA4_AUX_WR_DATA_ADDR                     UINT8_C(0X4F)

/**\name INTERRUPT ENABLE REGISTERS*/
#define BMA4_INT1_IO_CTRL_ADDR                    UINT8_C(0X53)
#define BMA4_INT2_IO_CTRL_ADDR                    UINT8_C(0X54)

/**\name LATCH DURATION REGISTERS*/
#define BMA4_INTR_LATCH_ADDR                      UINT8_C(0X55)

/**\name MAP INTERRUPT 1 and 2 REGISTERS*/
#define BMA4_INT_MAP_1_ADDR                       UINT8_C(0X56)
#define BMA4_INT_MAP_2_ADDR                       UINT8_C(0X57)
#define BMA4_INT_MAP_DATA_ADDR                    UINT8_C(0x58)
#define BMA4_INIT_CTRL_ADDR                       UINT8_C(0x59)

/**\name FEATURE CONFIG RELATED */
#define BMA4_RESERVED_REG_5B_ADDR                 UINT8_C(0x5B)
#define BMA4_RESERVED_REG_5C_ADDR                 UINT8_C(0x5C)
#define BMA4_FEATURE_CONFIG_ADDR                  UINT8_C(0x5E)
#define BMA4_INTERNAL_ERROR                       UINT8_C(0x5F)

/**\name SERIAL INTERFACE SETTINGS REGISTER*/
#define BMA4_IF_CONFIG_ADDR                       UINT8_C(0X6B)

/**\name SELF_TEST REGISTER*/
#define BMA4_ACC_SELF_TEST_ADDR                   UINT8_C(0X6D)

/**\name Macro to define accelerometer configuration value for FOC */
#define BMA4_FOC_ACC_CONF_VAL                     UINT8_C(0xB7)

/**\name SPI,I2C SELECTION REGISTER*/
#define BMA4_NV_CONFIG_ADDR                       UINT8_C(0x70)

/**\name ACCEL OFFSET REGISTERS*/
#define BMA4_OFFSET_0_ADDR                        UINT8_C(0X71)
#define BMA4_OFFSET_1_ADDR                        UINT8_C(0X72)
#define BMA4_OFFSET_2_ADDR                        UINT8_C(0X73)

/**\name POWER_CTRL REGISTER*/
#define BMA4_POWER_CONF_ADDR                      UINT8_C(0x7C)
#define BMA4_POWER_CTRL_ADDR                      UINT8_C(0x7D)

/**\name COMMAND REGISTER*/
#define BMA4_CMD_ADDR                             UINT8_C(0X7E)

/**\name GPIO REGISTERS*/
#define BMA4_STEP_CNT_OUT_0_ADDR                  UINT8_C(0x1E)
#define BMA4_HIGH_G_OUT_ADDR                      UINT8_C(0x1F)
#define BMA4_ACTIVITY_OUT_ADDR                    UINT8_C(0x27)
#define BMA4_ORIENTATION_OUT_ADDR                 UINT8_C(0x28)
#define BMA4_INTERNAL_STAT                        UINT8_C(0x2A)

/*!
 * @brief Block size for config write
 */
#define BMA4_BLOCK_SIZE                           UINT8_C(32)

/**\name I2C slave address */
#define BMA4_I2C_ADDR_PRIMARY                     UINT8_C(0x18)
#define BMA4_I2C_ADDR_SECONDARY                   UINT8_C(0x19)
#define BMA4_I2C_BMM150_ADDR                      UINT8_C(0x10)

/**\name Interface selection macro */
#define BMA4_SPI_WR_MASK                          UINT8_C(0x7F)
#define BMA4_SPI_RD_MASK                          UINT8_C(0x80)

/**\name Chip ID macros */
#define BMA4_CHIP_ID_MIN                          UINT8_C(0x10)
#define BMA4_CHIP_ID_MAX                          UINT8_C(0x15)

/**\name Auxiliary sensor selection macro */
#define BMM150_SENSOR                             UINT8_C(1)
#define AKM9916_SENSOR                            UINT8_C(2)
#define BMA4_ASIC_INITIALIZED                     UINT8_C(0x01)

/**\name Auxiliary sensor chip id macros */
#define BMM150_CHIP_ID                            UINT8_C(0x32)

/**\name Auxiliary sensor other macros */
#define BMM150_POWER_CONTROL_REG                  UINT8_C(0x4B)
#define BMM150_POWER_MODE_REG                     UINT8_C(0x4C)

/**\name    CONSTANTS */
#define BMA4_FIFO_CONFIG_LENGTH                   UINT8_C(2)
#define BMA4_ACCEL_CONFIG_LENGTH                  UINT8_C(2)
#define BMA4_FIFO_WM_LENGTH                       UINT8_C(2)
#define BMA4_NON_LATCH_MODE                       UINT8_C(0)
#define BMA4_LATCH_MODE                           UINT8_C(1)
#define BMA4_OPEN_DRAIN                           UINT8_C(1)
#define BMA4_PUSH_PULL                            UINT8_C(0)
#define BMA4_ACTIVE_HIGH                          UINT8_C(1)
#define BMA4_ACTIVE_LOW                           UINT8_C(0)
#define BMA4_EDGE_TRIGGER                         UINT8_C(1)
#define BMA4_LEVEL_TRIGGER                        UINT8_C(0)
#define BMA4_OUTPUT_ENABLE                        UINT8_C(1)
#define BMA4_OUTPUT_DISABLE                       UINT8_C(0)
#define BMA4_INPUT_ENABLE                         UINT8_C(1)
#define BMA4_INPUT_DISABLE                        UINT8_C(0)

/**\name ACCEL RANGE CHECK*/
#define BMA4_ACCEL_RANGE_2G                       UINT8_C(0)
#define BMA4_ACCEL_RANGE_4G                       UINT8_C(1)
#define BMA4_ACCEL_RANGE_8G                       UINT8_C(2)
#define BMA4_ACCEL_RANGE_16G                      UINT8_C(3)

/**\name  CONDITION CHECK FOR READING AND WRITING DATA*/
#define BMA4_MAX_VALUE_FIFO_FILTER                UINT8_C(1)
#define BMA4_MAX_VALUE_SPI3                       UINT8_C(1)
#define BMA4_MAX_VALUE_SELFTEST_AMP               UINT8_C(1)
#define BMA4_MAX_IF_MODE                          UINT8_C(3)
#define BMA4_MAX_VALUE_SELFTEST_SIGN              UINT8_C(1)

/**\name BUS READ AND WRITE LENGTH FOR MAG & ACCEL*/
#define BMA4_MAG_TRIM_DATA_SIZE                   UINT8_C(16)
#define BMA4_MAG_XYZ_DATA_LENGTH                  UINT8_C(6)
#define BMA4_MAG_XYZR_DATA_LENGTH                 UINT8_C(8)
#define BMA4_ACCEL_DATA_LENGTH                    UINT8_C(6)
#define BMA4_FIFO_DATA_LENGTH                     UINT8_C(2)
#define BMA4_TEMP_DATA_SIZE                       UINT8_C(1)

/**\name TEMPERATURE CONSTANT */
#define BMA4_OFFSET_TEMP                          UINT8_C(23)
#define BMA4_DEG                                  UINT8_C(1)
#define BMA4_FAHREN                               UINT8_C(2)
#define BMA4_KELVIN                               UINT8_C(3)

/**\name DELAY DEFINITION IN MSEC*/
#define BMA4_AUX_IF_DELAY                         UINT8_C(5)
#define BMA4_BMM150_WAKEUP_DELAY1                 UINT8_C(2)
#define BMA4_BMM150_WAKEUP_DELAY2                 UINT8_C(3)
#define BMA4_BMM150_WAKEUP_DELAY3                 UINT8_C(1)
#define BMA4_GEN_READ_WRITE_DELAY                 UINT16_C(1000)
#define BMA4_AUX_COM_DELAY                        UINT16_C(10000)

/**\name    ARRAY PARAMETER DEFINITIONS*/
#define BMA4_SENSOR_TIME_MSB_BYTE                 UINT8_C(2)
#define BMA4_SENSOR_TIME_XLSB_BYTE                UINT8_C(1)
#define BMA4_SENSOR_TIME_LSB_BYTE                 UINT8_C(0)
#define BMA4_MAG_X_LSB_BYTE                       UINT8_C(0)
#define BMA4_MAG_X_MSB_BYTE                       UINT8_C(1)
#define BMA4_MAG_Y_LSB_BYTE                       UINT8_C(2)
#define BMA4_MAG_Y_MSB_BYTE                       UINT8_C(3)
#define BMA4_MAG_Z_LSB_BYTE                       UINT8_C(4)
#define BMA4_MAG_Z_MSB_BYTE                       UINT8_C(5)
#define BMA4_MAG_R_LSB_BYTE                       UINT8_C(6)
#define BMA4_MAG_R_MSB_BYTE                       UINT8_C(7)
#define BMA4_TEMP_BYTE                            UINT8_C(0)
#define BMA4_FIFO_LENGTH_MSB_BYTE                 UINT8_C(1)

/*! @name To define success code */
#define BMA4_OK                                   INT8_C(0)

/*! @name To define error codes */
#define BMA4_E_NULL_PTR                           INT8_C(-1)
#define BMA4_E_COM_FAIL                           INT8_C(-2)
#define BMA4_E_DEV_NOT_FOUND                      INT8_C(-3)
#define BMA4_E_INVALID_SENSOR                     INT8_C(-4)
#define BMA4_E_CONFIG_STREAM_ERROR                INT8_C(-5)
#define BMA4_E_SELF_TEST_FAIL                     INT8_C(-6)
#define BMA4_E_FOC_FAIL                           INT8_C(-7)
#define BMA4_E_OUT_OF_RANGE                       INT8_C(-8)
#define BMA4_E_INT_LINE_INVALID                   INT8_C(-9)
#define BMA4_E_RD_WR_LENGTH_INVALID               INT8_C(-10)
#define BMA4_E_AUX_CONFIG_FAIL                    INT8_C(-11)
#define BMA4_E_SC_FIFO_HEADER_ERR                 INT8_C(-12)
#define BMA4_E_SC_FIFO_CONFIG_ERR                 INT8_C(-13)

/**\name    UTILITY MACROS  */
#define BMA4_SET_LOW_BYTE                         UINT16_C(0x00FF)
#define BMA4_SET_HIGH_BYTE                        UINT16_C(0xFF00)
#define BMA4_SET_LOW_NIBBLE                       UINT8_C(0x0F)

/* Macros used for Self test (BMA42X_VARIANT) */
/* Self-test: Resulting minimum difference signal in mg for BMA42X */
#define BMA42X_ST_ACC_X_AXIS_SIGNAL_DIFF          UINT16_C(400)
#define BMA42X_ST_ACC_Y_AXIS_SIGNAL_DIFF          UINT16_C(800)
#define BMA42X_ST_ACC_Z_AXIS_SIGNAL_DIFF          UINT16_C(400)

/* Macros used for Self test (BMA42X_B_VARIANT) */
/* Self-test: Resulting minimum difference signal in mg for BMA42X_B */
#define BMA42X_B_ST_ACC_X_AXIS_SIGNAL_DIFF        UINT16_C(1800)
#define BMA42X_B_ST_ACC_Y_AXIS_SIGNAL_DIFF        UINT16_C(1800)
#define BMA42X_B_ST_ACC_Z_AXIS_SIGNAL_DIFF        UINT16_C(1800)

/* Macros used for Self test (BMA45X_VARIANT) */
/* Self-test: Resulting minimum difference signal in mg for BMA45X */
#define BMA45X_ST_ACC_X_AXIS_SIGNAL_DIFF          UINT16_C(1800)
#define BMA45X_ST_ACC_Y_AXIS_SIGNAL_DIFF          UINT16_C(1800)
#define BMA45X_ST_ACC_Z_AXIS_SIGNAL_DIFF          UINT16_C(1800)

/**\name BOOLEAN TYPES*/
#ifndef TRUE
#define TRUE                                      UINT8_C(0x01)
#endif

#ifndef FALSE
#define FALSE                                     UINT8_C(0x00)
#endif

#ifndef NULL
#define NULL                                      UINT8_C(0x00)
#endif

/**\name    ERROR STATUS POSITION AND MASK*/
#define BMA4_FATAL_ERR_MSK                        UINT8_C(0x01)
#define BMA4_CMD_ERR_POS                          UINT8_C(1)
#define BMA4_CMD_ERR_MSK                          UINT8_C(0x02)
#define BMA4_ERR_CODE_POS                         UINT8_C(2)
#define BMA4_ERR_CODE_MSK                         UINT8_C(0x1C)
#define BMA4_FIFO_ERR_POS                         UINT8_C(6)
#define BMA4_FIFO_ERR_MSK                         UINT8_C(0x40)
#define BMA4_AUX_ERR_POS                          UINT8_C(7)
#define BMA4_AUX_ERR_MSK                          UINT8_C(0x80)

/**\name    NV_CONFIG POSITION AND MASK*/
/* NV_CONF Description - Reg Addr --> (0x70), Bit --> 3 */
#define BMA4_NV_ACCEL_OFFSET_POS                  UINT8_C(3)
#define BMA4_NV_ACCEL_OFFSET_MSK                  UINT8_C(0x08)

/**\name    MAG DATA XYZ POSITION AND MASK*/
#define BMA4_DATA_MAG_X_LSB_POS                   UINT8_C(3)
#define BMA4_DATA_MAG_X_LSB_MSK                   UINT8_C(0xF8)
#define BMA4_DATA_MAG_Y_LSB_POS                   UINT8_C(3)
#define BMA4_DATA_MAG_Y_LSB_MSK                   UINT8_C(0xF8)
#define BMA4_DATA_MAG_Z_LSB_POS                   UINT8_C(1)
#define BMA4_DATA_MAG_Z_LSB_MSK                   UINT8_C(0xFE)
#define BMA4_DATA_MAG_R_LSB_POS                   UINT8_C(2)
#define BMA4_DATA_MAG_R_LSB_MSK                   UINT8_C(0xFC)

/**\name ACCEL DATA READY POSITION AND MASK*/
#define BMA4_STAT_DATA_RDY_ACCEL_POS              UINT8_C(7)
#define BMA4_STAT_DATA_RDY_ACCEL_MSK              UINT8_C(0x80)

/**\name MAG DATA READY POSITION AND MASK*/
#define BMA4_STAT_DATA_RDY_MAG_POS                UINT8_C(5)
#define BMA4_STAT_DATA_RDY_MAG_MSK                UINT8_C(0x20)

/**\name ADVANCE POWER SAVE POSITION AND MASK*/
#define BMA4_ADVANCE_POWER_SAVE_MSK               UINT8_C(0x01)

/**\name ACCELEROMETER ENABLE POSITION AND MASK*/
#define BMA4_ACCEL_ENABLE_POS                     UINT8_C(2)
#define BMA4_ACCEL_ENABLE_MSK                     UINT8_C(0x04)

/**\name MAGNETOMETER ENABLE POSITION AND MASK*/
#define BMA4_MAG_ENABLE_MSK                       UINT8_C(0x01)

/**\name    ACCEL CONFIGURATION POSITION AND MASK*/
#define BMA4_ACCEL_ODR_MSK                        UINT8_C(0x0F)
#define BMA4_ACCEL_BW_POS                         UINT8_C(4)
#define BMA4_ACCEL_BW_MSK                         UINT8_C(0x70)
#define BMA4_ACCEL_RANGE_MSK                      UINT8_C(0x03)
#define BMA4_ACCEL_PERFMODE_POS                   UINT8_C(7)
#define BMA4_ACCEL_PERFMODE_MSK                   UINT8_C(0x80)

/**\name    MAG CONFIGURATION POSITION AND MASK*/
#define BMA4_MAG_CONFIG_OFFSET_POS                UINT8_C(4)
#define BMA4_MAG_CONFIG_OFFSET_LEN                UINT8_C(4)
#define BMA4_MAG_CONFIG_OFFSET_MSK                UINT8_C(0xF0)
#define BMA4_MAG_CONFIG_OFFSET_REG                (BMA4_AUX_CONFIG_ADDR)

/**\name FIFO SELF WAKE UP POSITION AND MASK*/
#define BMA4_FIFO_SELF_WAKE_UP_POS                UINT8_C(1)
#define BMA4_FIFO_SELF_WAKE_UP_MSK                UINT8_C(0x02)

/**\name    FIFO BYTE COUNTER POSITION AND MASK*/
#define BMA4_FIFO_BYTE_COUNTER_MSB_MSK            UINT8_C(0x3F)

/**\name    FIFO DATA POSITION AND MASK*/
#define BMA4_FIFO_DATA_POS                        UINT8_C(0)
#define BMA4_FIFO_DATA_MSK                        UINT8_C(0xFF)

/**\name    FIFO FILTER FOR ACCEL  POSITION AND MASK*/
#define BMA4_FIFO_DOWN_ACCEL_POS                  UINT8_C(4)
#define BMA4_FIFO_DOWN_ACCEL_MSK                  UINT8_C(0x70)
#define BMA4_FIFO_FILTER_ACCEL_POS                UINT8_C(7)
#define BMA4_FIFO_FILTER_ACCEL_MSK                UINT8_C(0x80)

/**\name    FIFO HEADER DATA DEFINITIONS    */
#define BMA4_FIFO_HEAD_A                          UINT8_C(0x84)
#define BMA4_FIFO_HEAD_M                          UINT8_C(0x90)
#define BMA4_FIFO_HEAD_M_A                        UINT8_C(0x94)
#define BMA4_FIFO_HEAD_SENSOR_TIME                UINT8_C(0x44)
#define BMA4_FIFO_HEAD_INPUT_CONFIG               UINT8_C(0x48)
#define BMA4_FIFO_HEAD_SKIP_FRAME                 UINT8_C(0x40)
#define BMA4_FIFO_HEAD_OVER_READ_MSB              UINT8_C(0x80)
#define BMA4_FIFO_HEAD_SAMPLE_DROP                UINT8_C(0x50)

/**\name    FIFO HEADERLESS MODE DATA ENABLE DEFINITIONS   */
#define BMA4_FIFO_M_A_ENABLE                      UINT8_C(0x60)
#define BMA4_FIFO_A_ENABLE                        UINT8_C(0x40)
#define BMA4_FIFO_M_ENABLE                        UINT8_C(0x20)

/**\name    FIFO CONFIGURATION SELECTION    */
#define BMA4_FIFO_STOP_ON_FULL                    UINT8_C(0x01)
#define BMA4_FIFO_TIME                            UINT8_C(0x02)
#define BMA4_FIFO_TAG_INTR2                       UINT8_C(0x04)
#define BMA4_FIFO_TAG_INTR1                       UINT8_C(0x08)
#define BMA4_FIFO_HEADER                          UINT8_C(0x10)
#define BMA4_FIFO_MAG                             UINT8_C(0x20)
#define BMA4_FIFO_ACCEL                           UINT8_C(0x40)
#define BMA4_FIFO_ALL                             UINT8_C(0x7F)
#define BMA4_FIFO_CONFIG_0_MASK                   UINT8_C(0x03)
#define BMA4_FIFO_CONFIG_1_MASK                   UINT8_C(0xFC)

/**\name    FIFO FRAME COUNT DEFINITION     */
#define BMA4_FIFO_LSB_CONFIG_CHECK                UINT8_C(0x00)
#define BMA4_FIFO_MSB_CONFIG_CHECK                UINT8_C(0x80)
#define BMA4_FIFO_TAG_INTR_MASK                   UINT8_C(0xFC)

/**\name    FIFO DROPPED FRAME DEFINITION     */
#define BMA4_AUX_FIFO_DROP                        UINT8_C(0x04)
#define BMA4_ACCEL_AUX_FIFO_DROP                  UINT8_C(0x05)
#define BMA4_ACCEL_FIFO_DROP                      UINT8_C(0x01)

/**\name FIFO MAG DEFINITION*/
#define BMA4_MA_FIFO_A_X_LSB                      UINT8_C(8)

/**\name FIFO sensor time length definitions*/
#define BMA4_SENSOR_TIME_LENGTH                   UINT8_C(3)

/**\name FIFO LENGTH DEFINITION*/
#define BMA4_FIFO_A_LENGTH                        UINT8_C(6)
#define BMA4_FIFO_M_LENGTH                        UINT8_C(8)
#define BMA4_FIFO_MA_LENGTH                       UINT8_C(14)

/**\name    MAG I2C ADDRESS SELECTION POSITION AND MASK*/
#define BMA4_I2C_DEVICE_ADDR_POS                  UINT8_C(1)
#define BMA4_I2C_DEVICE_ADDR_MSK                  UINT8_C(0xFE)

/**\name MAG CONFIGURATION FOR SECONDARY INTERFACE POSITION AND MASK*/
#define BMA4_MAG_BURST_MSK                        UINT8_C(0x03)
#define BMA4_MAG_MANUAL_ENABLE_POS                UINT8_C(7)
#define BMA4_MAG_MANUAL_ENABLE_MSK                UINT8_C(0x80)
#define BMA4_READ_ADDR_MSK                        UINT8_C(0xFF)
#define BMA4_WRITE_ADDR_MSK                       UINT8_C(0xFF)
#define BMA4_WRITE_DATA_MSK                       UINT8_C(0xFF)

/**\name    OUTPUT TYPE ENABLE POSITION AND MASK*/
#define BMA4_INT_EDGE_CTRL_MASK                   UINT8_C(0x01)
#define BMA4_INT_EDGE_CTRL_POS                    UINT8_C(0x00)
#define BMA4_INT_LEVEL_MASK                       UINT8_C(0x02)
#define BMA4_INT_LEVEL_POS                        UINT8_C(0x01)
#define BMA4_INT_OPEN_DRAIN_MASK                  UINT8_C(0x04)
#define BMA4_INT_OPEN_DRAIN_POS                   UINT8_C(0x02)
#define BMA4_INT_OUTPUT_EN_MASK                   UINT8_C(0x08)
#define BMA4_INT_OUTPUT_EN_POS                    UINT8_C(0x03)
#define BMA4_INT_INPUT_EN_MASK                    UINT8_C(0x10)
#define BMA4_INT_INPUT_EN_POS                     UINT8_C(0x04)

/**\name    IF CONFIG POSITION AND MASK*/
#define BMA4_CONFIG_SPI3_MSK                      UINT8_C(0x01)
#define BMA4_IF_CONFIG_IF_MODE_POS                UINT8_C(4)
#define BMA4_IF_CONFIG_IF_MODE_MSK                UINT8_C(0x10)

/**\name    ACCEL SELF TEST POSITION AND MASK*/
#define BMA4_ACCEL_SELFTEST_ENABLE_MSK            UINT8_C(0x01)
#define BMA4_ACCEL_SELFTEST_SIGN_POS              UINT8_C(2)
#define BMA4_ACCEL_SELFTEST_SIGN_MSK              UINT8_C(0x04)
#define BMA4_SELFTEST_AMP_POS                     UINT8_C(3)
#define BMA4_SELFTEST_AMP_MSK                     UINT8_C(0x08)

/**\name    ACCEL ODR          */
#define BMA4_OUTPUT_DATA_RATE_0_78HZ              UINT8_C(0x01)
#define BMA4_OUTPUT_DATA_RATE_1_56HZ              UINT8_C(0x02)
#define BMA4_OUTPUT_DATA_RATE_3_12HZ              UINT8_C(0x03)
#define BMA4_OUTPUT_DATA_RATE_6_25HZ              UINT8_C(0x04)
#define BMA4_OUTPUT_DATA_RATE_12_5HZ              UINT8_C(0x05)
#define BMA4_OUTPUT_DATA_RATE_25HZ                UINT8_C(0x06)
#define BMA4_OUTPUT_DATA_RATE_50HZ                UINT8_C(0x07)
#define BMA4_OUTPUT_DATA_RATE_100HZ               UINT8_C(0x08)
#define BMA4_OUTPUT_DATA_RATE_200HZ               UINT8_C(0x09)
#define BMA4_OUTPUT_DATA_RATE_400HZ               UINT8_C(0x0A)
#define BMA4_OUTPUT_DATA_RATE_800HZ               UINT8_C(0x0B)
#define BMA4_OUTPUT_DATA_RATE_1600HZ              UINT8_C(0x0C)

/**\name    ACCEL BANDWIDTH PARAMETER         */
#define BMA4_ACCEL_OSR4_AVG1                      UINT8_C(0)
#define BMA4_ACCEL_OSR2_AVG2                      UINT8_C(1)
#define BMA4_ACCEL_NORMAL_AVG4                    UINT8_C(2)
#define BMA4_ACCEL_CIC_AVG8                       UINT8_C(3)
#define BMA4_ACCEL_RES_AVG16                      UINT8_C(4)
#define BMA4_ACCEL_RES_AVG32                      UINT8_C(5)
#define BMA4_ACCEL_RES_AVG64                      UINT8_C(6)
#define BMA4_ACCEL_RES_AVG128                     UINT8_C(7)

/**\name    ACCEL PERFMODE PARAMETER         */
#define BMA4_CIC_AVG_MODE                         UINT8_C(0)
#define BMA4_CONTINUOUS_MODE                      UINT8_C(1)

/**\name    MAG OFFSET         */
#define BMA4_MAG_OFFSET_MAX                       UINT8_C(0x00)

/**\name    ENABLE/DISABLE SELECTIONS        */
#define BMA4_X_AXIS                               UINT8_C(0)
#define BMA4_Y_AXIS                               UINT8_C(1)
#define BMA4_Z_AXIS                               UINT8_C(2)

/**\name SELF TEST*/
#define BMA4_SELFTEST_PASS                        INT8_C(0)
#define BMA4_SELFTEST_FAIL                        INT8_C(1)

#define BMA4_SELFTEST_DIFF_X_AXIS_FAILED          INT8_C(1)
#define BMA4_SELFTEST_DIFF_Y_AXIS_FAILED          INT8_C(2)
#define BMA4_SELFTEST_DIFF_Z_AXIS_FAILED          INT8_C(3)
#define BMA4_SELFTEST_DIFF_X_AND_Y_AXIS_FAILED    INT8_C(4)
#define BMA4_SELFTEST_DIFF_X_AND_Z_AXIS_FAILED    INT8_C(5)
#define BMA4_SELFTEST_DIFF_Y_AND_Z_AXIS_FAILED    INT8_C(6)
#define BMA4_SELFTEST_DIFF_X_Y_AND_Z_AXIS_FAILED  INT8_C(7)

/**\name INTERRUPT MAPS    */
#define BMA4_INTR1_MAP                            UINT8_C(0)
#define BMA4_INTR2_MAP                            UINT8_C(1)

/**\name    INTERRUPT MASKS        */
#define BMA4_FIFO_FULL_INT                        UINT16_C(0x0100)
#define BMA4_FIFO_WM_INT                          UINT16_C(0x0200)
#define BMA4_DATA_RDY_INT                         UINT16_C(0x0400)
#define BMA4_MAG_DATA_RDY_INT                     UINT16_C(0x2000)
#define BMA4_ACCEL_DATA_RDY_INT                   UINT16_C(0x8000)

/**\name    AKM POWER MODE SELECTION     */
#define BMA4_AKM_POWER_DOWN_MODE                  UINT8_C(0)
#define BMA4_AKM_SINGLE_MEAS_MODE                 UINT8_C(1)

/**\name    SECONDARY_MAG POWER MODE SELECTION    */
#define BMA4_MAG_FORCE_MODE                       UINT8_C(0)
#define BMA4_MAG_SUSPEND_MODE                     UINT8_C(1)

/**\name    MAG POWER MODE SELECTION    */
#define BMA4_FORCE_MODE                           UINT8_C(0)
#define BMA4_SUSPEND_MODE                         UINT8_C(1)

/**\name    ACCEL POWER MODE    */
#define BMA4_ACCEL_MODE_NORMAL                    UINT8_C(0x11)

/**\name    MAG POWER MODE    */
#define BMA4_MAG_MODE_SUSPEND                     UINT8_C(0x18)

/**\name    ENABLE/DISABLE BIT VALUES    */
#define BMA4_ENABLE                               UINT8_C(0x01)
#define BMA4_DISABLE                              UINT8_C(0x00)

/**\name    DEFINITION USED FOR DIFFERENT WRITE   */
#define BMA4_MANUAL_DISABLE                       UINT8_C(0x00)
#define BMA4_MANUAL_ENABLE                        UINT8_C(0x01)
#define BMA4_ENABLE_MAG_IF_MODE                   UINT8_C(0x01)
#define BMA4_MAG_DATA_READ_REG                    UINT8_C(0x0A)
#define BMA4_BMM_POWER_MODE_REG                   UINT8_C(0x06)
#define BMA4_SEC_IF_NULL                          UINT8_C(0)
#define BMA4_SEC_IF_BMM150                        UINT8_C(1)
#define BMA4_SEC_IF_AKM09916                      UINT8_C(2)
#define BMA4_ENABLE_AUX_IF_MODE                   UINT8_C(0x01)

/**\name    SENSOR RESOLUTION   */
#define BMA4_12_BIT_RESOLUTION                    UINT8_C(12)
#define BMA4_14_BIT_RESOLUTION                    UINT8_C(14)
#define BMA4_16_BIT_RESOLUTION                    UINT8_C(16)

/**\name FOC MACROS */
#define BMA4_FOC_SAMPLE_LIMIT                     UINT8_C(128)
#define BMA4_MAX_NOISE_LIMIT(RANGE_VALUE)         (RANGE_VALUE + UINT16_C(255))
#define BMA4_MIN_NOISE_LIMIT(RANGE_VALUE)         (RANGE_VALUE - UINT16_C(255))

/*! for handling float temperature values */
#define BMA4_SCALE_TEMP                           INT32_C(1000)

/* BMA4_FAHREN_SCALED = 1.8 * 1000 */
#define BMA4_FAHREN_SCALED                        INT32_C(1800)

/* BMA4_KELVIN_SCALED = 273.15 * 1000 */
#define BMA4_KELVIN_SCALED                        INT32_C(273150)

/**\name    MAP BURST READ LENGTHS  */
#define BMA4_AUX_READ_LEN_0                       UINT8_C(0)
#define BMA4_AUX_READ_LEN_1                       UINT8_C(1)
#define BMA4_AUX_READ_LEN_2                       UINT8_C(2)
#define BMA4_AUX_READ_LEN_3                       UINT8_C(3)

#define BMA4_CONFIG_STREAM_MESSAGE_MSK            UINT8_C(0x0F)

#define BMA4_SOFT_RESET                           UINT8_C(0XB6)

#define BMA4_MS_TO_US(X)                          (X * 1000)

#ifndef ABS
#define ABS(a)                                    ((a) > 0 ? (a) : -(a)) /*!< Absolute value */
#endif

/**\name    BIT SLICE GET AND SET FUNCTIONS */
#define BMA4_GET_BITSLICE(regvar, bitname) \
    ((regvar & bitname##_MSK) >> bitname##_POS)

#define BMA4_SET_BITSLICE(regvar, bitname, val) \
    ((regvar & ~bitname##_MSK) | \
     ((val << bitname##_POS) & bitname##_MSK))

#define BMA4_GET_DIFF(x, y)                       ((x) - (y))

#define BMA4_GET_LSB(var)                         (uint8_t)(var & BMA4_SET_LOW_BYTE)
#define BMA4_GET_MSB(var)                         (uint8_t)((var & BMA4_SET_HIGH_BYTE) >> 8)

#define BMA4_SET_BIT_VAL_0(reg_data, bitname)     (reg_data & ~(bitname##_MSK))

#define BMA4_SET_BITS_POS_0(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) | \
     (data & bitname##_MSK))

#define BMA4_GET_BITS_POS_0(reg_data, bitname)    (reg_data & (bitname##_MSK))

/**
 * BMA4_INTF_RET_TYPE is the read/write interface return type which can be overwritten by the build system.
 * The default is set to int8_t.
 */
#ifndef BMA4_INTF_RET_TYPE
#define BMA4_INTF_RET_TYPE                        int8_t
#endif

/**
 * BST_INTF_RET_SUCCESS is the success return value read/write interface return type which can be
 * overwritten by the build system. The default is set to 0. It is used to check for a successful
 * execution of the read/write functions
 */
#ifndef BMA4_INTF_RET_SUCCESS
#define BMA4_INTF_RET_SUCCESS                     INT8_C(0)
#endif

/******************************************************************************/
/*!  @name         TYPEDEF DEFINITIONS                                        */
/******************************************************************************/

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific read functions of the user
 *
 * @param[in] reg_addr       : Register address from which data is read.
 * @param[out] read_data     : Pointer to data buffer where read data is stored.
 * @param[in] len            : Number of bytes of data to be read.
 * @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                             for interface related call backs.
 *
 *  @retval = BMA4_INTF_RET_SUCCESS -> Success
 *  @retval != BMA4_INTF_RET_SUCCESS  -> Failure Info
 *
 */
typedef BMA4_INTF_RET_TYPE (*bma4_read_fptr_t)(uint8_t reg_addr, uint8_t *read_data, uint32_t len, void *intf_ptr);

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific write functions of the user
 *
 * @param[in] reg_addr      : Register address to which the data is written.
 * @param[in] read_data     : Pointer to data buffer in which data to be written
 *                            is stored.
 * @param[in] len           : Number of bytes of data to be written.
 * @param[in, out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                            for interface related call backs
 *
 *  @retval = BMA4_INTF_RET_SUCCESS -> Success
 *  @retval != BMA4_INTF_RET_SUCCESS  -> Failure Info
 *
 */
typedef BMA4_INTF_RET_TYPE (*bma4_write_fptr_t)(uint8_t reg_addr, const uint8_t *read_data, uint32_t len,
                                                void *intf_ptr);

/*!
 * @brief Delay function pointer which should be mapped to
 * delay function of the user
 *
 * @param[in] period              : Delay in microseconds.
 * @param[in, out] intf_ptr       : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *
 */
typedef void (*bma4_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

/******************************************************************************/
/*!  @name         Enum Declarations                                  */
/******************************************************************************/
/*!  @name Enum to define BMA4 variants */
enum  bma4_variant {
    BMA42X_VARIANT = 1,
    BMA42X_B_VARIANT,
    BMA45X_VARIANT
};

/* Enumerator describing interfaces */
enum bma4_intf {
    BMA4_SPI_INTF,
    BMA4_I2C_INTF
};

/**\name    STRUCTURE DEFINITIONS*/

/*!
 *  @brief
 *  This structure holds asic info. for feature configuration.
 */
struct bma4_asic_data
{
    /* Feature config start addr (0-3 bits)*/
    uint8_t asic_lsb;

    /* Feature config start addr (4-11 bits)*/
    uint8_t asic_msb;
};

/*!
 * @brief Auxiliary configuration structure for user settings
 */
struct bma4_aux_config
{
    /*! Device address of auxiliary sensor */
    uint8_t aux_dev_addr;

    /*! To enable manual or auto mode */
    uint8_t manual_enable;

    /*! No of bytes to be read at a time */
    uint8_t burst_read_length;

    /*! Variable to set the auxiliary interface */
    uint8_t if_mode;
};

/*!
 *  @brief
 *  This structure holds all relevant information about BMA4
 */
struct bma4_dev
{
    /*! Chip id of BMA4 */
    uint8_t chip_id;

    /*! Chip id of auxiliary sensor */
    uint8_t aux_chip_id;

    /*! Interface pointer */
    void *intf_ptr;

    /*! Interface detail */
    enum bma4_intf intf;

    /*! Variable that holds error code */
    BMA4_INTF_RET_TYPE intf_rslt;

    /*! Auxiliary sensor information */
    uint8_t aux_sensor;

    /*! Decide SPI or I2C read mechanism */
    uint8_t dummy_byte;

    /*! Resolution for FOC */
    uint8_t resolution;

    /*! Define the BMA4 variant BMA42X or BMA45X */
    enum bma4_variant variant;

    /*! Used to check mag manual/auto mode status
     * int8_t mag_manual_enable;
     */

    /*! Config stream data buffer address will be assigned*/
    const uint8_t *config_file_ptr;

    /*! Read/write length */
    uint16_t read_write_len;

    /*! Feature len */
    uint8_t feature_len;

    /*! Contains asic information */
    struct bma4_asic_data asic_data;

    /*! Contains aux configuration settings */
    struct bma4_aux_config aux_config;

    /*! Bus read function pointer */
    bma4_read_fptr_t bus_read;

    /*! Bus write function pointer */
    bma4_write_fptr_t bus_write;

    /*! Delay(in microsecond) function pointer */
    bma4_delay_us_fptr_t delay_us;

    /*! Variable to store the size of config file */
    uint16_t config_size;

    /*! Variable to store the status of performance mode */
    uint8_t perf_mode_status;
};

/*!
 *  @brief This structure holds the information for usage of
 *  FIFO by the user.
 */
struct bma4_fifo_frame
{
    /*! Data buffer of user defined length is to be mapped here */
    uint8_t *data;

    /*! Number of bytes of FIFO to be read as specified by the user */
    uint16_t length;

    /*! Enabling of the FIFO header to stream in header mode */
    uint8_t fifo_header_enable;

    /*! Streaming of the Accelerometer, Auxiliary
     * sensor data or both in FIFO
     */
    uint8_t fifo_data_enable;

    /*! Will be equal to length when no more frames are there to parse */
    uint16_t accel_byte_start_idx;

    /*! Will be equal to length when no more frames are there to parse */
    uint16_t mag_byte_start_idx;

    /*! Will be equal to length when no more frames are there to parse */
    uint16_t sc_frame_byte_start_idx;

    /*! Value of FIFO sensor time time */
    uint32_t sensor_time;

    /*! Value of Skipped frame counts */
    uint8_t skipped_frame_count;

    /*! Value of accel dropped frame count */
    uint8_t accel_dropped_frame_count;

    /*! Value of mag dropped frame count */
    uint8_t mag_dropped_frame_count;
};

/*!
 *  @brief Error Status structure
 */
struct bma4_err_reg
{
    /*! Indicates fatal error */
    uint8_t fatal_err;

    /*! Indicates command error */
    uint8_t cmd_err;

    /*! Indicates error code */
    uint8_t err_code;

    /*! Indicates fifo error */
    uint8_t fifo_err;

    /*! Indicates mag error */
    uint8_t aux_err;
};

/*!
 * @brief Asic Status structure
 */
struct bma4_asic_status
{
    /*! Asic is in sleep/halt state */
    uint8_t sleep;

    /*! Dedicated interrupt is set again before previous interrupt
     * was acknowledged
     */
    uint8_t irq_ovrn;

    /*! Watchcell event detected (asic stopped) */
    uint8_t wc_event;

    /*! Stream transfer has started and transactions are ongoing */
    uint8_t stream_transfer_active;
};

/*!
 * @brief Interrupt Pin Configuration structure
 */
struct  bma4_int_pin_config
{
    /*! Trigger condition of interrupt pin */
    uint8_t edge_ctrl;

    /*! Level of interrupt pin */
    uint8_t lvl;

    /*! Behaviour of interrupt pin to open drain */
    uint8_t od;

    /*! Output enable for interrupt pin */
    uint8_t output_en;

    /*! Input enable for interrupt pin */
    uint8_t input_en;
};

/*!
 * @brief Accelerometer configuration structure
 */
struct bma4_accel_config
{
    /*! Output data rate in Hz */
    uint8_t odr;

    /*! Bandwidth parameter, determines filter configuration */
    uint8_t bandwidth;

    /*! Filter performance mode */
    uint8_t perf_mode;

    /*! G-range */
    uint8_t range;
};

/*!
 * @brief Auxiliary magnetometer configuration structure
 */
struct bma4_aux_mag_config
{
    /*! Poll rate for the sensor attached to the Magnetometer interface */
    uint8_t odr;

    /*! Trigger-readout offset in units of 2.5 ms.
     * If set to zero, the offset is maximum, i.e. after readout a trigger
     * is issued immediately
     */
    uint8_t offset;
};

/*!
 * @brief ASIC Config structure
 */
struct bma4_asic_config
{
    /*! Enable/Disable ASIC Wake Up */
    uint8_t asic_en;

    /*! Configure stream_transfer/FIFO mode */
    uint8_t fifo_mode_en;

    /*! Mapping of instance RAM1 */
    uint8_t mem_conf_ram1;

    /*! Mapping of instance RAM2 */
    uint8_t mem_conf_ram2;

    /*! Mapping of instance RAM3 */
    uint8_t mem_conf_ram3;
};

/*!
 * @brief bmm150 or akm09916
 *  magnetometer values structure
 */
struct bma4_mag
{
    /*! BMM150 and AKM09916 X raw data */
    int32_t x;

    /*! BMM150 and AKM09916 Y raw data */
    int32_t y;

    /*! BMM150 and AKM09916 Z raw data */
    int32_t z;
};

/*!
 * @brief bmm150 xyz data structure
 */
struct bma4_mag_xyzr
{
    /*! BMM150 X raw data */
    int16_t x;

    /*! BMM150 Y raw data */
    int16_t y;

    /*! BMM150 Z raw data */
    int16_t z;

    /*! BMM150 R raw data */
    uint16_t r;
};

/*!
 * @brief Accel xyz data structure
 */
struct bma4_accel
{
    /*! Accel X data */
    int16_t x;

    /*! Accel Y data */
    int16_t y;

    /*! Accel Z data */
    int16_t z;
};

/*!
 * @brief FIFO mag data structure
 */
struct bma4_mag_fifo_data
{
    /*! The value of mag x LSB data */
    uint8_t mag_x_lsb;

    /*! The value of mag x MSB data */
    uint8_t mag_x_msb;

    /*! The value of mag y LSB data */
    uint8_t mag_y_lsb;

    /*! The value of mag y MSB data */
    uint8_t mag_y_msb;

    /*! The value of mag z LSB data */
    uint8_t mag_z_lsb;

    /*! The value of mag z MSB data */
    uint8_t mag_z_msb;

    /*! The value of mag r for BMM150 Y2 for YAMAHA LSB data */
    uint8_t mag_r_y2_lsb;

    /*! The value of mag r for BMM150 Y2 for YAMAHA MSB data */
    uint8_t mag_r_y2_msb;
};

/*!
 * @brief Accel self test difference data structure
 */
struct bma4_selftest_delta_limit
{
    /*! Accel X  data */
    int32_t x;

    /*! Accel Y  data */
    int32_t y;

    /*! Accel Z  data */
    int32_t z;
};

/*!  @name Structure to enable an accel axis for FOC */
struct bma4_accel_foc_g_value
{
    /* '0' to disable x-axis and '1' to enable x-axis */
    uint8_t x;

    /* '0' to disable y-axis and '1' to enable y-axis */
    uint8_t y;

    /* '0' to disable z-axis and '1' to enable z-axis */
    uint8_t z;

    /* '0' for positive input and '1' for negative input */
    uint8_t sign;
};

/*! @name Structure to store temporary accelerometer values */
struct bma4_foc_temp_value
{
    /*! X data */
    int32_t x;

    /*! Y data */
    int32_t y;

    /*! Z data */
    int32_t z;
};

/* Structure to store temporary axes data values */
struct bma4_temp_axes_val
{
    /* X data */
    int32_t x;

    /* Y data */
    int32_t y;

    /* Z data */
    int32_t z;
};

#endif /* End of BMA4_DEFS_H__ */
