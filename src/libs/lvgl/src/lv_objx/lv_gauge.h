/**
 * @file lv_gauge.h
 *
 */

#ifndef LV_GAUGE_H
#define LV_GAUGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_GAUGE != 0

/*Testing of dependencies*/
#if LV_USE_LMETER == 0
#error "lv_gauge: lv_lmeter is required. Enable it in lv_conf.h (LV_USE_LMETER  1) "
#endif

#include "../lv_core/lv_obj.h"
#include "lv_lmeter.h"
#include "lv_label.h"
#include "lv_line.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of gauge*/
typedef struct
{
    lv_lmeter_ext_t lmeter; /*Ext. of ancestor*/
    /*New data for this type */
    int16_t * values;                 /*Array of the set values (for needles) */
    const lv_color_t * needle_colors; /*Color of the needles (lv_color_t my_colors[needle_num])*/
    uint8_t needle_count;             /*Number of needles*/
    uint8_t label_count;              /*Number of labels on the scale*/
} lv_gauge_ext_t;

/*Styles*/
enum {
    LV_GAUGE_STYLE_MAIN,
};
typedef uint8_t lv_gauge_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a gauge objects
 * @param par pointer to an object, it will be the parent of the new gauge
 * @param copy pointer to a gauge object, if not NULL then the new object will be copied from it
 * @return pointer to the created gauge
 */
lv_obj_t * lv_gauge_create(lv_obj_t * par, const lv_obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the number of needles
 * @param gauge pointer to gauge object
 * @param needle_cnt new count of needles
 * @param colors an array of colors for needles (with 'num' elements)
 */
void lv_gauge_set_needle_count(lv_obj_t * gauge, uint8_t needle_cnt, const lv_color_t colors[]);

/**
 * Set the value of a needle
 * @param gauge pointer to a gauge
 * @param needle_id the id of the needle
 * @param value the new value
 */
void lv_gauge_set_value(lv_obj_t * gauge, uint8_t needle_id, int16_t value);

/**
 * Set minimum and the maximum values of a gauge
 * @param gauge pointer to he gauge object
 * @param min minimum value
 * @param max maximum value
 */
static inline void lv_gauge_set_range(lv_obj_t * gauge, int16_t min, int16_t max)
{
    lv_lmeter_set_range(gauge, min, max);
}

/**
 * Set a critical value on the scale. After this value 'line.color' scale lines will be drawn
 * @param gauge pointer to a gauge object
 * @param value the critical value
 */
static inline void lv_gauge_set_critical_value(lv_obj_t * gauge, int16_t value)
{
    lv_lmeter_set_value(gauge, value);
}

/**
 * Set the scale settings of a gauge
 * @param gauge pointer to a gauge object
 * @param angle angle of the scale (0..360)
 * @param line_cnt count of scale lines.
 * The get a given "subdivision" lines between label, `line_cnt` = (sub_div + 1) * (label_cnt - 1) +
 * 1
 * @param label_cnt count of scale labels.
 */
void lv_gauge_set_scale(lv_obj_t * gauge, uint16_t angle, uint8_t line_cnt, uint8_t label_cnt);

/**
 * Set the styles of a gauge
 * @param gauge pointer to a gauge object
 * @param type which style should be set (can be only `LV_GAUGE_STYLE_MAIN`)
 * @param style set the style of the gauge
 *  */
static inline void lv_gauge_set_style(lv_obj_t * gauge, lv_gauge_style_t type, lv_style_t * style)
{
    (void)type; /*Unused*/
    lv_obj_set_style(gauge, style);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a needle
 * @param gauge pointer to gauge object
 * @param needle the id of the needle
 * @return the value of the needle [min,max]
 */
int16_t lv_gauge_get_value(const lv_obj_t * gauge, uint8_t needle);

/**
 * Get the count of needles on a gauge
 * @param gauge pointer to gauge
 * @return count of needles
 */
uint8_t lv_gauge_get_needle_count(const lv_obj_t * gauge);

/**
 * Get the minimum value of a gauge
 * @param gauge pointer to a gauge object
 * @return the minimum value of the gauge
 */
static inline int16_t lv_gauge_get_min_value(const lv_obj_t * lmeter)
{
    return lv_lmeter_get_min_value(lmeter);
}

/**
 * Get the maximum value of a gauge
 * @param gauge pointer to a gauge object
 * @return the maximum value of the gauge
 */
static inline int16_t lv_gauge_get_max_value(const lv_obj_t * lmeter)
{
    return lv_lmeter_get_max_value(lmeter);
}

/**
 * Get a critical value on the scale.
 * @param gauge pointer to a gauge object
 * @return the critical value
 */
static inline int16_t lv_gauge_get_critical_value(const lv_obj_t * gauge)
{
    return lv_lmeter_get_value(gauge);
}

/**
 * Set the number of labels (and the thicker lines too)
 * @param gauge pointer to a gauge object
 * @return count of labels
 */
uint8_t lv_gauge_get_label_count(const lv_obj_t * gauge);

/**
 * Get the scale number of a gauge
 * @param gauge pointer to a gauge object
 * @return number of the scale units
 */
static inline uint16_t lv_gauge_get_line_count(const lv_obj_t * gauge)
{
    return lv_lmeter_get_line_count(gauge);
}

/**
 * Get the scale angle of a gauge
 * @param gauge pointer to a gauge object
 * @return angle of the scale
 */
static inline uint16_t lv_gauge_get_scale_angle(const lv_obj_t * gauge)
{
    return lv_lmeter_get_scale_angle(gauge);
}

/**
 * Get the style of a gauge
 * @param gauge pointer to a gauge object
 * @param type which style should be get (can be only `LV_GAUGE_STYLE_MAIN`)
 * @return pointer to the gauge's style
 */
static inline const lv_style_t * lv_gauge_get_style(const lv_obj_t * gauge, lv_gauge_style_t type)
{
    (void)type; /*Unused*/
    return lv_obj_get_style(gauge);
}

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GAUGE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_GAUGE_H*/
