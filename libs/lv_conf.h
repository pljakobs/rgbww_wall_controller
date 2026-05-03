/**
 * lv_conf.h — LVGL v8 configuration for GUITION ESP32-S3-4848S040
 *
 * 480×480, RGB565, PSRAM draw buffer, esp_timer tick, widgets + benchmark demo.
 */

#if 1  /* Set this to 1 to enable content */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*===========================================================================
 * Color settings
 *==========================================================================*/
#define LV_COLOR_DEPTH     16      /* RGB565 */
#define LV_COLOR_16_SWAP   0       /* bytes already in correct order for RGB panel */

/*===========================================================================
 * Memory settings
 *==========================================================================*/
/* LVGL internal heap — allocated from PSRAM via lv_mem_alloc().
 * The draw buffers below are separate and also go to PSRAM. */
#define LV_MEM_CUSTOM      1
#if LV_MEM_CUSTOM
#  define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#  define LV_MEM_CUSTOM_ALLOC   malloc
#  define LV_MEM_CUSTOM_FREE    free
#  define LV_MEM_CUSTOM_REALLOC realloc
#else
#  define LV_MEM_SIZE (256U * 1024U)
#endif

/*===========================================================================
 * HAL tick
 *==========================================================================*/
/* Use esp_timer so we don't need a separate timer task */
#define LV_TICK_CUSTOM         1
#define LV_TICK_CUSTOM_INCLUDE "esp_timer.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR ((uint32_t)(esp_timer_get_time() / 1000LL))

/*===========================================================================
 * Display & input
 *==========================================================================*/
#define LV_HOR_RES_MAX    480
#define LV_VER_RES_MAX    480

/* DPI — 5-inch-class panel at 480 px ≈ 150 dpi */
#define LV_DPI_DEF        150

/*===========================================================================
 * Drawing
 *==========================================================================*/
/* Anti-aliasing — enable for smooth fonts/arcs */
#define LV_DRAW_ANTI_ALIASING  1

/* Shadows / gradients / image cache */
#define LV_SHADOW_CACHE_SIZE   0
#define LV_IMG_CACHE_DEF_SIZE  0

/*===========================================================================
 * Logging
 *==========================================================================*/
#define LV_USE_LOG     1
#define LV_LOG_LEVEL   LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF  1   /* use printf — Sming maps this to the IDF log */

/*===========================================================================
 * Asserts
 *==========================================================================*/
#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/*===========================================================================
 * Fonts
 *==========================================================================*/
#define LV_FONT_MONTSERRAT_14  1
#define LV_FONT_MONTSERRAT_16  1
#define LV_FONT_MONTSERRAT_24  1
#define LV_FONT_DEFAULT        &lv_font_montserrat_14

/*===========================================================================
 * Widgets
 *==========================================================================*/
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     1
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1
#define LV_USE_TABLE      1
#define LV_USE_CHART      1
#define LV_USE_COLORWHEEL 1
#define LV_USE_IMGBTN     1
#define LV_USE_KEYBOARD   1
#define LV_USE_LED        1
#define LV_USE_LIST       1
#define LV_USE_MENU       1
#define LV_USE_METER      1
#define LV_USE_MSGBOX     1
#define LV_USE_SPAN       1
#define LV_USE_SPINBOX    1
#define LV_USE_SPINNER    1
#define LV_USE_TABVIEW    1
#define LV_USE_TILEVIEW   1
#define LV_USE_WIN        1

/*===========================================================================
 * Extra themes / layouts
 *==========================================================================*/
#define LV_USE_THEME_DEFAULT    1
#define LV_THEME_DEFAULT_DARK   0
#define LV_USE_THEME_BASIC      1
#define LV_USE_THEME_MONO       0

#define LV_USE_FLEX   1
#define LV_USE_GRID   1

/*===========================================================================
 * Demo apps (used in application.cpp)
 *==========================================================================*/
#define LV_USE_DEMO_WIDGETS    1
#define LV_DEMO_WIDGETS_SLIDESHOW 0

#define LV_USE_DEMO_BENCHMARK  1
#define LV_USE_DEMO_STRESS     0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_MUSIC      0

/*===========================================================================
 * Performance monitor overlay (useful during bringup)
 *==========================================================================*/
#define LV_USE_PERF_MONITOR     1
#define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT

#endif /* LV_CONF_H */
#endif /* if 1 */
