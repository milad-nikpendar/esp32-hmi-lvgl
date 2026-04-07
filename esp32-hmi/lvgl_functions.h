/*
   hmi_lvgl_functions header V1.0.0
   Created by Milad Nikpendar
*/
#ifndef LVGL_FUNCTIONS_H
#define LVGL_FUNCTIONS_H

/* Set your display type */
// #define ESP32_HMI_24TFT // uncomment this line if you are using ESP32 with 2.4" TFT
// #define ESP32_HMI_28TFT // uncomment this line if you are using ESP32 with 2.8" TFT
#define ESP32_HMI_35TFT // uncomment this line if you are using ESP32 with 3.5" TFT
// #define ESP32_HMI_43TFT // uncomment this line if you are using ESP32 with 4.3" TFT

/* choose your touchpad type */
// #define USE_RES_TOUCH // uncomment this line to use resistive touchpad
#define USE_CAP_TOUCH // uncomment this line to use capacitive touchpad
// #define USE_KEYPAD_READ // uncomment this line to use keypad

#define USE_PSRAM // uncomment this line to use PSRAM (if available)
// #define USE_DMA   // uncomment this line to use SPI DMA (faster, but more memory usage)
// #define USE_I18N  // uncomment this line to use internationalization (i18n) features

/* Set to your screen rotation */
/*
  LV_DISPLAY_ROTATION_0
  LV_DISPLAY_ROTATION_90
  LV_DISPLAY_ROTATION_180
  LV_DISPLAY_ROTATION_270
*/
#define TFT_ROTATION LV_DISPLAY_ROTATION_270

#include <Arduino.h>

/* lvgl library */
#include <lvgl.h>          // main lvgl header file
#include "../ui/ui.h"      // include the generated UI header file
#include "../ui/vars.h"    // for use native variables you must create vars.cpp or vars.c file and add your variables there
#include "../ui/actions.h" // for use native actions you must create actions.cpp or actions.c file and add your actions there
#include "../ui/screens.h" // for use screen objects in your code you can declare them in screens.h and define them in screens.cpp

/*Set to your screen resolution*/
#if defined(ESP32_HMI_28TFT) || defined(ESP32_HMI_24TFT)
#define TFT_HOR_RES 240
#define TFT_VER_RES 320
#elif defined(ESP32_HMI_35TFT) || defined(ESP32_HMI_43TFT)
#define TFT_HOR_RES 320
#define TFT_VER_RES 480
#endif

#ifndef CONFIG_SPIRAM_SUPPORT
#define CONFIG_SPIRAM_SUPPORT
#endif // CONFIG_SPIRAM_SUPPORT
#include "esp_heap_caps.h"

/* TFT_eSPI library */
#if !LV_USE_TFT_ESPI
#include <TFT_eSPI.h>

typedef struct
{
  lv_display_t *disp;
  lv_area_t area;
  uint16_t *px_map;
} tft_flush_req_t;

#define INIT_TFT_RTOS // Use RTOS TFT task
// #define INIT_PARALLEL_RTOS // Use RTOS Parallel task
// #define TFT_PARALLEL_8_BIT // 8 bit parallel interface
// #define ESP32_PARALLEL     // ESP32 parallel interface
#endif

#include "../config.h"

typedef void (*hmi_user_callback_t)(void);

extern uint32_t flush_fps; // FPS calculated from flushes, updated every 20ms

/* lv_i18n functions */
#ifdef USE_I18N
extern userLanguage language;          // Global variable to hold the current language setting
void translations_init();       // Initialize translations
void translations_set_locale(); // Apply the current language setting to the UI
#endif

#ifdef INIT_TFT_RTOS
extern hmi_user_callback_t user_tft_callback; // User callback for TFT task
void tft_task(void *pvParameters);            // TFT task for RTOS
#endif

void lvgl_task(void *pvParameters); // LVGL task for RTOS

void hmi_touch_init(hmi_user_callback_t cb = nullptr);         // touch init
void hmi_touch_read(lv_indev_t *indev, lv_indev_data_t *data); // touch read function

void hmi_keypad_init();                                         // keypad init
void hmi_keypad_read(lv_indev_t *indev, lv_indev_data_t *data); // keypad read function

// void hmi_lvgl_handler(); // LVGL handler
void hmi_lvgl_init(hmi_user_callback_t cb = nullptr); // LVGL init

#endif // LVGL_FUNCTIONS_H
