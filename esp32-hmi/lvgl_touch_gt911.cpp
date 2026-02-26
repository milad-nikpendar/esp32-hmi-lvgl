#include "lvgl_functions.h"

#ifdef USE_CAP_TOUCH

/* initlialize the capitive touchpad */
#include <Wire.h>
#include "I2Cscanner.h"
#include <initGT911.h>

// ---- Configuration ----
#define TCH_I2C_ADDR GT911_I2C_ADDR_28
#define TCH_MODE GT911_MODE_INTERRUPT
#define TCH_ROTATION (rotation_t) TFT_ROTATION
// #define Debug_GT911_Serial

initGT911 Touchscreen(&Wire, TCH_I2C_ADDR);

#ifdef Debug_GT911_Serial
#define TCH_Log(a) Serial.println("[GT911] " + String(a))
#else
#define TCH_Log(a)
#endif

/* ------------------------------
   Shared Multi-Touch Data
--------------------------------*/
typedef struct
{
  uint8_t count;     // number of touch points
  GTPoint points[5]; // up to 5 points
} touch_data_t;

static touch_data_t touch_data = {0};

/* ------------------------------
   User Callback Pointer
--------------------------------*/
static hmi_user_callback_t user_i2c_callback = nullptr;

/* ------------------------------
   I2C + Touch Task
--------------------------------*/
void i2c_touch_task(void *pv)
{
  const TickType_t delay = pdMS_TO_TICKS(5);

  static uint8_t stable_press = 0;
  static uint8_t stable_release = 0;
  static bool is_pressed = false;

  while (true)
  {
    uint8_t t = Touchscreen.touched(TCH_MODE);

    if (t > 0)
    {
      // Always update coordinates when touch exists
      uint8_t count = t > 5 ? 5 : t;
      GTPoint *tp = Touchscreen.getPoints();

      touch_data.count = count;
      for (uint8_t i = 0; i < count; i++)
        touch_data.points[i] = tp[i];

      // Debounce only for PRESS state
      stable_press++;
      stable_release = 0;

      if (!is_pressed && stable_press >= 2)
        is_pressed = true;
    }
    else
    {
      stable_release++;
      stable_press = 0;

      if (is_pressed && stable_release >= 2)
      {
        is_pressed = false;
        touch_data.count = 0;
      }
    }

    if (user_i2c_callback)
      user_i2c_callback();

    vTaskDelay(delay);
  }
}

/* ------------------------------
   LVGL Touch Read (No I2C)
--------------------------------*/
void hmi_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (touch_data.count > 0)
  {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touch_data.points[0].x;
    data->point.y = touch_data.points[0].y;

    if (touch_data.count > 1)
    {
      TCH_Log("Multi touch found!");
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  data->continue_reading = false;
}

/* ------------------------------
   Touch Init
--------------------------------*/
void hmi_touch_init(hmi_user_callback_t cb)
{
  user_i2c_callback = cb;

  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  I2Cscanner(&Wire);

  Touchscreen.begin(TCH_IRQ, TCH_RST, 400000);
  Touchscreen.setupDisplay(TFT_HOR_RES, TFT_VER_RES, TCH_ROTATION);

  // Create I2C + Touch Task
  xTaskCreatePinnedToCore(i2c_touch_task, "I2C Touch Task", 4096, NULL, 4, NULL, 0);
}

#endif // USE_CAP_TOUCH
