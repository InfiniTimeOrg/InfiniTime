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
 * @file       bma4.h
 * @date       2020-05-08
 * @version    V2.14.13
 *
 */

/*
 * @file       bma4.h
 * @brief   Source file for the BMA4 Sensor API
 */

/*!
 * @defgroup bma4xy BMA4XY
 */

/**
 * \ingroup bma4xy
 * \defgroup bma4 BMA4
 * @brief Sensor driver for BMA4 sensor
 */

#ifndef BMA4_H__
#define BMA4_H__

/*********************************************************************/
/* header files */

#include "bma4_defs.h"
#ifdef AKM9916
#include "aux_akm9916.h"
#endif

#ifdef BMM150
#include "aux_bmm150.h"
#endif

/*********************************************************************/
/* (extern) variable declarations */
/*********************************************************************/
/* function prototype declarations */

/**
 * \ingroup bma4
 * \defgroup bma4ApiInit Initialization
 * @brief Initialize the sensor and device structure
 */

/*!
 * \ingroup bma4ApiInit
 * \page bma4_api_bma4_init bma4_init
 * \code
 * int8_t bma4_init(struct bma4_dev *dev);
 * \endcode
 * @details This API is the entry point.
 * Call this API before using all other APIs.
 * This API reads the chip-id of the sensor which is the first step to
 * verify the sensor and also it configures the read mechanism of SPI and
 * I2C interface.
 *
 * @param[in,out] dev : Structure instance of bma4_dev
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 *
 * @note
 * While changing the parameter of the bma4
 * consider the following point:
 * Changing the reference value of the parameter
 * will changes the local copy or local reference
 * make sure your changes will not
 * affect the reference value of the parameter
 * (Better case don't change the reference value of the parameter)
 */
