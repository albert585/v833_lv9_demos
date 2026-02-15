/**
 * @file lv_lib_100ask_conf.h
 * Configuration file for v9.1
 *
 */

#ifndef LV_LIB_100ASK_CONF_H
#define LV_LIB_100ASK_CONF_H

#include "lv_conf.h"

#include "lvgl/src/lvgl_private.h"

/*******************
 * GENERAL SETTING
 *******************/

/*********************
 * USAGE
 *********************/

/* Simplified Pinyin input method */
#if LV_USE_KEYBOARD
    /* Requires LV_USE_KEYBOARD = 1 */
    #define LV_USE_100ASK_PINYIN_IME                    0
#endif



/* Page manager */

/*screenshot*/
#if LV_USE_SNAPSHOT
    /* Requires LV_USE_SNAPSHOT = 1 */
    #define LV_USE_100ASK_SCREENSHOT                    0
#endif
#if LV_USE_100ASK_SCREENSHOT && LV_USE_LODEPNG
    /*test*/
    #define LV_USE_100ASK_SCREENSHOT_TEST               0
#endif

/* sketchpad */
#if LV_USE_CANVAS
    /* Requires LV_USE_CANVAS = 1 */
    #define LV_USE_100ASK_SKETCHPAD                     0
#endif
#if LV_USE_100ASK_SKETCHPAD
    /* set sketchpad default size */
    #define SKETCHPAD_DEFAULT_WIDTH                     1024    /*LV_HOR_RES*/
    #define SKETCHPAD_DEFAULT_HEIGHT                    600     /*LV_VER_RES*/

    /*test*/
    #define LV_100ASK_SKETCHPAD_SIMPLE_TEST             0
#endif


/*Calculator*/
#define LV_USE_100ASK_CALC                              0
#if LV_USE_100ASK_CALC
    /*Calculation expression*/
    #define LV_100ASK_CALC_EXPR_LEN                      (128) /*Maximum allowed length of expression*/
    #define LV_100ASK_CALC_MAX_NUM_LEN                   (5)   /*Maximum length of operands allowed*/

    /*test*/
    #define LV_100ASK_CALC_SIMPLE_TEST                  0
#endif

/*GAME*/
/*Memory game*/
#define LV_USE_100ASK_MEMORY_GAME                       0
#if LV_USE_100ASK_MEMORY_GAME
    /*Initial values of rows and columns.*/
    /*Recommended row == column*/
    #define  LV_100ASK_MEMORY_GAME_DEFAULT_ROW          4
    #define  LV_100ASK_MEMORY_GAME_DEFAULT_COL          4

    /*test*/
    #define  LV_100ASK_MEMORY_GAME_SIMPLE_TEST          0
#endif

/*2048 game*/
#define LV_USE_100ASK_2048                              1
#if LV_USE_100ASK_2048
    /* Matrix size*/
    /*Do not modify*/
    #define  LV_100ASK_2048_MATRIX_SIZE                 4

    /*test*/
    #define  LV_100ASK_2048_SIMPLE_TEST                 0
#endif

/*File explorer*/
/*NES*/
#if LV_USE_IMAGE
    #define LV_USE_100ASK_NES                           0
#endif
#if LV_USE_100ASK_NES
    /*platform*/
    #define LV_100ASK_NES_PLATFORM_POSIX                1
    #define LV_100ASK_NES_PLATFORM_FREERTOS             0
    #define LV_100ASK_NES_PLATFORM_RTTHREAD             0

    /*test*/
#if LV_USE_100ASK_FILE_EXPLORER
    #define LV_100ASK_NES_SIMPLE_TEST                   0
#endif
#endif

#endif /*LV_LIB_100ASK_H*/