#include "lvgl_functions.h"

#ifdef USE_RES_TOUCH

// SPI & Touch Libraries
#if LV_USE_TFT_ESPI
#include <SPI.h>
SPIClass *hmiSpi = NULL;
#else
extern "C" TFT_eSPI tft;
#endif

#include "XPT2046_Touchscreen.h"

// Configuration
#define TCH_ROTATION (uint8_t)TFT_ROTATION
#define TCH_DEBOUNCE_PRESS 2
#define TCH_DEBOUNCE_RELEASE 4
//#define Debug_XPT2046_Serial

#define TCH_SAMPLES 5
#define TCH_IIR_ALPHA 4 // bigger = smoother

// Touch Object
XPT2046_Touchscreen Touchscreen(TCH_CS, TCH_IRQ);

#ifdef Debug_XPT2046_Serial
#define TCH_Log(a) Serial.println("[XPT2046] " + String(a))
#define TCH_Logf(a, ...) Serial.printf("[XPT2046] " a "\n", ##__VA_ARGS__)
#else
#define TCH_Log(a)
#define TCH_Logf(a, ...)
#endif

// Shared Touch Data
typedef struct
{
  bool pressed;
  TS_Point point;

} touch_data_t;

static touch_data_t touch_data = {false, TS_Point()};
static portMUX_TYPE touch_mux = portMUX_INITIALIZER_UNLOCKED;
static hmi_user_callback_t user_touch_callback = nullptr;

// Median helper
static int median5(int *v)
{
  for (int i = 0; i < 4; i++)
    for (int j = i + 1; j < 5; j++)
      if (v[j] < v[i])
      {
        int t = v[i];
        v[i] = v[j];
        v[j] = t;
      }

  return v[2];
}

// Read filtered point
static TS_Point read_filtered_point()
{
  int xs[TCH_SAMPLES];
  int ys[TCH_SAMPLES];

  for (int i = 0; i < TCH_SAMPLES; i++)
  {
    TS_Point p = Touchscreen.getPoint();
    xs[i] = p.x;
    ys[i] = p.y;
  }

  TS_Point out;
  out.x = median5(xs);
  out.y = median5(ys);

  return out;
}

// Touch Task (called from TFT task)
void spi_touch_task()
{
  static uint8_t stable_press = 0;
  static uint8_t stable_release = 0;
  static bool pressed_state = false;

  static int filtered_x = 0;
  static int filtered_y = 0;

  if (Touchscreen.touched())
  {
    TS_Point p = read_filtered_point();

    stable_press++;
    stable_release = 0;

    if (!pressed_state && stable_press >= TCH_DEBOUNCE_PRESS)
    {
      pressed_state = true;
      filtered_x = p.x;
      filtered_y = p.y;
    }

    if (pressed_state)
    {
      // IIR filter (low-pass smoothing)
      filtered_x = (filtered_x * (TCH_IIR_ALPHA - 1) + p.x) / TCH_IIR_ALPHA;
      filtered_y = (filtered_y * (TCH_IIR_ALPHA - 1) + p.y) / TCH_IIR_ALPHA;
      TCH_Logf("X: %d, Y: %d", filtered_x, filtered_y);

      portENTER_CRITICAL(&touch_mux);

      touch_data.pressed = true;
      touch_data.point.x = filtered_x;
      touch_data.point.y = filtered_y;

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
      touch_data.pressed = false;
      portEXIT_CRITICAL(&touch_mux);
    }
  }

  if (user_touch_callback)
    user_touch_callback();
}

// Touch Initialization
void hmi_touch_init(hmi_user_callback_t cb)
{
  user_touch_callback = cb;

#if !LV_USE_TFT_ESPI
  Touchscreen.begin(tft.getSPIinstance());
#else

  hmiSpi = new SPIClass(VSPI);
  hmiSpi->begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TCH_CS);

  Touchscreen.begin(*hmiSpi);

#endif

  if (TCH_ROTATION == LV_DISPLAY_ROTATION_0 || TCH_ROTATION == LV_DISPLAY_ROTATION_180)
    Touchscreen.setRotation(1);
  else
    Touchscreen.setRotation(3);

#ifdef INIT_TFT_RTOS
  user_tft_callback = spi_touch_task;
#endif
}

// LVGL Input Read
void hmi_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{

#ifndef INIT_TFT_RTOS
  spi_touch_task();
#endif

  bool pressed;
  TS_Point p;

  portENTER_CRITICAL(&touch_mux);

  pressed = touch_data.pressed;
  p = touch_data.point;

  portEXIT_CRITICAL(&touch_mux);

  if (pressed)
  {
    data->state = LV_INDEV_STATE_PRESSED;

#if defined(ESP32_HMI_28TFT) || defined(ESP32_HMI_35TFT)

    data->point.y = map(p.x, 235, 3900, TFT_VER_RES, 0);
    data->point.x = map(p.y, 355, 3800, 0, TFT_HOR_RES);
#endif
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  data->continue_reading = false;
}

#endif // USE_RES_TOUCH
