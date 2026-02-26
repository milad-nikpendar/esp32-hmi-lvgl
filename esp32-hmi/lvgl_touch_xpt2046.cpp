#include "lvgl_functions.h"

#ifdef USE_RES_TOUCH

/* initialize the resistive touchpad */
#if LV_USE_TFT_ESPI
#include <SPI.h>
SPIClass *hmiSpi = NULL; // SPI object
#else
extern "C" TFT_eSPI tft;
#endif
#include "XPT2046_Touchscreen.h"

#define TCH_ROTATION (uint8_t)TFT_ROTATION
#define Debug_XPT2046_Serial

XPT2046_Touchscreen Touchscreen(TCH_CS, TCH_IRQ); // CS pin and IRQ pin
TS_Point rawLocation;                             // raw location of touch

#ifdef Debug_XPT2046_Serial
#define TCH_Log(a) Serial.println("[XPT2046] " + String(a))
#else
#define TCH_Log(a)
#endif // Debug_XPT2046_Serial

/* initialize the touchpad */
void hmi_touch_init()
{
/* initialize the resistive touchpad */
#if !LV_USE_TFT_ESPI
  Touchscreen.begin(tft.getSPIinstance()); // initialize the touchpad with the same SPI settings as TFT
#else
  hmiSpi = new SPIClass(VSPI);
  hmiSpi->begin(TFT_SCLK, TFT_MISO, TFT_MOSI, -1);
  Touchscreen.begin(*hmiSpi);
#endif
  if (TCH_ROTATION == LV_DISPLAY_ROTATION_0 || TCH_ROTATION == LV_DISPLAY_ROTATION_180)
    Touchscreen.setRotation(1);
  else
    Touchscreen.setRotation(3);
}

/*Read the touchpad*/
void hmi_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  /* read the resistive touchpad */
  if (Touchscreen.touched())
  {                                       // check if touch is pressed
    data->state = LV_INDEV_STATE_PRESSED; // touch is pressed

    rawLocation = Touchscreen.getPoint(); // get the raw location of touch
#ifdef ESP32_HMI_28TFT
    data->point.y = map(rawLocation.x, 235, 3900, 0, TFT_VER_RES); // map the x-axis to the screen resolution
    data->point.x = map(rawLocation.y, 355, 3800, 0, TFT_HOR_RES); // map the y-axis to the screen resolution
#endif                                                             // ESP32_HMI_28TFT

#ifdef ESP32_HMI_35TFT
    data->point.y = map(rawLocation.x, 235, 3900, 0, TFT_VER_RES); // map the x-axis to the screen resolution
    data->point.x = map(rawLocation.y, 355, 3800, 0, TFT_HOR_RES); // map the y-axis to the screen resolution
#endif                                                             // ESP32_HMI_35TFT

    TCH_Log(String(data->point.x) + "\t" + String(data->point.y));
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED; // touch is released
  }
  data->continue_reading = false;
}

#endif // USE_RES_TOUCH
