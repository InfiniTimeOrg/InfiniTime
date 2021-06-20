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
 * @file       bma423.h
 * @date       2020-05-08
 * @version    V2.14.13
 *
 */

/**
 * \ingroup bma4xy
 * \defgroup bma423 BMA423
 * @brief Sensor driver for BMA423 sensor
 */

#ifndef BMA423_H
#define BMA423_H

#ifdef __cplusplus
extern "C" {
#endif
#include "bma4.h"

/**\name Chip ID of BMA423 sensor */
#define BMA423_CHIP_ID                 UINT8_C(0x11)
#define BMA425_CHIP_ID                 UINT8_C(0x13)

/**\ Configuration ID start position of BMA423 sensor */
#define BMA423_CONFIG_ID_START_ADDR    UINT8_C(66)

/**\name Sensor feature size */
#define BMA423_FEATURE_SIZE            UINT8_C(70)
#define BMA423_ANY_MOT_LEN             UINT8_C(4)

/**\name Feature offset address */
#define BMA423_ANY_MOT_OFFSET          UINT8_C(0x00)
#define BMA423_NO_MOT_OFFSET           UINT8_C(0x04)
#define BMA423_STEP_CNTR_PARAM_OFFSET  UINT8_C(0x08)
#define BMA423_STEP_CNTR_OFFSET        UINT8_C(0x3A)
#define BMA423_SINGLE_TAP_OFFSET       UINT8_C(0x3C)
#define BMA423_DOUBLE_TAP_OFFSET       UINT8_C(0x3E)
#define BMA423_WRIST_WEAR_OFFSET       UINT8_C(0x40)
#define BMA423_CONFIG_ID_OFFSET        UINT8_C(0x42)
#define BMA423_AXES_REMAP_OFFSET       UINT8_C(0x44)

/**\name Read/Write Lengths */
#define BMA423_RD_WR_MIN_LEN           UINT8_C(2)
#define BMA423_NO_MOT_RD_WR_LEN        (BMA423_ANY_MOT_LEN + BMA423_NO_MOT_OFFSET)

/*! @name Mask definitions for major and minor config */
#define BMA423_CONFIG_MAJOR_MSK        UINT16_C(0X3C0)
#define BMA423_CONFIG_MINOR_MSK        UINT8_C(0X1F)

/*! @name Bit position for major config */
#define BMA423_CONFIG_MAJOR_POS        UINT8_C(0X06)

/**************************************************************/
/**\name    Re-map Axes */
/**************************************************************/
#define BMA423_X_AXIS_MASK             UINT8_C(0x03)
#define BMA423_X_AXIS_SIGN_MASK        UINT8_C(0x04)
#define BMA423_Y_AXIS_MASK             UINT8_C(0x18)
#define BMA423_Y_AXIS_SIGN_MASK        UINT8_C(0x20)
#define BMA423_Z_AXIS_MASK             UINT8_C(0xC0)
#define BMA423_Z_AXIS_SIGN_MASK        UINT8_C(0x01)

/**************************************************************/
/**\name    Step Counter/Detector/Activity */
/**************************************************************/
/**\name Step counter/activity enable macros */
#define BMA423_STEP_CNTR_EN_MSK        UINT8_C(0x10)
#define BMA423_STEP_ACT_EN_MSK         UINT8_C(0x20)

/**\name Step counter water-mark macros */
#define BMA423_STEP_CNTR_WM_MSK        UINT16_C(0x03FF)

/**\name Step counter reset macros */
#define BMA423_STEP_CNTR_RST_POS       UINT8_C(2)
#define BMA423_STEP_CNTR_RST_MSK       UINT8_C(0x04)

/**\name Step detector enable macros */
#define BMA423_STEP_DETECTOR_EN_POS    UINT8_C(3)
#define BMA423_STEP_DETECTOR_EN_MSK    UINT8_C(0x08)

/**\name Wrist-wear enable macros */
#define BMA423_WRIST_WEAR_EN_MSK       UINT8_C(0x01)

/**\name Step count output length*/
#define BMA423_STEP_CNTR_DATA_SIZE     UINT16_C(4)

/**\name single tap enable macros */
#define BMA423_SINGLE_TAP_EN_MSK       UINT8_C(0x01)

/**\name double tap enable macros */
#define BMA423_DOUBLE_TAP_EN_MSK       UINT8_C(0x01)

/**\name tap sensitivity macros */
#define BMA423_TAP_SENS_POS            UINT8_C(1)
#define BMA423_TAP_SENS_MSK            UINT8_C(0x0E)

/**\name Tap selection macro */
#define BMA423_TAP_SEL_POS             UINT8_C(4)
#define BMA423_TAP_SEL_MSK             UINT8_C(0x10)

/**************************************************************/
/**\name    Any/no Motion */
/**************************************************************/
/**\name Any/No motion threshold macros */
#define BMA423_ANY_NO_MOT_THRES_MSK    UINT16_C(0x07FF)

/**\name Any/No motion duration macros */
#define BMA423_ANY_NO_MOT_DUR_MSK      UINT16_C(0x1FFF)

/**\name Any/No motion enable macros */
#define BMA423_ANY_NO_MOT_AXIS_EN_POS  UINT8_C(0x0D)
#define BMA423_ANY_NO_MOT_AXIS_EN_MSK  UINT16_C(0xE000)

/**************************************************************/
/**\name    User macros */
/**************************************************************/
/**\name Any-motion/No-motion axis enable macros */
#define BMA423_X_AXIS_EN               UINT8_C(0x01)
#define BMA423_Y_AXIS_EN               UINT8_C(0x02)
#define BMA423_Z_AXIS_EN               UINT8_C(0x04)
#define BMA423_EN_ALL_AXIS             UINT8_C(0x07)
#define BMA423_DIS_ALL_AXIS            UINT8_C(0x00)

/**\name Feature enable macros for the sensor */
#define BMA423_STEP_CNTR               UINT8_C(0x01)
#define BMA423_STEP_ACT                UINT8_C(0x02)
#define BMA423_WRIST_WEAR              UINT8_C(0x04)
#define BMA423_SINGLE_TAP              UINT8_C(0x08)
#define BMA423_DOUBLE_TAP              UINT8_C(0x10)

/**\name Interrupt status macros */
#define BMA423_SINGLE_TAP_INT          UINT8_C(0x01)
#define BMA423_STEP_CNTR_INT           UINT8_C(0x02)
#define BMA423_ACTIVITY_INT            UINT8_C(0x04)
#define BMA423_WRIST_WEAR_INT          UINT8_C(0x08)
#define BMA423_DOUBLE_TAP_INT          UINT8_C(0x10)
#define BMA423_ANY_MOT_INT             UINT8_C(0x20)
#define BMA423_NO_MOT_INT              UINT8_C(0x40)
#define BMA423_ERROR_INT               UINT8_C(0x80)

/**\name Activity recognition macros */
#define BMA423_USER_STATIONARY         UINT8_C(0x00)
#define BMA423_USER_WALKING            UINT8_C(0x01)
#define BMA423_USER_RUNNING            UINT8_C(0x02)
#define BMA423_STATE_INVALID           UINT8_C(0x03)

/******************************************************************************/
/*!  @name         Structure Declarations                             */
/******************************************************************************/

/*!
 * @brief Any/No motion configuration
 */
struct bma423_any_no_mot_config
{
    /*! Expressed in 50 Hz samples (20 ms) */
    uint16_t duration;

    /*! Threshold value for Any-motion/No-motion detection in
     * 5.11g format
     */
    uint16_t threshold;

    /*! To enable selected axes */
    uint8_t axes_en;
};

/*!
 * @brief Axes re-mapping configuration
 */
struct bma423_axes_remap
{
    /*! Re-mapped x-axis */
    uint8_t x_axis;

    /*! Re-mapped y-axis */
    uint8_t y_axis;

    /*! Re-mapped z-axis */
    uint8_t z_axis;

    /*! Re-mapped x-axis sign */
    uint8_t x_axis_sign;

    /*! Re-mapped y-axis sign */
    uint8_t y_axis_sign;

    /*! Re-mapped z-axis sign */
    uint8_t z_axis_sign;
};

/*!
 * @brief Step counter param settings
 */
struct bma423_stepcounter_settings
{
    /*! Step Counter param 1 */
    uint16_t param1;

    /*! Step Counter param 2 */
    uint16_t param2;

    /*! Step Counter param 3 */
    uint16_t param3;

    /*! Step Counter param 4 */
    uint16_t param4;

    /*! Step Counter param 5 */
    uint16_t param5;

    /*! Step Counter param 6 */
    uint16_t param6;

    /*! Step Counter param 7 */
    uint16_t param7;

    /*! Step Counter param 8 */
    uint16_t param8;

    /*! Step Counter param 9 */
    uint16_t param9;

    /*! Step Counter param 10 */
    uint16_t param10;

    /*! Step Counter param 11 */
    uint16_t param11;

    /*! Step Counter param 12 */
    uint16_t param12;

    /*! Step Counter param 13 */
    uint16_t param13;

    /*! Step Counter param 14 */
    uint16_t param14;

    /*! Step Counter param 15 */
    uint16_t param15;

    /*! Step Counter param 16 */
    uint16_t param16;

    /*! Step Counter param 17 */
    uint16_t param17;

    /*! Step Counter param 18 */
    uint16_t param18;

    /*! Step Counter param 19 */
    uint16_t param19;

    /*! Step Counter param 20 */
    uint16_t param20;

    /*! Step Counter param 21 */
    uint16_t param21;

    /*! Step Counter param 22 */
    uint16_t param22;

    /*! Step Counter param 23 */
    uint16_t param23;

    /*! Step Counter param 24 */
    uint16_t param24;

    /*! Step Counter param 25 */
    uint16_t param25;
};

/***************************************************************************/

/*!     BMA423 User Interface function prototypes
 ****************************************************************************/

/**
 * \ingroup bma423
 * \defgroup bma423ApiInit Initialization
 * @brief Initialize the sensor and device structure
 */

/*!
 * \ingroup bma423ApiInit
 * \page bma423_api_bma423_init bma423_init
 * \code
 * int8_t bma423_init(struct bma4_dev *dev);
 * \endcode
 * @details This API is the entry point.
 * Call this API before using all other APIs.
 * This API reads the chip-id of the sensor and sets the resolution.
 *
 * @param[in,out] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_init(struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiConfig ConfigFile
 * @brief Write binary configuration in the sensor
 */

/*!
 * \ingroup bma423ApiConfig
 * \page bma423_api_bma423_write_config_file bma423_write_config_file
 * \code
 * int8_t bma423_write_config_file(struct bma4_dev *dev);
 * \endcode
 * @details This API is used to upload the config file to enable the features of
 * the sensor.
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_write_config_file(struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiConfigId ConfigId
 * @brief Get Configuration ID of the sensor
 */

/*!
 * \ingroup bma423ApiConfig
 * \page bma423_api_bma423_get_config_id bma423_get_config_id
 * \code
 * int8_t bma423_get_config_id(uint16_t *config_id, struct bma4_dev *dev);
 * \endcode
 * @details This API is used to get the configuration id of the sensor.
 *
 * @param[out] config_id : Pointer variable used to store the configuration id.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_get_config_id(uint16_t *config_id, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiMapInt Map / Unmap Interrupt
 * @brief Map / Unmap user provided interrupt to interrupt pin1 or pin2 of the sensor
 */

/*!
 * \ingroup bma423ApiMapInt
 * \page bma423_api_bma423_map_interrupt bma423_map_interrupt
 * \code
 * int8_t bma423_map_interrupt(uint8_t int_line, uint16_t int_map, uint8_t enable, struct bma4_dev *dev);
 * \endcode
 * @details This API sets/unsets the user provided interrupt to either
 * interrupt pin1 or pin2 in the sensor.
 *
 * @param[in] int_line: Variable to select either interrupt pin1 or pin2.
 *
 *@verbatim
 *  int_line    |   Macros
 *  ------------|-------------------
 *  0x00        |  BMA4_INTR1_MAP
 *  0x01      |  BMA4_INTR2_MAP
 *@endverbatim
 *
 * @param[in] int_map : Variable to specify the interrupts.
 * @param[in] enable : Variable to specify mapping or unmapping of interrupts.
 *
 *@verbatim
 *  enable  |   Macros
 *  --------|-------------------
 *  0x00    |  BMA4_DISABLE
 *  0x01    |  BMA4_ENABLE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @note Below macros specify the interrupts.
 *
 * Feature Interrupts
 *  - BMA423_STEP_CNTR_INT
 *  - BMA423_ACTIVITY_INT
 *  - BMA423_WRIST_WEAR_INT
 *  - BMA423_WAKEUP_INT
 *  - BMA423_ANY_MOT_INT
 *  - BMA423_NO_MOT_INT
 *  - BMA423_ERROR_INT
 *
 * Hardware Interrupts
 *  - BMA4_FIFO_FULL_INT
 *  - BMA4_FIFO_WM_INT
 *  - BMA4_DATA_RDY_INT
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_map_interrupt(uint8_t int_line, uint16_t int_map, uint8_t enable, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiIntS Interrupt Status
 * @brief Read interrupt status of the sensor
 */

/*!
 * \ingroup bma423ApiIntS
 * \page bma423_api_bma423_read_int_status bma423_read_int_status
 * \code
 * int8_t bma423_read_int_status(uint16_t *int_status, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the bma423 interrupt status from the sensor.
 *
 * @param[out] int_status : Variable to store the interrupt status read from
 * the sensor.
 * @param[in] dev : Structure instance of bma4_dev.
 *
 *  @note Below macros are used to check the interrupt status.
 *
 * Feature Interrupts
 *  - BMA423_STEP_CNTR_INT
 *  - BMA423_ACTIVITY_INT
 *  - BMA423_WRIST_WEAR_INT
 *  - BMA423_WAKEUP_INT
 *  - BMA423_ANY_MOT_INT
 *  - BMA423_NO_MOT_INT
 *  - BMA423_ERROR_INT
 *
 * Hardware Interrupts
 *  - BMA4_FIFO_FULL_INT
 *  - BMA4_FIFO_WM_INT
 *  - BMA4_DATA_RDY_INT
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_read_int_status(uint16_t *int_status, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiFeat Sensor Feature
 * @brief Enables / Disables features of the sensor
 */

/*!
 * \ingroup bma423ApiFeat
 * \page bma423_api_bma423_feature_enable bma423_feature_enable
 * \code
 * int8_t bma423_feature_enable(uint8_t feature, uint8_t enable, struct bma4_dev *dev);
 * \endcode
 * @details This API enables/disables the features of the sensor.
 *
 * @param[in] feature : Variable to specify the features which are to be set in
 * bma423 sensor.
 * @param[in] enable : Variable which specifies whether to enable or disable the
 * features in the bma456 sensor.
 *
 *@verbatim
 *  enable  |   Macros
 *  --------|-------------------
 *  0x00    |  BMA4_DISABLE
 *  0x01    |  BMA4_ENABLE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev.
 *
 * @note User should use the below macros to enable or disable the
 * features of bma423 sensor
 *
 *    - BMA423_STEP_CNTR
 *    - BMA423_ACTIVITY
 *    - BMA423_WAKEUP
 *    - BMA423_WRIST_WEAR
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_feature_enable(uint8_t feature, uint8_t enable, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiRemap Remap Axes
 * @brief Set / Get x, y and z axis re-mapping in the sensor
 */

/*!
 * \ingroup bma423ApiRemap
 * \page bma423_api_bma423_set_remap_axes bma423_set_remap_axes
 * \code
 * int8_t bma423_set_remap_axes(const struct bma423_axes_remap *remap_data, struct bma4_dev *dev);
 * \endcode
 * @details This API performs x, y and z axis remapping in the sensor.
 *
 * @param[in] remap_data : Pointer to store axes remapping data.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_set_remap_axes(const struct bma423_axes_remap *remap_data, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiRemap
 * \page bma423_api_bma423_get_remap_axes bma423_get_remap_axes
 * \code
 * int8_t bma423_get_remap_axes(struct bma423_axes_remap *remap_data, struct bma4_dev *dev);
 * \endcode
 * @details This API reads the x, y and z axis remap data from the sensor.
 *
 * @param[out] remap_data : Pointer to store axis remap data which is read
 * from the bma423 sensor.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_get_remap_axes(struct bma423_axes_remap *remap_data, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiStepC Step counter
 * @brief Operations of step counter feature of the sensor
 */

/*!
 * \ingroup bma423ApiStepC
 * \page bma423_api_bma423_step_counter_set_watermark bma423_step_counter_set_watermark
 * \code
 * int8_t bma423_step_counter_set_watermark(uint16_t step_counter_wm, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the watermark level for step counter interrupt in
 * the sensor.
 *
 * @param[in] step_counter_wm : Variable which specifies watermark level
 * count
 * @note Valid values are from 1 to 1023
 * @note Value 0 is used for step detector interrupt
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_step_counter_set_watermark(uint16_t step_counter_wm, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiStepC
 * \page bma423_api_bma423_step_counter_get_watermark bma423_step_counter_get_watermark
 * \code
 * int8_t bma423_step_counter_get_watermark(uint16_t *step_counter_wm, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the water mark level set for step counter interrupt
 * in the sensor
 *
 * @param[out] step_counter_wm : Pointer variable which stores the water mark
 * level read from the sensor.
 * @note valid values are from 1 to 1023
 * @note value 0 is used for step detector interrupt
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_step_counter_get_watermark(uint16_t *step_counter_wm, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiStepC
 * \page bma423_api_bma423_reset_step_counter bma423_reset_step_counter
 * \code
 * int8_t bma423_reset_step_counter(struct bma4_dev *dev);
 * \endcode
 * @details This API resets the counted steps of step counter.
 *
 * @param[in] dev : structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_reset_step_counter(struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiStepC
 * \page bma423_api_bma423_step_counter_output bma423_step_counter_output
 * \code
 * int8_t bma423_step_counter_output(uint32_t *step_count, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the number of counted steps of the step counter
 * feature from the sensor.
 *
 * @param[out] step_count : Pointer variable which stores counted steps
 * read from the sensor.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_step_counter_output(uint32_t *step_count, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiAct Activity Feature
 * @brief Get output for activity feature of the sensor
 */

/*!
 * \ingroup bma423ApiAct
 * \page bma423_api_bma423_activity_output bma423_activity_output
 * \code
 * int8_t bma423_activity_output(uint8_t *activity, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the output for activity feature.
 *
 * @param[out] activity : Pointer variable which stores activity output read
 * from the sensor.
 *
 *@verbatim
 *       activity |   State
 *  --------------|------------------------
 *        0x00    | BMA423_USER_STATIONARY
 *        0x01    | BMA423_USER_WALKING
 *        0x02    | BMA423_USER_RUNNING
 *        0x03    | BMA423_STATE_INVALID
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_activity_output(uint8_t *activity, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiStepC
 * \page bma423_api_bma423_stepcounter_get_parameter bma423_stepcounter_get_parameter
 * \code
 * int8_t bma423_stepcounter_get_parameter(struct bma423_stepcounter_settings *setting, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the parameter1 to parameter7 settings of the step
 * counter feature.
 *
 * @param[out] setting : Pointer to structure variable which stores the
 * parameter1 to parameter7 read from the sensor.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_stepcounter_get_parameter(struct bma423_stepcounter_settings *setting, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiStepC
 * \page bma423_api_bma423_stepcounter_set_parameter bma423_stepcounter_set_parameter
 * \code
 * int8_t bma423_stepcounter_set_parameter(const struct bma423_stepcounter_settings *setting, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the parameter1 to parameter7 settings of the step
 * counter feature in the sensor.
 *
 * @param[in] setting : Pointer to structure variable which stores the
 * parameter1 to parameter7 settings read from the sensor.
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_stepcounter_set_parameter(const struct bma423_stepcounter_settings *setting, struct bma4_dev *dev);

/**
 * \ingroup bma421
 * \defgroup bma421ApiStepD Step detector
 * @brief Operations of step detector feature of the sensor
 */

/*!
 * \ingroup bma421ApiStepD
 * \page bma421_api_bma421_step_detector_enable bma421_step_detector_enable
 * \code
 * int8_t bma421_step_detector_enable(uint8_t enable, struct bma4_dev *dev);
 * \endcode
 * @details This API enables or disables the step detector feature in the
 * sensor.
 *
 * @param[in] enable : Variable used to enable or disable step detector
 *
 *@verbatim
 *  enable  |   Macros
 *  --------|-------------------
 *  0x00    |  BMA4_DISABLE
 *  0x01    |  BMA4_ENABLE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_step_detector_enable(uint8_t enable, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiAnyMot Any motion Feature
 * @brief Functions of Any motion feature of the sensor
 */

/*!
 * \ingroup bma423ApiAnyMot
 * \page bma423_api_bma423_set_any_motion_config bma423_set_any_motion_config
 * \code
 * int8_t bma423_set_any_motion_config(const struct bma423_anymotion_config *any_motion, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the configuration of any-motion feature in the sensor
 * This API enables/disables the any-motion feature according to the axis set.
 *
 * @param[in] any_mot           : Pointer to structure variable to configure
 *                                any-motion.
 *
 * @verbatim
 * -------------------------------------------------------------------------
 *         Structure parameters    |        Description
 * --------------------------------|----------------------------------------
 *                                 |        Defines the number of
 *                                 |        consecutive data points for
 *                                 |        which the threshold condition
 *         duration                |        must be respected, for interrupt
 *                                 |        assertion. It is expressed in
 *                                 |        50 Hz samples (20 ms).
 *                                 |        Range is 0 to 163sec.
 *                                 |        Default value is 5 = 100ms.
 * --------------------------------|----------------------------------------
 *                                 |        Slope threshold value for
 *                                 |        Any-motion detection
 *         threshold               |        in 5.11g format.
 *                                 |        Range is 0 to 1g.
 *                                 |        Default value is 0xAA = 83mg.
 * --------------------------------|----------------------------------------
 *                                 |        Enables the feature on a per-axis
 *         axis_en                 |        basis.
 * ---------------------------------------------------------------------------
 * @endverbatim
 *
 *@verbatim
 *  Value    |  axis_en
 *  ---------|-------------------------
 *  0x00     |  BMA423_DIS_ALL_AXIS
 *  0x01     |  BMA423_X_AXIS_EN
 *  0x02     |  BMA423_Y_AXIS_EN
 *  0x04     |  BMA423_Z_AXIS_EN
 *  0x07     |  BMA423_EN_ALL_AXIS
 *@endverbatim
 *
 * @param[in] dev               : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_set_any_mot_config(const struct bma423_any_no_mot_config *any_mot, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiAnyMot
 * \page bma423_api_bma423_get_any_motion_config bma423_get_any_motion_config
 * \code
 * int8_t bma423_get_any_motion_config(struct bma423_anymotion_config *any_motion, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the configuration of any-motion feature from the
 * sensor.
 *
 * @param[out] any_mot        : Pointer to structure variable to configure
 *                              any-motion.
 *
 * @verbatim
 * -------------------------------------------------------------------------
 *         Structure parameters    |        Description
 * --------------------------------|----------------------------------------
 *                                 |        Defines the number of
 *                                 |        consecutive data points for
 *                                 |        which the threshold condition
 *         duration                |        must be respected, for interrupt
 *                                 |        assertion. It is expressed in
 *                                 |        50 Hz samples (20 ms).
 *                                 |        Range is 0 to 163sec.
 *                                 |        Default value is 5 = 100ms.
 * --------------------------------|----------------------------------------
 *                                 |        Slope threshold value for
 *                                 |        Any-motion detection
 *         threshold               |        in 5.11g format.
 *                                 |        Range is 0 to 1g.
 *                                 |        Default value is 0xAA = 83mg.
 * --------------------------------|-----------------------------------------
 *                                 |        Enables the feature on a per-axis
 *         axis_en                 |        basis.
 * ---------------------------------------------------------------------------
 * @endverbatim
 *
 *@verbatim
 *  Value    |  axis_en
 *  ---------|-------------------------
 *  0x00     |  BMA423_DIS_ALL_AXIS
 *  0x01     |  BMA423_X_AXIS_EN
 *  0x02     |  BMA423_Y_AXIS_EN
 *  0x04     |  BMA423_Z_AXIS_EN
 *  0x07     |  BMA423_EN_ALL_AXIS
 *@endverbatim
 *
 * @param[in] dev               : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_get_any_mot_config(struct bma423_any_no_mot_config *any_mot, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiNomot No-Motion Feature
 * @brief Operations of no-motion feature of the sensor
 */

/*!
 * \ingroup bma423ApiNomot
 * \page bma423_api_bma423_set_no_motion_config bma423_set_no_motion_config
 * \code
 * int8_t bma423_set_no_motion_config(const struct bma423_nomotion_config *no_motion, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the configuration of no-motion feature in the sensor
 * This API enables/disables the no-motion feature according to the axis set.
 *
 * @param[in] no_mot                : Pointer to structure variable to configure
 *                                  no-motion.
 *
 * @verbatim
 * -------------------------------------------------------------------------
 *         Structure parameters    |        Description
 * --------------------------------|----------------------------------------
 *                                 |        Defines the number of
 *                                 |        consecutive data points for
 *                                 |        which the threshold condition
 *         duration                |        must be respected, for interrupt
 *                                 |        assertion. It is expressed in
 *                                 |        50 Hz samples (20 ms).
 *                                 |        Range is 0 to 163sec.
 *                                 |        Default value is 5 = 100ms.
 * --------------------------------|----------------------------------------
 *                                 |        Slope threshold value for
 *                                 |        No-motion detection
 *         threshold               |        in 5.11g format.
 *                                 |        Range is 0 to 1g.
 *                                 |        Default value is 0xAA = 83mg.
 * --------------------------------|----------------------------------------
 *                                 |        Enables the feature on a per-axis
 *         axis_en                 |        basis.
 * ---------------------------------------------------------------------------
 * @endverbatim
 *
 *@verbatim
 *  Value    |  axis_en
 *  ---------|-------------------------
 *  0x00     |  BMA423_DIS_ALL_AXIS
 *  0x01     |  BMA423_X_AXIS_EN
 *  0x02     |  BMA423_Y_AXIS_EN
 *  0x04     |  BMA423_Z_AXIS_EN
 *  0x07     |  BMA423_EN_ALL_AXIS
 *@endverbatim
 *
 * @param[in] dev               : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_set_no_mot_config(const struct bma423_any_no_mot_config *no_mot, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiNomot
 * \page bma423_api_bma423_get_no_motion_config bma423_get_no_motion_config
 * \code
 * int8_t bma423_get_no_motion_config(struct bma423_nomotion_config *no_motion, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the configuration of no-motion feature from the
 * sensor.
 *
 * @param[out] no_mot        : Pointer to structure variable to configure
 *                              no-motion.
 *
 * @verbatim
 * -------------------------------------------------------------------------
 *         Structure parameters    |        Description
 * --------------------------------|----------------------------------------
 *                                 |        Defines the number of
 *                                 |        consecutive data points for
 *                                 |        which the threshold condition
 *         duration                |        must be respected, for interrupt
 *                                 |        assertion. It is expressed in
 *                                 |        50 Hz samples (20 ms).
 *                                 |        Range is 0 to 163sec.
 *                                 |        Default value is 5 = 100ms.
 * --------------------------------|----------------------------------------
 *                                 |        Slope threshold value for
 *                                 |        No-motion detection
 *         threshold               |        in 5.11g format.
 *                                 |        Range is 0 to 1g.
 *                                 |        Default value is 0xAA = 83mg.
 * --------------------------------|-----------------------------------------
 *                                 |        Enables the feature on a per-axis
 *         axis_en                 |        basis.
 * ---------------------------------------------------------------------------
 * @endverbatim
 *
 *@verbatim
 *  Value    |  axis_en
 *  ---------|-------------------------
 *  0x00     |  BMA423_DIS_ALL_AXIS
 *  0x01     |  BMA423_X_AXIS_EN
 *  0x02     |  BMA423_Y_AXIS_EN
 *  0x04     |  BMA423_Z_AXIS_EN
 *  0x07     |  BMA423_EN_ALL_AXIS
 *@endverbatim
 *
 * @param[in] dev               : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_get_no_mot_config(struct bma423_any_no_mot_config *no_mot, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiTap Single Tap and Double tap
 * @brief Single and Double tap feature operations
 */

/*!
 * \ingroup bma423ApiTap
 * \page bma423_api_bma423_single_tap_set_sensitivity bma423_single_tap_set_sensitivity
 * \code
 * int8_t bma423_single_tap_set_sensitivity(uint8_t sensitivity, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the sensitivity of single tap wake-up feature in the sensor
 *
 * @param[in] sensitivity : Variable used to specify the sensitivity of the
 * Wake up feature.
 *
 *@verbatim
 *  Value   |  Sensitivity
 *  --------|-------------------------
 *  0x00    |  MOST SENSITIVE
 *  0x07    |  LEAST SENSITIVE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_single_tap_set_sensitivity(uint8_t sensitivity, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiTap
 * \page bma423_api_bma423_double_tap_set_sensitivity bma423_double_tap_set_sensitivity
 * \code
 * int8_t bma423_double_tap_set_sensitivity(uint8_t sensitivity, struct bma4_dev *dev);
 * \endcode
 * @details This API sets the sensitivity of double tap wake-up feature in the sensor
 *
 * @param[in] sensitivity : Variable used to specify the sensitivity of the
 * Wake up feature.
 *
 *@verbatim
 *  Value   |  Sensitivity
 *  --------|-------------------------
 *  0x00    |  MOST SENSITIVE
 *  0x07    |  LEAST SENSITIVE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_double_tap_set_sensitivity(uint8_t sensitivity, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiTap
 * \page bma423_api_bma423_single_tap_get_sensitivity bma423_single_tap_get_sensitivity
 * \code
 * int8_t bma423_single_tap_get_sensitivity(uint8_t *sensitivity, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the sensitivity of single tap wake up feature in the sensor
 *
 * @param[out] sensitivity : Pointer variable which stores the sensitivity
 * value read from the sensor.
 *
 * @verbatim
 *  Value   |  Sensitivity
 *  --------|-------------------------
 *  0x00    |  MOST SENSITIVE
 *  0x07    |  LEAST SENSITIVE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_single_tap_get_sensitivity(uint8_t *sensitivity, struct bma4_dev *dev);

/*!
 * \ingroup bma423ApiTap
 * \page bma423_api_bma423_double_tap_get_sensitivity bma423_double_tap_get_sensitivity
 * \code
 * int8_t bma423_double_tap_get_sensitivity(uint8_t *sensitivity, struct bma4_dev *dev);
 * \endcode
 * @details This API gets the sensitivity of double tap wake up feature in the sensor
 *
 * @param[out] sensitivity : Pointer variable which stores the sensitivity
 * value read from the sensor.
 *
 *@verbatim
 *  Value   |  Sensitivity
 *  --------|-------------------------
 *  0x00    |  MOST SENSITIVE
 *  0x07    |  LEAST SENSITIVE
 *@endverbatim
 *
 * @param[in] dev : Structure instance of bma4_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bma423_double_tap_get_sensitivity(uint8_t *sensitivity, struct bma4_dev *dev);

/**
 * \ingroup bma423
 * \defgroup bma423ApiVersionConfig Version Config
 * @brief Get version configuration of the sensor
 */

/*!
 * \ingroup bma423ApiVersionConfig
 * \page bma423_api_bma422_huawei_get_version_config bma423_get_version_config
 * \code
 * int8_t bma423_get_version_config(uint16_t *config_major, uint16_t *config_minor, struct bma4_dev *dev);
 * \endcode
 * @details This API is used to get the config file major and minor information.
 *
 * @param[in] dev   : Structure instance of bma4_dev.
 * @param[out] config_major    : Pointer to data buffer to store the config major.
 * @param[out] config_minor    : Pointer to data buffer to store the config minor.
 *
 * @retval BMA4_OK - Success.
 * @retval BMA4_E_NULL_PTR - Error: Null pointer error
 */
int8_t bma423_get_version_config(uint16_t *config_major, uint16_t *config_minor, struct bma4_dev *dev);

#ifdef __cplusplus
}
#endif /*End of CPP guard */

#endif /*End of header guard macro */