int8_t bma4_init(struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiConfig ConfigFile
 * @brief Write binary configuration in the sensor
 */

/*!
 * \ingroup bma4ApiConfig
 * \page bma4_api_bma4_write_config_file bma4_write_config_file
 * \code
 * int8_t bma4_write_config_file(struct bma4_dev *dev);
 * \endcode
 * @details This API is used to write the binary configuration in the sensor
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_write_config_file(struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiRegisters Registers
 * @brief Perform read / write operation to registers of the sensor
 */

/*!
 * \ingroup bma4ApiRegisters
 * \page bma4_api_bma4_write_regs bma4_write_regs
 * \code
 * int8_t bma4_write_regs(uint8_t addr, uint8_t *data, uint8_t len, struct bma4_dev *dev);
 * \endcode
 * @details  This API checks whether the write operation requested is for
 * feature config or register write and accordingly writes the data in the
 * sensor.
 *
 * @note user has to disable the advance power save mode in the sensor when
 * using this API in burst write mode.
 * bma4_set_advance_power_save(BMA4_DISABLE, dev);
 *
 * @param[in] addr : Register address.
 * @param[in] data : Write data buffer
 * @param[in] len  : No of bytes to write
 * @param[in] dev  : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_write_regs(uint8_t addr, const uint8_t *data, uint32_t len, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiRegisters
 * \page bma4_api_bma4_read_regs bma4_read_regs
 * \code
 * int8_t bma4_write_regs(uint8_t addr, uint8_t *data, uint8_t len, struct bma4_dev *dev);
 * \endcode
 * @details This API checks whether the read operation requested is for
 * feature or register read and accordingly reads the data from the sensor.
 *
 * @param[in] addr : Register address.
 * @param[in] data : Read data buffer.
 * @param[in] len  : No of bytes to read.
 * @param[in] dev  : Structure instance of bma4_dev
 *
 * @note For most of the registers auto address increment applies, with the
 * exception of a few special registers, which trap the address. For e.g.,
 * Register address - 0x26, 0x5E.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_read_regs(uint8_t addr, uint8_t *data, uint32_t len, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiErrorStatus Error Status
 * @brief Read error status of the sensor
 */

/*!
 * \ingroup bma4ApiErrorStatus
 * \page bma4_api_bma4_get_error_status bma4_get_error_status
 * \code
 * int8_t bma4_get_error_status(struct bma4_err_reg *err_reg, struct bma4_dev *dev);;
 * \endcode
 * @details This API reads the error status from the sensor.
 *
 * Below table mention the types of error which can occur in the sensor
 *
 *@verbatim
 *************************************************************************
 *        Error           |       Description
 *************************|***********************************************
 *                        |       Fatal Error, chip is not in operational
 *        fatal           |       state (Boot-, power-system).
 *                        |       This flag will be reset only by
 *                        |       power-on-reset or soft reset.
 *************************|***********************************************
 *        cmd             |       Command execution failed.
 *************************|***********************************************
 *                        |       Value        Name       Description
 *        error_code      |       000        no_error    no error
 *                        |       001        acc_err      error in
 *                        |                               ACC_CONF
 *************************|***********************************************
 *                        |        Error in FIFO detected: Input data was
 *        fifo            |        discarded in stream mode. This flag
 *                        |        will be reset when read.
 *************************|***********************************************
 *        mag             |        Error in I2C-Master detected.
 *                        |        This flag will be reset when read.
 *************************************************************************
 *@endverbatim
 *
 * @param[in,out] err_reg : Pointer to structure variable which stores the
 * error status read from the sensor.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_get_error_status(struct bma4_err_reg *err_reg, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiStatus Status
 * @brief Read sensor status
 */

/*!
 * \ingroup bma4ApiStatus
 * \page bma4_api_bma4_get_status bma4_get_status
 * \code
 * int8_t bma4_get_status(uint8_t *status, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the sensor status from the dev sensor.
 *
 * Below table lists the sensor status flags
 *
 * @verbatim
 *                 Status          |       Description
 *     ----------------------------|----------------------------------------
 *     BMA4_MAG_MAN_OP_ONGOING     | Manual Mag. interface operation ongoing
 *     BMA4_CMD_RDY                | Command decoder is ready.
 *     BMA4_MAG_DATA_RDY           | Data ready for Mag.
 *     BMA4_ACC_DATA_RDY           | Data ready for Accel.
 *@endverbatim
 *
 * @param[in] status : Variable used to store the sensor status flags
 * which is read from the sensor.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_get_status(uint8_t *status, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAccelxyz Accel XYZ Data
 * @brief Read accel xyz data from the sensor
 */

/*!
 * \ingroup bma4ApiAccelxyz
 * \page bma4_api_bma4_read_accel_xyz bma4_read_accel_xyz
 * \code
 * int8_t bma4_read_accel_xyz(struct bma4_accel *accel, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the Accel data for x,y and z axis from the sensor.
 *  The data units is in LSB format.
 *
 * @param[in] accel : Variable used to store the Accel data which is read
 * from the sensor.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @note For setting the Accel configuration use the below function
 * bma4_set_accel_config
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_read_accel_xyz(struct bma4_accel *accel, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiSensorTime Sensor Time
 * @brief Read sensor time of the sensor
 */

/*!
 * \ingroup bma4ApiSensorTime
 * \page bma4_api_bma4_get_sensor_time bma4_get_sensor_time
 * \code
 * int8_t bma4_get_sensor_time(uint32_t *sensor_time, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the sensor time of Sensor time gets updated
 *  with every update of data register or FIFO.
 *
 * @param[in] sensor_time : Pointer variable which stores sensor time
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_get_sensor_time(uint32_t *sensor_time, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiTemperature Temperature
 * @brief Read chip temperature of the sensor
 */

/*!
 * \ingroup bma4ApiTemperature
 * \page bma4_api_bma4_get_temperature bma4_get_temperature
 * \code
 * int8_t bma4_get_temperature(int32_t *temp, uint8_t temp_unit, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the chip temperature of sensor.
 * @note If Accel and Mag are disabled, the temperature value will be set
 * to invalid.
 *
 * @param[out] temp : Pointer variable which stores the temperature value.
 * @param[in] temp_unit : indicates the unit of temperature
 *
 * @verbatim
 * temp_unit   |   description
 * ------------|-------------------
 * BMA4_DEG    |   degrees Celsius
 * BMA4_FAHREN |   degrees fahrenheit
 * BMA4_KELVIN |   degrees kelvin
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @note Using a scaling factor of 1000, to obtain integer values, which
 * at the user end, are used to get accurate temperature value.
 * BMA4_SCALE_FARHAN = 1.8 * 1000, BMA4_SCALE_KELVIN = 273.15 * 1000
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_get_temperature(int32_t *temp, uint8_t temp_unit, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAccel Accel Configuration
 * @brief Read / Write configurations of accel sensor
 */

/*!
 * \ingroup bma4ApiAccel
 * \page bma4_api_bma4_get_accel_config bma4_get_accel_config
 * \code
 * int8_t bma4_get_accel_config(struct bma4_accel_config *accel, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the Output data rate, Bandwidth, perf_mode
 * and Range of accel.
 *
 * @param[in,out] accel :  Address of user passed structure which is used
 *  to store the Accel configurations read from the sensor.
 *
 * @note Enums and corresponding values for structure parameters like
 * Odr, Bandwidth and Range are mentioned in the below tables.
 *
 *@verbatim
 *  Value      |        Odr
 *  -----------|------------------------------------
 *   1         |     BMA4_OUTPUT_DATA_RATE_0_78HZ
 *   2         |     BMA4_OUTPUT_DATA_RATE_1_56HZ
 *   3         |     BMA4_OUTPUT_DATA_RATE_3_12HZ
 *   4         |     BMA4_OUTPUT_DATA_RATE_6_25HZ
 *   5         |     BMA4_OUTPUT_DATA_RATE_12_5HZ
 *   6         |     BMA4_OUTPUT_DATA_RATE_25HZ
 *   7         |     BMA4_OUTPUT_DATA_RATE_50HZ
 *   8         |     BMA4_OUTPUT_DATA_RATE_100HZ
 *   9         |     BMA4_OUTPUT_DATA_RATE_200HZ
 *   10        |     BMA4_OUTPUT_DATA_RATE_400HZ
 *   11        |     BMA4_OUTPUT_DATA_RATE_800HZ
 *   12        |     BMA4_OUTPUT_DATA_RATE_1600HZ
 *@endverbatim
 *
 *@verbatim
 *  Value |  accel_bw
 *  ------|--------------------------
 *    0   |  BMA4_ACCEL_OSR4_AVG1
 *    1   |  BMA4_ACCEL_OSR2_AVG2
 *    2   |  BMA4_ACCEL_NORMAL_AVG4
 *    3   |  BMA4_ACCEL_CIC_AVG8
 *    4   |  BMA4_ACCEL_RES_AVG16
 *    5   |  BMA4_ACCEL_RES_AVG32
 *    6   |  BMA4_ACCEL_RES_AVG64
 *    7   |  BMA4_ACCEL_RES_AVG128
 *@endverbatim
 *
 *@verbatim
 *   Value   | g_range
 *   --------|---------------------
 *   0x00    | BMA4_ACCEL_RANGE_2G
 *   0x01    | BMA4_ACCEL_RANGE_4G
 *   0x02    | BMA4_ACCEL_RANGE_8G
 *   0x03    | BMA4_ACCEL_RANGE_16G
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_get_accel_config(struct bma4_accel_config *accel, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAccel
 * \page bma4_api_bma4_set_accel_config bma4_set_accel_config
 * \code
 * int8_t bma4_set_accel_config(struct bma4_accel_config *accel, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the output_data_rate, bandwidth, perf_mode
 * and range of Accel.
 *
 * @param[in] accel : Pointer to structure variable which specifies the
 * Accel configurations.
 *
 * @note Enums and corresponding values for structure parameters like
 * Odr, Bandwidth and Range are mentioned in the below tables.
 *
 * @verbatim
 *  Value   |   ODR
 *  --------|-----------------------------------------
 *   1      |   BMA4_OUTPUT_DATA_RATE_0_78HZ
 *   2      |   BMA4_OUTPUT_DATA_RATE_1_56HZ
 *   3      |   BMA4_OUTPUT_DATA_RATE_3_12HZ
 *   4      |   BMA4_OUTPUT_DATA_RATE_6_25HZ
 *   5      |   BMA4_OUTPUT_DATA_RATE_12_5HZ
 *   6      |   BMA4_OUTPUT_DATA_RATE_25HZ
 *   7      |   BMA4_OUTPUT_DATA_RATE_50HZ
 *   8      |   BMA4_OUTPUT_DATA_RATE_100HZ
 *   9      |   BMA4_OUTPUT_DATA_RATE_200HZ
 *   10     |   BMA4_OUTPUT_DATA_RATE_400HZ
 *   11     |   BMA4_OUTPUT_DATA_RATE_800HZ
 *   12     |   BMA4_OUTPUT_DATA_RATE_1600HZ
 *
 *@endverbatim
 *
 *@verbatim
 *  Value |  accel_bw
 *  ------|--------------------------
 *    0   |  BMA4_ACCEL_OSR4_AVG1
 *    1   |  BMA4_ACCEL_OSR2_AVG2
 *    2   |  BMA4_ACCEL_NORMAL_AVG4
 *    3   |  BMA4_ACCEL_CIC_AVG8
 *    4   |  BMA4_ACCEL_RES_AVG16
 *    5   |  BMA4_ACCEL_RES_AVG32
 *    6   |  BMA4_ACCEL_RES_AVG64
 *    7   |  BMA4_ACCEL_RES_AVG128
 *@endverbatim
 *
 *@verbatim
 *  Value   | g_range
 *  --------|---------------------
 *  0x00    | BMA4_ACCEL_RANGE_2G
 *  0x01    | BMA4_ACCEL_RANGE_4G
 *  0x02    | BMA4_ACCEL_RANGE_8G
 *  0x03    | BMA4_ACCEL_RANGE_16G
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 *
 */
int8_t bma4_set_accel_config(const struct bma4_accel_config *accel, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAdvancedPowerMode Advanced Power Mode
 * @brief Read / Write advance power mode of accel sensor
 */

/*!
 * \ingroup bma4ApiAdvancedPowerMode
 * \page bma4_api_bma4_set_advance_power_save bma4_set_advance_power_save
 * \code
 * int8_t bma4_set_advance_power_save(uint8_t adv_pwr_save, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the advance power save mode in the sensor.
 *
 * @note If advanced power save is enabled and the Accel  and/or
 * magnetometer operate in duty cycling mode, the length of the unlatched
 * DRDY interrupt pulse is longer than 1/3.2 kHz (312.5 us).
 *
 * @param[in] adv_pwr_save : The value of advance power save mode
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_set_advance_power_save(uint8_t adv_pwr_save, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAdvancedPowerMode
 * \page bma4_api_bma4_get_advance_power_save bma4_get_advance_power_save
 * \code
 * int8_t bma4_get_advance_power_save(uint8_t adv_pwr_save, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the status of advance power save mode
 * from the sensor.
 *
 * @note If the advanced power save is enabled and the Accel  and/or
 * magnetometer operate in duty cycling mode, the length of the unlatched
 * DRDY interrupt pulse is longer than 1/3.2 kHz (312.5 us).
 *
 * @param[out] adv_pwr_save : The value of advance power save mode
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_get_advance_power_save(uint8_t *adv_pwr_save, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiFIFOSelfWakeUp FIFO Self Wake up
 * @brief Read / Write FIFO self wake up functionality in the sensor
 */

/*!
 * \ingroup bma4ApiFIFOSelfWakeUp
 * \page bma4_api_bma4_set_fifo_self_wakeup bma4_set_fifo_self_wakeup
 * \code
 * int8_t bma4_set_fifo_self_wakeup(uint8_t fifo_self_wakeup, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the FIFO self wake up functionality in the sensor.
 *
 * @note Functionality related to FIFO self wake up depends upon the
 * advance power save mode. for more info. refer data sheet.
 *
 * @param[in] fifo_self_wakeup : Variable used to enable or disable
 * FIFO self wake up functionality.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_set_fifo_self_wakeup(uint8_t fifo_self_wakeup, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiFIFOSelfWakeUp
 * \page bma4_api_bma4_get_fifo_self_wakeup bma4_get_fifo_self_wakeup
 * \code
 * int8_t bma4_get_fifo_self_wakeup(uint8_t *fifo_self_wakeup, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the status of FIFO self wake up functionality from
 * the sensor.
 *
 * @note Functionality related to FIFO self wake up depends upon the
 * advance power save mode. for more info. refer data sheet.
 *
 * @param[out] fifo_self_wake_up : Pointer variable used to store the
 * fifo self wake up status.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @return Result of API execution status
 *  @retval 0 -> Success
 *  @retval < 0 -> Fail
 */
int8_t bma4_get_fifo_self_wakeup(uint8_t *fifo_self_wake_up, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAccelEnable Accel Enable
 * @brief Enables / Disables accelerometer in the sensor
 */

/*!
 * \ingroup bma4ApiAccelEnable
 * \page bma4_api_bma4_set_accel_enable bma4_set_accel_enable
 * \code
 * int8_t bma4_set_accel_enable(uint8_t accel_en, struct bma4_dev *dev);
 * \endcode
 * @details This API enables or disables the Accel in the sensor.
 *
 * @note Before reading Accel data, user should call this API.
 *
 * @param[in] accel_en : Variable used to enable or disable the  Accel.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_accel_enable(uint8_t accel_en, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAccelEnable
 * \page bma4_api_bma4_get_accel_enable bma4_get_accel_enable
 * \code
 * int8_t bma4_get_accel_enable(uint8_t *accel_en, struct bma4_dev *dev);
 * \endcode
 * @details This API checks whether Accel is enabled or not in the sensor.
 *
 * @param[out] accel_en : Pointer variable used to store the Accel enable
 * status
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_accel_enable(uint8_t *accel_en, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagEnable Magnetometer Enable
 * @brief Enables / Disables Auxiliary Magnetometer in the sensor
 */

/*!
 * \ingroup bma4ApiMagEnable
 * \page bma4_api_bma4_set_mag_enable bma4_set_mag_enable
 * \code
 * int8_t bma4_set_mag_enable(uint8_t mag_en, struct bma4_dev *dev);
 * \endcode
 * @details This API is used to enable or disable auxiliary Mag
 * in the sensor.
 *
 * @note Before reading Mag data, user should call this API.
 *
 * @param[in] mag_en : Variable used to enable or disable the Mag.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_mag_enable(uint8_t mag_en, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiMagEnable
 * \page bma4_api_bma4_get_mag_enable bma4_get_mag_enable
 * \code
 * int8_t bma4_get_mag_enable(uint8_t *mag_en, struct bma4_dev *dev);
 * \endcode
 * @details This API is used to check whether the auxiliary Mag is enabled
 * or not in the sensor.
 *
 * @param[out] mag_en : Pointer variable used to store the enable status of
 * Mag in the sensor.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_mag_enable(uint8_t *mag_en, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiSpiInterface SPI interface
 * @brief Read / Write SPI interface mode set for primary interface
 */

/*!
 * \ingroup bma4AiSpiInterface
 * \page bma4_api_bma4_set_spi_interface bma4_set_spi_interface
 * \code
 * int8_t bma4_set_spi_interface(uint8_t *spi, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the SPI interface mode which is set for primary
 * interface.
 *
 * @param[out] spi : Pointer variable which stores the SPI mode selection
 *
 * @verbatim
 *  Value   |  Description
 *  --------|------------------
 *      0   |  SPI 4-wire mode
 *      1   |  SPI 3-wire mode
 *
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_spi_interface(uint8_t *spi, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiSpiInterface
 * \page bma4_api_bma4_get_spi_interface bma4_get_spi_interface
 * \code
 * int8_t bma4_get_spi_interface(uint8_t spi, struct bma4_dev *dev);
 * \endcode
 * @details This API configures the SPI interface Mode for primary interface
 *
 *  @param[in] spi : The value of SPI mode selection
 *
 *@verbatim
 *  Value   |  Description
 *  --------|------------------
 *      0   |  SPI 4-wire mode
 *      1   |  SPI 3-wire mode
 *
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_spi_interface(uint8_t spi, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiCommandReg Command Register
 * @brief Write available sensor specific commands to the sensor
 */

/*!
 * \ingroup bma4ApiCommandReg
 * \page bma4_api_bma4_set_command_register bma4_set_command_register
 * \code
 * int8_t bma4_set_command_register(uint8_t command_reg, struct bma4_dev *dev);
 * \endcode
 * @details This API writes the available sensor specific commands
 * to the sensor.
 *
 *  @param[in] command_reg : The command to write to the command register
 *
 *@verbatim
 *  value   |       Description
 *  --------|------------------------------------------------------
 *  0xB6    |       Triggers a soft reset
 *  0xB0    |       Clears all data in the FIFO, does not change
 *          |       FIFO_CONFIG and FIFO_DOWNS registers
 *  0xF0    |       Reset acceleration data path
 *@endverbatim
 *
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @note Register will always read as 0x00
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_command_register(uint8_t command_reg, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiI2CAddr i2C Device Address
 * @brief Write I2C device address of auxiliary sensor
 */

/*!
 * \ingroup bma4ApiI2CAddr
 * \page bma4_api_bma4_set_i2c_device_addr bma4_set_i2c_device_addr
 * \code
 * int8_t bma4_set_i2c_device_addr(struct bma4_dev *dev);
 * \endcode
 * @details This API sets the I2C device address of auxiliary sensor
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_i2c_device_addr(struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagManualEnable Mag Manual Enable
 * @brief Read / Write register access on magnetometer manually
 */

/*!
 * \ingroup bma4ApiMagManualEnable
 * \page bma4_api_bma4_set_mag_manual_enable bma4_set_mag_manual_enable
 * \code
 * int8_t bma4_set_mag_manual_enable(uint8_t mag_manual, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the register access on MAG_IF[2], MAG_IF[3],
 * MAG_IF[4] in the sensor. This implies that the DATA registers are
 * not updated with Mag values automatically.
 *
 * @param[in] mag_manual : Variable used to specify the Mag manual
 * enable status.
 *
 *@verbatim
 *  value    |  mag manual
 *  ---------|--------------------
 *  0x01     |  BMA4_ENABLE
 *  0x00     |  BMA4_DISABLE
 *@endverbatim
 *
 * @param[out] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_mag_manual_enable(uint8_t mag_manual, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiMagManualEnable
 * \page bma4_api_bma4_get_mag_manual_enable bma4_get_mag_manual_enable
 * \code
 * int8_t bma4_get_mag_manual_enable(uint8_t *mag_manual, struct bma4_dev *dev);
 * \endcode
 * @details This API checks whether the Mag access is done manually or
 * automatically in the sensor.
 * If the Mag access is done through manual mode then Mag data registers
 * in sensor are not updated automatically.
 *
 * @param[out] mag_manual : Mag manual enable value
 *
 *@verbatim
 *  value   | mag_manual
 *  --------|-------------------
 *  0x01    |   BMA4_ENABLE
 *  0x00    |   BMA4_DISABLE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_mag_manual_enable(uint8_t *mag_manual, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagIFMode Mag Interface Mode
 * @brief Set I2C interface configuration mode for auxiliary magnetometer
 */

/*!
 * \ingroup bma4ApiMagIFMode
 * \page bma4_api_bma4_set_aux_if_mode bma4_set_aux_if_mode
 * \code
 * int8_t bma4_set_aux_if_mode(uint8_t if_mode, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the I2C interface configuration(if) mode
 * for auxiliary Mag.
 *
 * @param[in] if_mode : The value of interface configuration mode
 *
 *@verbatim
 *      Value   |       Description
 *  ------------|-------------------------------------------
 *      0       |   p_auto_s_off Auxiliary interface:off
 *      1       |   p_auto_s_mag Auxiliary interface:on
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_aux_if_mode(uint8_t if_mode, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagRead Mag Read
 * @brief Set / Get address of register of Aux Mag sensor to read data
 */

/*!
 * \ingroup bma4ApiMagRead
 * \page bma4_api_bma4_get_mag_read_addr bma4_get_mag_read_addr
 * \code
 * int8_t bma4_get_mag_read_addr(uint8_t *mag_read_addr, struct bma4_dev *dev);
 * \endcode
 * @details  This API gets the address of the register of Aux Mag sensor
 * where the data to be read.
 *
 * @param[out]  mag_read_addr : Pointer variable used to store the
 * mag read address.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_mag_read_addr(uint8_t *mag_read_addr, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiMagRead
 * \page bma4_api_bma4_set_mag_read_addr bma4_set_mag_read_addr
 * \code
 * int8_t bma4_set_mag_read_addr(uint8_t mag_read_addr, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the address of the register of Aux Mag sensor
 * where the data to be read.
 *
 * @param[in] mag_read_addr: Value of Mag. read address in order to read
 * the data from the auxiliary Mag.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_mag_read_addr(uint8_t mag_read_addr, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagWrite Mag Write
 * @brief Set / Get address of register of Aux Mag sensor to write data
 */

/*!
 * \ingroup bma4ApiMagWrite
 * \page bma4_api_bma4_get_mag_write_addr bma4_get_mag_write_addr
 * \code
 * int8_t bma4_get_mag_write_addr(uint8_t *mag_write_addr, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the Aux Mag write address from the sensor.
 *  Mag write address is where the Mag data will be written.
 *
 * @param[out]  mag_write_addr: Pointer used to store the Mag write address
 * which is read from the sensor.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_mag_write_addr(uint8_t *mag_write_addr, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiMagWrite
 * \page bma4_api_bma4_set_mag_write_addr bma4_set_mag_write_addr
 * \code
 * int8_t bma4_set_mag_write_addr(uint8_t mag_write_addr, struct bma4_dev *dev);
 * \endcode
 * @details  This API sets the Aux Mag write address in the sensor.
 * Mag write address is where the Mag data will be written.
 *
 * @param[in]  mag_write_addr: Write address of Mag where the data will
 * be written.
 * @param[out] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_mag_write_addr(uint8_t mag_write_addr, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagData Mag Data
 * @brief Read / Write data from the Mag sensor
 */

/*!
 * \ingroup bma4ApiMagData
 * \page bma4_api_bma4_get_mag_write_data bma4_get_mag_write_data
 * \code
 * int8_t bma4_get_mag_write_data(uint8_t *mag_write_data, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the data from the sensor which is written to the
 * Mag.
 *
 * @param[out]  mag_write_data: Pointer variable which stores the
 * data which is written in Mag through sensor.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_mag_write_data(uint8_t *mag_write_data, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiMagData
 * \page bma4_api_bma4_set_mag_write_data bma4_set_mag_write_data
 * \code
 * int8_t bma4_set_mag_write_data(uint8_t mag_write_data, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the data in the sensor which in turn will
 * be written to Mag.
 *
 * @param[in]  mag_write_data: variable which specify the data which is to
 * be written in Mag.
 * @param[out] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_mag_write_data(uint8_t mag_write_data, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiReadMagXYZR Mag xyzr Data
 * @brief Read xyzr axes data from auxiliary Mag sensor
 */

/*!
 * \ingroup bma4ApiReadMagXYZR
 * \page bma4_api_bma4_read_mag_xyzr bma4_read_mag_xyzr
 * \code
 * int8_t bma4_read_mag_xyzr(struct bma4_mag_xyzr *mag, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the x,y,z and r axis data from the auxiliary
 * Mag BMM150/AKM9916 sensor.
 *
 * @param[out] mag : Pointer variable to store the auxiliary Mag x,y,z
 * and r axis data read from the sensor.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_read_mag_xyzr(struct bma4_mag_xyzr *mag, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagBurst Mag Burst
 * @brief Set / Get burst data length of auxiliary Mag Sensor
 */

/*!
 * \ingroup bma4ApiMagBurst
 * \page bma4_api_bma4_set_mag_burst bma4_set_mag_burst
 * \code
 * int8_t bma4_set_mag_burst(uint8_t mag_burst, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the burst data length (1,2,6,8 byte) of auxiliary
 * Mag sensor.
 *
 * @param[in] mag_burst : Variable used to specify the Mag burst read length
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_mag_burst(uint8_t mag_burst, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiMagBurst
 * \page bma4_api_bma4_get_mag_burst bma4_get_mag_burst
 * \code
 * int8_t bma4_get_mag_burst(uint8_t *mag_burst, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the burst data length of Mag set in the sensor.
 *
 * @param[out] mag_burst : Pointer variable used to store the burst length
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_mag_burst(uint8_t *mag_burst, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiReadFIFO Read FIFO Data
 * @brief Read FIFO data of accel and/or Mag sensor
 */

/*!
 * \ingroup bma4ApiReadFIFO
 * \page bma4_api_bma4_read_fifo_data bma4_read_fifo_data
 * \code
 * int8_t bma4_read_fifo_data(struct bma4_fifo_frame *fifo, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the FIFO data of Accel and/or Mag sensor
 *
 * @param dev  : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_read_fifo_data(struct bma4_fifo_frame *fifo, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiFIFOWM FIFO Watermark level
 * @brief Read / Write FIFO watermark level in the sensor
 */

/*!
 * \ingroup bma4ApiFIFOWM
 * \page bma4_api_bma4_get_fifo_wm bma4_get_fifo_wm
 * \code
 * int8_t bma4_get_fifo_wm(uint16_t *fifo_wm, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the FIFO water mark level which is set
 * in the sensor.
 *
 * @note The FIFO watermark is issued when the FIFO fill level is
 * equal or above the watermark level.
 *
 * @param[out]  fifo_wm : Pointer variable to store FIFO water mark level
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_fifo_wm(uint16_t *fifo_wm, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiFIFOWM
 * \page bma4_api_bma4_set_fifo_wm bma4_set_fifo_wm
 * \code
 * int8_t bma4_set_fifo_wm(uint16_t fifo_wm, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the FIFO watermark level in the sensor.
 *
 * @note The FIFO watermark is issued when the FIFO fill level is
 * equal or above the watermark level.
 *
 * @param[in]  fifo_wm : Variable used to set the FIFO water mark level
 * @param[out] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_fifo_wm(uint16_t fifo_wm, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAccelFIFOFilterData Accel FIFO Filter data
 * @brief Set / Get Filtered or unfiltered mode of Accel FIFO data
 */

/*!
 * \ingroup bma4ApiAccelFIFOFilterData
 * \page bma4_api_bma4_get_accel_fifo_filter_data bma4_get_accel_fifo_filter_data
 * \code
 * int8_t bma4_get_accel_fifo_filter_data(uint8_t *accel_fifo_filter, struct bma4_dev *dev);
 * \endcode
 * @details This API checks whether the Accel FIFO data is set for filtered
 * or unfiltered mode.
 *
 * @param[out] accel_fifo_filter : Variable used to check whether the Accel
 * data is filtered or unfiltered.
 *
 *@verbatim
 *  Value    |  accel_fifo_filter
 *  ---------|-------------------------
 *  0x00     |  Unfiltered data
 *  0x01     |  Filtered data
 *@endverbatim
 *
 * @param[in] dev : structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_accel_fifo_filter_data(uint8_t *accel_fifo_filter, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAccelFIFOFilterData
 * \page bma4_api_bma4_set_accel_fifo_filter_data bma4_set_accel_fifo_filter_data
 * \code
 * int8_t bma4_set_accel_fifo_filter_data(uint8_t accel_fifo_filter, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the condition of Accel FIFO data either to
 * filtered or unfiltered mode.
 *
 * @param[in] accel_fifo_filter : Variable used to set the filtered or
 * unfiltered condition of Accel FIFO data.
 *
 *@verbatim
 *  value      |  accel_fifo_filter_data
 *  -----------|-------------------------
 *    0x00     |  Unfiltered data
 *    0x01     |  Filtered data
 *@endverbatim
 *
 * @param[out] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_accel_fifo_filter_data(uint8_t accel_fifo_filter, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAccelFIFODown Accel FIFO Down Sampling
 * @brief Read / Write Down sampling rates configured for Accel FIFO Data
 */

/*!
 * \ingroup bma4ApiAccelFIFOFilterData
 * \page bma4_api_bma4_get_fifo_down_accel bma4_get_fifo_down_accel
 * \code
 * int8_t bma4_get_fifo_down_accel(uint8_t *fifo_down, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the down sampling rates which is configured
 * for Accel FIFO data.
 *
 * @param[out] fifo_down : Variable used to specify the Accel FIFO
 * down-sampling rates
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_fifo_down_accel(uint8_t *fifo_down, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAccelFIFOFilterData
 * \page bma4_api_bma4_set_fifo_down_accel bma4_set_fifo_down_accel
 * \code
 * int8_t bma4_set_fifo_down_accel(uint8_t fifo_down, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the down-sampling rates for Accel FIFO.
 *
 * @param[in] fifo_down : Variable used to specify the Accel FIFO
 * down-sampling rates.
 * @param[in] dev : structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_fifo_down_accel(uint8_t fifo_down, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiFIFOLength FIFO Length
 * @brief Read length of FIFO data available in the sensor (unit of bytes)
 */

/*!
 * \ingroup bma4ApiAccelFIFOFilterData
 * \page bma4_api_bma4_get_fifo_length bma4_get_fifo_length
 * \code
 * int8_t bma4_get_fifo_length(uint16_t *fifo_length, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the length of FIFO data available in the sensor
 * in the units of bytes.
 *
 * @note This byte counter is updated each time a complete frame was read
 * or written
 *
 * @param[in] fifo_length : Pointer variable used to store the value of
 * fifo byte counter
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_get_fifo_length(uint16_t *fifo_length, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagCompensate Compensate Mag data
 * @brief Aligns and Compensates Mag Data of BMM150 / AKM9916
 */

/*!
 * \ingroup bma4ApiAccelFIFOFilterData
 * \page bma4_api_bma4_second_if_mag_compensate_xyz bma4_second_if_mag_compensate_xyz
 * \code
 * int8_t bma4_second_if_mag_compensate_xyz(struct bma4_mag_fifo_data mag_fifo_data,
 *                                          uint8_t mag_second_if,
 *                                          const struct bma4_mag *compensated_mag_data);
 *
 * \endcode
 * @details This API aligns and compensates the Mag data of BMM150/AKM9916
 * sensor.
 *
 * @param[in] mag_fifo_data: Structure object which stores the Mag x,yand z
 * axis FIFO data which is to be aligned and/or compensated.
 * @param[in] mag_second_if: Variable used to select the Mag sensor.
 *
 *@verbatim
 *  Value   |   mag_second_if
 *  --------|----------------------
 *    1     |   BMA4_SEC_IF_BMM150
 *    2     |   BMA4_SEC_IF_AKM09916
 *@endverbatim
 *
 * @param[out] compensated_mag_data: Pointer variable used to store the
 * compensated Mag xyz axis data
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_second_if_mag_compensate_xyz(struct bma4_mag_fifo_data mag_fifo_data,
                                         uint8_t mag_second_if,
                                         const struct bma4_mag *compensated_mag_data);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagXYZ Mag XYZ
 * @brief Read x, y and z axes data from either BMM150 or AKM9916 sensor
 */

/*!
 * \ingroup bma4ApiMagXYZ
 * \page bma4_api_bma4_read_mag_xyz bma4_read_mag_xyz
 * \code
 * int8_t bma4_read_mag_xyz(const struct bma4_mag *mag, uint8_t sensor_select, const struct bma4_dev *dev);
 * \endcode
 * @details This API reads Mag. x,y and z axis data from either BMM150 or
 * AKM9916 sensor
 *
 * @param[out] mag : Structure pointer used to store the Mag x,y, and z axis
 * data read from the sensor.
 *
 * @param[in] sensor_select : Variable used to select the Mag sensor
 *
 *@verbatim
 *  Value    |   Sensor
 *  ---------|----------------------
 *   0       | BMA4_SEC_IF_NULL
 *   1       | BMA4_SEC_IF_BMM150
 *   2       | BMA4_SEC_IF_AKM09916
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_read_mag_xyz(const struct bma4_mag *mag, uint8_t sensor_select, const struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiIFMode I2C interface configuration
 * @brief Set / Get auxiliary I2C interface configuration set in the sensor
 */

/*!
 * \ingroup bma4ApiIFMode
 * \page bma4_api_bma4_get_if_mode bma4_get_if_mode
 * \code
 * int8_t bma4_get_if_mode(uint8_t *if_mode, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the auxiliary I2C interface configuration which
 * is set in the sensor.
 *
 * @param[out]  if_mode : Pointer variable used to store the auxiliary
 * interface configuration.
 *
 *@verbatim
 *  Value |  Description
 *  ----- |----------------------------------
 *  0x00  |  auxiliary interface:off
 *  0x01  |  auxiliary interface:on
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_if_mode(uint8_t *if_mode, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiIFMode
 * \page bma4_api_bma4_set_if_mode bma4_set_if_mode
 * \code
 * int8_t bma4_set_if_mode(uint8_t if_mode, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the auxiliary interface configuration in the sensor.
 *
 * @param[in]  if_mode : Variable used to select the auxiliary interface
 * configuration.
 *
 *@verbatim
 *  Value |  Description
 *  ----- |--------------------------
 *  0x00  |  auxiliary interface:off
 *  0x01  |  auxiliary interface:on
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_if_mode(uint8_t if_mode, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAccelDataRdy Accel Data Ready
 * @brief Get accel data ready status from the sensor
 */

/*!
 * \ingroup bma4ApiAccelDataRdy
 * \page bma4_api_bma4_get_accel_data_rdy bma4_get_accel_data_rdy
 * \code
 * int8_t bma4_get_accel_data_rdy(uint8_t *data_rdy, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the data ready status of Accel from the sensor.
 * @note The status get reset when Accel data register is read.
 *
 * @param[out] data_rdy : Pointer variable to store the data ready  status
 * @param[in] dev : structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_accel_data_rdy(uint8_t *data_rdy, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiMagDataRdy Mag Data Ready
 * @brief Get Mag data ready status from the sensor
 */

/*!
 * \ingroup bma4ApiMagDataRdy
 * \page bma4_api_bma4_get_mag_data_rdy bma4_get_mag_data_rdy
 * \code
 * int8_t bma4_get_mag_data_rdy(uint8_t *data_rdy, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the data ready status of Mag from the sensor.
 * The status get reset when Mag data register is read.
 *
 * @param[out] data_rdy : Pointer variable to store the data ready status
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_mag_data_rdy(uint8_t *data_rdy, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiASICStatus ASIC status
 * @brief Read ASIC status from the sensor
 */

/*!
 * \ingroup bma4ApiASICStatus
 * \page bma4_api_bma4_get_asic_status bma4_get_asic_status
 * \code
 * int8_t bma4_get_asic_status(struct bma4_asic_status *asic_status, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the ASIC status from the sensor.
 * The status information is mentioned in the below table.
 *
 *@verbatim
 *******************************************************************************
 *       Status            |  Description
 **************************|****************************************************
 *      sleep              |  ASIC is in sleep/halt state.
 *      irq_ovrn           |  Dedicated interrupt is set again before previous
 *                         |  interrupt was acknowledged.
 *      wc_event           |  Watchcell event detected (ASIC stopped).
 *  stream_transfer_active | stream transfer has started.
 *******************************************************************************
 *@endverbatim
 *
 * @param[out] asic_status : Structure pointer used to store the ASIC
 * status read from the sensor.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_asic_status(struct bma4_asic_status *asic_status, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiOffsetComp Accel Offset Compensation
 * @brief Set / Get Accel Offset Compensation
 */

/*!
 * \ingroup bma4ApiOffsetComp
 * \page bma4_api_bma4_set_offset_comp bma4_set_offset_comp
 * \code
 * int8_t bma4_set_offset_comp(uint8_t offset_en, struct bma4_dev *dev);
 * \endcode
 * @details This API enables the offset compensation for filtered and
 * unfiltered  Accel data.
 *
 * @param[in] offset_en : Variable used to enable or disable offset
 * compensation
 *
 *@verbatim
 *  offset_en   |  Description
 *  ------------|----------------------
 *      0       | BMA4_DISABLE
 *      1       | BMA4_ENABLE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_offset_comp(uint8_t offset_en, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiOffsetComp
 * \page bma4_api_bma4_get_offset_comp bma4_get_offset_comp
 * \code
 * int8_t bma4_get_offset_comp(uint8_t *offset_en, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the status of Accel offset compensation
 *
 * @param[out] offset_en : Pointer variable used to store the Accel offset
 * enable or disable status.
 *
 *@verbatim
 *  offset_en |  Description
 *  ----------|--------------
 *      0     | BMA4_DISABLE
 *      1     | BMA4_ENABLE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_get_offset_comp(uint8_t *offset_en, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiExtractAccel Accel Extract and Parse Frames
 * @brief Parse and Extract Accel frames from FIFO data read
 */

/*!
 * \ingroup bma4ApiExtractAccel
 * \page bma4_api_bma4_extract_accel bma4_extract_accel
 * \code
 * int8_t bma4_extract_accel(struct bma4_accel *accel_data,
 *                           uint16_t *accel_length,
 *                           struct bma4_fifo_frame *fifo,
 *                           const struct bma4_dev *dev);
 * \endcode
 * @details This API parses and extracts the accelerometer frames from
 *  FIFO data read by the "bma4_read_fifo_data" API and stores it in the
 *  "accel_data" structure instance.
 *
 *  @note The bma4_extract_accel API should be called only after reading
 *  the FIFO data by calling the bma4_read_fifo_data() API
 *
 *  @param[in,out] accel_data   : Structure instance of bma4_accel where
 *   the accelerometer data in FIFO is stored.
 *  @param[in,out] accel_length : Number of accelerometer frames
 *   (x,y,z axes data)
 *  @param[in,out] dev          : Structure instance of bma4_dev.
 *
 *  @note accel_length has the number of accelerometer frames
 *  (1 accel frame   = 6 bytes) which the user needs to extract and store is
 *  provided as input parameter by the user and the Number of valid
 *  accelerometer frames extracted and stored is updated in
 *  "accel_length" at the end of execution of this API.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_extract_accel(struct bma4_accel *accel_data,
                          uint16_t *accel_length,
                          struct bma4_fifo_frame *fifo,
                          const struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiExtractMag Mag Extract and Parse Frames
 * @brief Parse and Extract Accel frames from FIFO data read
 */

/*!
 * \ingroup bma4ApiExtractMag
 * \page bma4_api_bma4_extract_mag bma4_extract_mag
 * \code
 * int8_t bma4_extract_mag(const struct bma4_mag *mag_data,
 *                         uint16_t *mag_length,
 *                         struct bma4_fifo_frame *fifo,
 *                         const struct bma4_dev *dev);
 * \endcode
 * @details This API parses and extracts the magnetometer frames from
 *  FIFO data read by the "bma4_read_fifo_data" API and stores it in the
 *  "mag_data" structure instance parameter of this API
 *
 *  @note The bma4_extract_mag API should be called only after reading
 *  the FIFO data by calling the bma4_read_fifo_data() API
 *
 *  @param[in,out] mag_data   : Structure instance of bma4_mag_xyzr where
 *  the magnetometer data in FIFO is stored.
 *  @param[in,out] mag_length : Number of magnetometer frames (x,y,z,r data)
 *  @param[in,out] dev        : Structure instance of bma4_dev.
 *
 *  @note mag_length has the number of magnetometer frames(x,y,z,r data)
 *  (1 mag frame = 8 bytes) which the user needs to extract and store,It is
 *  provided as input parameter by the user and the number of valid
 *  magnetometer frames extracted and stored is updated in
 *  "mag_length" at the end of execution of this API.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_extract_mag(const struct bma4_mag *mag_data,
                        uint16_t *mag_length,
                        struct bma4_fifo_frame *fifo,
                        const struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAccelSelftest Accel Self test
 * @brief Self test for accel
 */

/*!
 * \ingroup bma4ApiAccelSelftest
 * \page bma4_api_bma4_perform_accel_selftest bma4_perform_accel_selftest
 * \code
 * int8_t bma4_perform_accel_selftest(int8_t *result, struct bma4_dev *dev);
 * \endcode
 * @details This API checks whether the self test functionality of the sensor
 *  is working or not
 *
 *  @param[in] result : Pointer variable used to store the result of self test
 *  operation
 *
 *@verbatim
 *  result   |  Description
 *  ---------|--------------------
 *  0x00     | BMA4_SELFTEST_PASS
 *  0x01     | BMA4_SELFTEST_FAIL
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_perform_accel_selftest(int8_t *result, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAccelSelftest
 * \page bma4_api_bma4_selftest_config bma4_selftest_config
 * \code
 * int8_t bma4_selftest_config(uint8_t sign, struct bma4_dev *dev);
 * \endcode
 * @details This API performs the steps needed for Self test operation
 *  before reading the Accel Self test data.
 *
 *  @param[in] sign: Variable used to specify the self test sign
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_selftest_config(uint8_t sign, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiInterrupt Interrupt Functions
 * @brief Interrupt mapping and set/get interrupt mode
 */

/*!
 * \ingroup bma4ApiInterrupt
 * \page bma4_api_bma4_map_interrupt bma4_map_interrupt
 * \code
 * int8_t bma4_map_interrupt(uint8_t int_line, uint16_t int_map, uint8_t enable, struct bma4_dev *dev);
 * \endcode
 * @details API sets the interrupt to either interrupt1 or
 *  interrupt2 pin in the sensor.
 *
 *  @param[in] int_line: Variable used to select interrupt pin1 or pin2
 *
 *@verbatim
 *  int_line | interrupt selection
 *  ---------|-------------------
 *   0       | BMA4_INTR1_MAP
 *   1       | BMA4_INTR2_MAP
 *@endverbatim
 *
 *  @param[in] int_map: Variable used to select a particular interrupt
 *      in the sensor
 *
 *  @param[in] enable : Variable used to enable or disable the interrupt
 *
 *@verbatim
 *  Value    | Behaviour
 *  ---------|-------------------
 *  0x01     |  BMA4_ENABLE
 *  0x00     |  BMA4_DISABLE
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_map_interrupt(uint8_t int_line, uint16_t int_map, uint8_t enable, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiInterrupt
 * \page bma4_api_bma4_set_interrupt_mode bma4_set_interrupt_mode
 * \code
 * int8_t bma4_set_interrupt_mode(uint8_t mode, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the interrupt mode in the sensor.
 *
 *  @param[in] mode: Variable used to specify the interrupt mode which
 *  is to be set in the sensor.
 *
 *@verbatim
 *  Mode                    |  Value
 *  ----------------------- |---------
 *  BMA4_NON_LATCH_MODE     |  0
 *  BMA4_LATCH_MODE         |  1
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_set_interrupt_mode(uint8_t mode, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiInterrupt
 * \page bma4_api_bma4_get_interrupt_mode bma4_get_interrupt_mode
 * \code
 * int8_t bma4_get_interrupt_mode(uint8_t *mode, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the interrupt mode which is set in the sensor.
 *
 *  @param[out] mode: Pointer variable used to store the interrupt mode set in
 *  in the sensor.
 *
 *@verbatim
 *  Mode                 |    Value
 *  ---------------------|---------------
 *  BMA4_NON_LATCH_MODE  |     0
 *  BMA4_LATCH_MODE      |     1
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_get_interrupt_mode(uint8_t *mode, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAuxMagConfig Auxiliary Mag Config
 * @brief Set / Get Auxiliary Mag(BMM150 or AKM9916) output data rate and offset
 */

/*!
 * \ingroup bma4ApiAuxMagConfig
 * \page bma4_api_bma4_set_aux_mag_config bma4_set_aux_mag_config
 * \code
 * int8_t bma4_set_aux_mag_config(const struct bma4_aux_mag_config *aux_mag, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the auxiliary Mag(BMM150 or AKM9916) output data
 *  rate and offset.
 *
 *  @param[in]  aux_mag : Pointer to structure variable used to specify
 *  the auxiliary Mag configuration.
 *
 *@verbatim
 *------------------------------------------------------------------------------
 *            Odr                         |        Value
 *----------------------------------------|---------------------------------
 *    BMA4_OUTPUT_DATA_RATE_0_78HZ        |        0x01
 *    BMA4_OUTPUT_DATA_RATE_1_56HZ        |        0x02
 *    BMA4_OUTPUT_DATA_RATE_3_12HZ        |        0x03
 *    BMA4_OUTPUT_DATA_RATE_6_25HZ        |        0x04
 *    BMA4_OUTPUT_DATA_RATE_12_5HZ        |        0x05
 *    BMA4_OUTPUT_DATA_RATE_25HZ          |        0x06
 *    BMA4_OUTPUT_DATA_RATE_50HZ          |        0x07
 *    BMA4_OUTPUT_DATA_RATE_100HZ         |        0x08
 *    BMA4_OUTPUT_DATA_RATE_200HZ         |        0x09
 *    BMA4_OUTPUT_DATA_RATE_400HZ         |        0x0A
 *    BMA4_OUTPUT_DATA_RATE_800HZ         |        0x0B
 *    BMA4_OUTPUT_DATA_RATE_1600HZ        |        0x0C
 *------------------------------------------------------------------------------
 *              Offset                        |        Value
 *--------------------------------------------|---------------------------------
 *      BMA4_MAG_OFFSET_MAX                   |        0x00
 *--------------------------------------------|---------------------------------
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_set_aux_mag_config(const struct bma4_aux_mag_config *aux_mag, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAuxMagConfig
 * \page bma4_api_bma4_get_aux_mag_config bma4_get_aux_mag_config
 * \code
 * int8_t bma4_get_aux_mag_config(struct bma4_aux_mag_config *aux_mag, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the auxiliary Mag(BMM150 or AKM9916) output data
 *  rate and offset.
 *  @note : Valid output data rates are mentioned in the below table
 *
 *  @param[out] aux_mag : Pointer to structure variable used to store the
 *  auxiliary Mag configuration read from the sensor
 *
 *@verbatim
 *------------------------------------------------------------------------
 *            Odr                         |        Value
 *----------------------------------------|-------------------------------
 *    BMA4_OUTPUT_DATA_RATE_0_78HZ        |        0x01
 *    BMA4_OUTPUT_DATA_RATE_1_56HZ        |        0x02
 *    BMA4_OUTPUT_DATA_RATE_3_12HZ        |        0x03
 *    BMA4_OUTPUT_DATA_RATE_6_25HZ        |        0x04
 *    BMA4_OUTPUT_DATA_RATE_12_5HZ        |        0x05
 *    BMA4_OUTPUT_DATA_RATE_25HZ          |        0x06
 *    BMA4_OUTPUT_DATA_RATE_50HZ          |        0x07
 *    BMA4_OUTPUT_DATA_RATE_100HZ         |        0x08
 *    BMA4_OUTPUT_DATA_RATE_200HZ         |        0x09
 *    BMA4_OUTPUT_DATA_RATE_400HZ         |        0x0A
 *    BMA4_OUTPUT_DATA_RATE_800HZ         |        0x0B
 *    BMA4_OUTPUT_DATA_RATE_1600HZ        |        0x0C
 *-------------------------------------------------------------------------
 *              Offset                    |        Value
 *----------------------------------------|--------------------------------
 *      BMA4_MAG_OFFSET_MAX               |        0x00
 *-------------------------------------------------------------------------
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_get_aux_mag_config(struct bma4_aux_mag_config *aux_mag, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiFIFOConfig FIFO Configuration
 * @brief Set / Get FIFO Configuration in the sensor
 */

/*!
 * \ingroup bma4ApiFIFOConfig
 * \page bma4_api_bma4_set_fifo_config bma4_set_fifo_config
 * \code
 * int8_t bma4_set_fifo_config(uint8_t config, uint8_t enable, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the FIFO configuration in the sensor.
 *
 *  @param[in] config : Enum variable used to specify the FIFO
 *  configurations which are to be enabled or disabled in the sensor.
 *
 *  @note : User can set either one or more or all FIFO configurations
 *  by ORing the below mentioned enums.
 *
 *@verbatim
 *      config                  |   Value
 *      ------------------------|---------------------------
 *      BMA4_FIFO_STOP_ON_FULL  |   0x01
 *      BMA4_FIFO_TIME          |   0x02
 *      BMA4_FIFO_TAG_INTR2     |   0x04
 *      BMA4_FIFO_TAG_INTR1     |   0x08
 *      BMA4_FIFO_HEADER        |   0x10
 *      BMA4_FIFO_MAG           |   0x20
 *      BMA4_FIFO_ACCEL         |   0x40
 *      BMA4_FIFO_ALL           |   0x7F
 *@endverbatim
 *
 *  @param[in] enable : Parameter used to enable or disable the above
 *  FIFO configuration
 *  @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_set_fifo_config(uint8_t config, uint8_t enable, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiFIFOConfig
 * \page bma4_api_bma4_get_fifo_config bma4_get_fifo_config
 * \code
 * int8_t bma4_get_fifo_config(uint8_t *fifo_config, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the FIFO configuration from the sensor.
 *
 *  @param[in] fifo_config : Enum variable used to get the below fifo
 *  configuration from the sensor.
 *
 *  @note After calling this function user should do the AND operation with
 *  the enum value populated by this function to know which FIFO
 *  configuration is enabled.
 *
 *@verbatim
 *      fifo_config              |      Value
 *      -------------------------|--------------------------
 *      BMA4_FIFO_STOP_ON_FULL   |       0x01
 *      BMA4_FIFO_TIME           |       0x02
 *      BMA4_FIFO_TAG_INTR2      |       0x04
 *      BMA4_FIFO_TAG_INTR1      |       0x08
 *      BMA4_FIFO_HEADER         |       0x10
 *      BMA4_FIFO_MAG            |       0x20
 *      BMA4_FIFO_ACCEL          |       0x40
 *      BMA4_FIFO_ALL            |       0x7F
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_get_fifo_config(uint8_t *fifo_config, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiIntConfig Interrupt Pin Config
 * @brief Set / Get Electrical behavior of interrupt pin1 or pin2 of the sensor
 */

/*!
 * \ingroup bma4ApiIntConfig
 * \page bma4_api_bma4_set_int_pin_config bma4_set_int_pin_config
 * \code
 * int8_t bma4_set_int_pin_config(const struct bma4_int_pin_config *int_pin_config,
 *                                uint8_t int_line,
 *                                struct bma4_dev *dev);
 *
 * \endcode
 * @details This function sets the electrical behaviour of interrupt pin1 or
 *  pin2 in the sensor.
 *
 *  @param[in] int_pin_config : Pointer to structure variable which specifies
 *  the configuration data of either interrupt pin1 or 2.
 *
 *@verbatim
 * ************************************************************************
 *  Structure field members        |        Macros
 * ********************************|***************************************
 *         edge_ctrl               |        BMA4_LEVEL_TRIGGER(0)
 *                                 |        BMA4_EDGE_TRIGGER(1)
 * ********************************|***************************************
 *         lvl                     |        BMA4_ACTIVE_LOW(0)
 *                                 |        BMA4_ACTIVE_HIGH(1)
 * ********************************|***************************************
 *         od                      |        BMA4_PUSH_PULL(0)
 *                                 |        BMA4_OPEN_DRAIN(1)
 * ********************************|***************************************
 *         output_en               |        BMA4_OUTPUT_DISABLE(0)
 *                                 |        BMA4_OUTPUT_ENABLE(1)
 * ********************************|***************************************
 *         input_en                |        BMA4_INPUT_DISABLE(0)
 *                                 |        BMA4_INPUT_ENABLE(1)
 * ************************************************************************
 *@endverbatim
 *
 *  @param[in] int_line : Variable used to select the interrupt pin1 or
 *  pin2 for interrupt configuration.
 *
 *@verbatim
 *      int_line        |    Value
 *      ----------------|----------------------
 *      BMA4_INTR1_MAP  |   0
 *      BMA4_INTR2_MAP  |   1
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma4_set_int_pin_config(const struct bma4_int_pin_config *int_pin_config, uint8_t int_line,
                               struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiIntConfig
 * \page bma4_api_bma4_get_int_pin_config bma4_get_int_pin_config
 * \code
 * int8_t bma4_get_int_pin_config(struct bma4_int_pin_config *int_pin_config, uint8_t int_line, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the electrical behavior of interrupt pin1 or pin2
 *  from the sensor.
 *
 *  @param[out] int_pin_config : Pointer to structure variable which stores the
 *  configuration data of either interrupt pin1 or pin2 read from the sensor
 *
 *@verbatim
 * ************************************************************************
 * Structure field members |        Macros
 * ************************|***********************************************
 * edge_ctrl               |        BMA4_LEVEL_TRIGGER(0)
 *                         |        BMA4_EDGE_TRIGGER(1)
 * ************************|***********************************************
 * lvl                     |        BMA4_ACTIVE_LOW(0)
 *                         |        BMA4_ACTIVE_HIGH(1)
 * ************************|***********************************************
 * od                      |        BMA4_PUSH_PULL(0)
 *                         |        BMA4_OPEN_DRAIN(1)
 * ************************|***********************************************
 * output_en               |        BMA4_OUTPUT_DISABLE(0)
 *                         |        BMA4_OUTPUT_ENABLE(1)
 * ************************|***********************************************
 * input_en                |        BMA4_INPUT_DISABLE(0)
 *                         |        BMA4_INPUT_ENABLE(1)
 * ************************************************************************
 *@endverbatim
 *
 *  @param[in] int_line : Variable used to select the interrupt pin1 or
 *  pin2 for interrupt configuration.
 *
 *@verbatim
 *       int_line          | Value
 *      -------------------|---------------
 *      BMA4_INTR1_MAP     |   0
 *      BMA4_INTR2_MAP     |   1
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_get_int_pin_config(struct bma4_int_pin_config *int_pin_config, uint8_t int_line, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiIntStatus Interrupt Status
 * @brief Read Feature interrupt and/or Hardware interrupt status from the sensor
 */

/*!
 * \ingroup bma4ApiIntStatus
 * \page bma4_api_bma4_read_int_status bma4_read_int_status
 * \code
 * int8_t bma4_read_int_status(uint16_t *int_status, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the Feature and Hardware interrupt status from the sensor.
 *
 *  @param[out] int_status : Variable used to get the interrupt status.
 *  @param[in] dev       : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_read_int_status(uint16_t *int_status, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiIntStatus
 * \page bma4_api_bma4_read_int_status_0 bma4_read_int_status_0
 * \code
 * int8_t bma4_read_int_status_0(uint8_t *int_status_0, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the Feature interrupt status from the sensor.
 *
 *  @param[out] int_status_0 : Variable used to get the interrupt status.
 *  @param[in] dev       : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_read_int_status_0(uint8_t *int_status_0, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiIntStatus
 * \page bma4_api_bma4_read_int_status_1 bma4_read_int_status_1
 * \code
 * int8_t bma4_read_int_status_1(uint8_t *int_status_1, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the Hardware interrupt status from the sensor.
 *
 *  @param[out] int_status_1 : Variable used to get the interrupt status.
 *  @param[in] dev       : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_read_int_status_1(uint8_t *int_status_1, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAux Auxiliary interface
 * @brief Initialize and read/write data to auxiliary interface
 */

/*!
 * \ingroup bma4ApiAux
 * \page bma4_api_bma4_aux_interface_init bma4_aux_interface_init
 * \code
 * int8_t bma4_aux_interface_init(struct bma4_dev *dev);
 * \endcode
 * @details This API initializes the auxiliary interface to access
 *          auxiliary sensor
 *
 *  @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_aux_interface_init(struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAux
 * \page bma4_api_bma4_aux_read bma4_aux_read
 * \code
 * int8_t bma4_aux_read(uint8_t aux_reg_addr, uint8_t *aux_data, uint16_t len, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the data from the auxiliary sensor
 *
 *  @param[in]  dev : Structure instance of bma4_dev.
 *  @param[in]  len : User specified data length
 *  @param[out] aux_data : Pointer variable to store data read
 *  @param[in]  aux_reg_addr : Variable to pass address from where
 *                      data is to be read
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_aux_read(uint8_t aux_reg_addr, uint8_t *aux_data, uint16_t len, struct bma4_dev *dev);

/*!
 * \ingroup bma4ApiAux
 * \page bma4_api_bma4_aux_write bma4_aux_write
 * \code
 * int8_t bma4_aux_write(uint8_t aux_reg_addr, uint8_t *aux_data, uint16_t len, struct bma4_dev *dev);
 * \endcode
 * @details This API writes the data into the auxiliary sensor
 *
 *  @param[in]  dev : Structure instance of bma4_dev.
 *  @param[in]  len : User specified data length
 *  @param[out] aux_data : Pointer variable to store data read
 *  @param[in]  aux_reg_addr : Variable to pass address from where
 *                      data is to be written
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_aux_write(uint8_t aux_reg_addr, const uint8_t *aux_data, uint16_t len, struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiSoftReset Soft Reset
 * @brief Perform Soft Reset of the sensor
 */

/*!
 * \ingroup bma4ApiSoftReset
 * \page bma4_api_bma4_soft_reset bma4_soft_reset
 * \code
 * int8_t bma4_soft_reset(struct bma4_dev *dev);
 * \endcode
 * @details This API commands to do soft reset
 *
 *  @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 *
 */
int8_t bma4_soft_reset(struct bma4_dev *dev);

/**
 * \ingroup bma4
 * \defgroup bma4ApiAccelFoc Accel FOC
 * @brief Performs Fast Offset Compensation for accel
 */

/*!
 * \ingroup bma4ApiAccelFoc
 * \page bma4_api_bma4_perform_accel_foc bma4_perform_accel_foc
 * \code
 * int8_t bma4_perform_accel_foc(const struct bma4_accel_foc_g_value *accel_g_value, struct bma4_dev *dev);
 * \endcode
 * @details This API performs Fast Offset Compensation for Accel.
 * @param[in] accel_g_value : Array which stores the Accel g units
 *  for x,y and z-axis.
 *
 *@verbatim
 *      accel_g_value             |   Description
 *      --------------------------|---------------------------------------
 *      accel_g_value[0]          |   x-axis g units
 *      accel_g_value[1]          |   y-axis g units
 *      accel_g_value[2]          |   z-axis g units
 *@endverbatim
 *
 *  @param[in] dev : Structure instance of dev.
 *
 *  @return Result of API execution status.
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
int8_t bma4_perform_accel_foc(const struct bma4_accel_foc_g_value *accel_g_value, struct bma4_dev *dev);

#endif

/* End of __BMA4_H__ */
