#include "lvgl_functions.h"

#ifdef USE_CAP_TOUCH

#include <Wire.h>
#include "I2Cscanner.h"
#include <initGT911.h>

// Touch Configuration
#define TCH_I2C_ADDR GT911_I2C_ADDR_28
#define TCH_MODE GT911_MODE_INTERRUPT // Use interrupt mode for better responsiveness
#define TCH_ROTATION (rotation_t) TFT_ROTATION
#define TCH_COUNT 5            // Max 5 touch points
#define TCH_DEBOUNCE_PRESS 2   // Press stable after 2 consecutive reads (10ms)
#define TCH_DEBOUNCE_RELEASE 6 // Release stable after 6 consecutive reads (30ms)

// #define Debug_GT911_Serial // Uncomment for Serial Debugging

// Touchscreen Object
initGT911 Touchscreen(&Wire, TCH_I2C_ADDR);

#ifdef Debug_GT911_Serial
#define TCH_Log(a) Serial.println("[GT911] " + String(a))
#else
#define TCH_Log(a)
#endif

// Shared Touch Data Structure
typedef struct
{
  uint8_t count;             // Number of touch points
  bool pressed;              // Touch state
  GTPoint points[TCH_COUNT]; // Max 5 touch points
} touch_data_t;

static touch_data_t touch_data = {0};                         // Shared touch data structure
static portMUX_TYPE touch_mux = portMUX_INITIALIZER_UNLOCKED; // Synchronization (ESP32)
static hmi_user_callback_t user_i2c_callback = nullptr;       // User Callback

// Touch Task
void i2c_touch_task(void *pv)
{
  const TickType_t delay = pdMS_TO_TICKS(5);

  uint8_t stable_press = 0;
  uint8_t stable_release = 0;

  bool pressed_state = false;

  while (true)
  {
    uint8_t t = Touchscreen.touched(TCH_MODE);

    if (t > 0)
    {
      GTPoint *tp = Touchscreen.getPoints();
      uint8_t count = (t > TCH_COUNT) ? TCH_COUNT : t;

      stable_press++;
      stable_release = 0;

      if (!pressed_state && stable_press >= TCH_DEBOUNCE_PRESS)
        pressed_state = true;

      if (pressed_state)
      {
        portENTER_CRITICAL(&touch_mux);

        touch_data.count = count;
        touch_data.pressed = true;

        memcpy(touch_data.points, tp, sizeof(GTPoint) * count);

        portEXIT_CRITICAL(&touch_mux);
      }
    }
    else
    {
      stable_release++;
      stable_press = 0;

      if (pressed_state && stable_release >= TCH_DEBOUNCE_RELEASE)
      {
        pressed_state = false;

        portENTER_CRITICAL(&touch_mux);

        touch_data.count = 0;
        touch_data.pressed = false;

        portEXIT_CRITICAL(&touch_mux);
      }
    }

    if (user_i2c_callback)
      user_i2c_callback();

    vTaskDelay(delay);
  }
}

// LVGL Touch Read
void hmi_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  GTPoint p;
  bool pressed;
  uint8_t count;

  portENTER_CRITICAL(&touch_mux);

  pressed = touch_data.pressed;
  count = touch_data.count;

  if (count > 0)
    p = touch_data.points[0];

  portEXIT_CRITICAL(&touch_mux);

  if (pressed && count > 0)
  {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = p.x;
    data->point.y = p.y;
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  data->continue_reading = false;
}

// Touch Initialization
void hmi_touch_init(hmi_user_callback_t cb)
{
  user_i2c_callback = cb;
  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  I2Cscanner(&Wire);

  Touchscreen.begin(TCH_IRQ, TCH_RST, 400000);
  Touchscreen.setupDisplay(TFT_HOR_RES, TFT_VER_RES, TCH_ROTATION);

  // Create dedicated touch task on Core 0
  xTaskCreatePinnedToCore(i2c_touch_task, "TouchTask", 4096, NULL, 2, NULL, 0);
}

#endif // USE_CAP_TOUCH
