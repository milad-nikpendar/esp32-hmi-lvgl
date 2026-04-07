#include "lvgl_functions.h"

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define BUF_PIXELS (TFT_HOR_RES * TFT_VER_RES)
// #define BUF_PIXELS (TFT_HOR_RES * 20)
#define BUF_SIZE (BUF_PIXELS * sizeof(lv_color_t))
static lv_color_t *draw_buf1 = nullptr;
static lv_color_t *draw_buf2 = nullptr;

extern "C" QueueHandle_t tft_flush_queue;

static hmi_user_callback_t user_lvgl_callback = nullptr;

/* LV_USE_LOG must be enabled in lv_conf.h to use this function */
#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

#if LV_USE_TFT_ESPI == 0
/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  tft_flush_req_t req;             // Create a request
  req.disp = disp;                 // Set the display
  req.area = *area;                // Set the area to be flushed
  req.px_map = (uint16_t *)px_map; // Set the pixel map

  xQueueSend(tft_flush_queue, &req, portMAX_DELAY); // Send the request to the TFT task
}
#endif // LV_USE_TFT_ESPI

/* LVGL needs a tick source to know how much time passed. */
static uint32_t my_tick(void) { return millis(); }

/* initialize the LVGL functions */
void hmi_lvgl_init(hmi_user_callback_t cb)
{
  user_lvgl_callback = cb;
#ifdef USE_I18N
  translations_init();
#endif // USE_I18N
  lv_init();

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);

  /* register print function for debugging */
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

#ifdef USE_PSRAM
  if (psramFound()) // use PSRAM buffer
  {
    draw_buf1 = (lv_color_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    draw_buf2 = (lv_color_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (!draw_buf1) // if PSRAM allocation failed, use RAM buffer
    {
      draw_buf1 = (lv_color_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      draw_buf2 = (lv_color_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      Serial.println("Use RAM buffer");
    }

    else
      Serial.println("Use PSRAM buffer");
  }
  else // use RAM buffer
  {
    draw_buf1 = (lv_color_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    draw_buf2 = (lv_color_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    Serial.println("Use RAM buffer");
  }
#else  // use RAM buffer
  draw_buf1 = (lv_color_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  draw_buf2 = (lv_color_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#endif // USE_PSRAM

  if (!draw_buf1)
    Serial.println("ERROR: No buffer1 allocated!");
  if (!draw_buf2)
    Serial.println("ERROR: No buffer2 allocated!");

  lv_display_t *disp;
#if LV_USE_TFT_ESPI
  /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/

  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf1, BUF_SIZE);
  lv_display_set_buffers(disp, draw_buf1, (draw_buf2) ? draw_buf2 : NULL, BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_rotation(disp, TFT_ROTATION);

#else
  tft_flush_queue = xQueueCreate(2, sizeof(tft_flush_req_t));
  xTaskCreatePinnedToCore(tft_task, "TFT Task", 4096, NULL, 5, NULL, 1);

  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);                                                                // Initialize the lvgl display
  lv_display_set_flush_cb(disp, my_disp_flush);                                                                      // Set your driver function
  lv_display_set_buffers(disp, draw_buf1, (draw_buf2) ? draw_buf2 : NULL, BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL); // Set the display buffers
  lv_display_set_rotation(disp, TFT_ROTATION);                                                                       // Set the display rotation
#endif

#if defined(USE_CAP_TOUCH) || defined(USE_RES_TOUCH)
  /*Initialize the (dummy) input device driver*/
  lv_indev_t *indev_t = lv_indev_create();
  lv_indev_set_type(indev_t, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev_t, hmi_touch_read);
#endif

#if defined(USE_KEYPAD_READ)
  /* Initialize the keypad input device */
  lv_indev_t *indev_k = lv_indev_create();
  lv_indev_set_type(indev_k, LV_INDEV_TYPE_KEYPAD); /* Keypad should have KEYPAD type */
  lv_indev_set_read_cb(indev_k, hmi_keypad_read);
  /* Create and assign a group for keypad navigation */
  lv_group_t *group = lv_group_create();
  lv_indev_set_group(indev_k, group);
  lv_group_set_default(group); // Optional: make it the default group
#endif

  lv_timer_set_period(lv_display_get_refr_timer(disp), 1);

  /*Initialize the display*/
  ui_init();
  xTaskCreatePinnedToCore(lvgl_task, "LVGL Task", 8192, NULL, 3, NULL, 1);

#ifdef TFT_LED // Use Backlight control if defined
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH); // Turn on the backlight
#endif                         // TFT_LED
}

/* Call this function in your loop() to let LVGL do its work*/
/*
void hmi_lvgl_handler()
{
  lv_timer_handler(); // Call the LVGL timer handler
  ui_tick();          // Call the gui tick function
  yield();            // Yield to allow other tasks to run
}
*/

uint32_t flush_fps = 0;
static uint64_t flush_last = 0;
static uint32_t instant_fps = 0;
static uint32_t timer_fps = 0;

void lvgl_task(void *pvParameters)
{
  const TickType_t delay = 1; // 1ms delay for stable LVGL timing and CPU balance

  while (true)
  {
    lv_timer_handler(); // Main LVGL engine: rendering, animations, input handling
    ui_tick();          // UI engine generated by EEZ Studio: events, animations, state updates

    if (user_lvgl_callback)
      user_lvgl_callback();

    uint64_t now = esp_timer_get_time(); // High precision timer (µs)
    uint64_t dt = now - flush_last;      // Time between two flushes in µs
    flush_last = now;

    if (dt > 0)
    {
      instant_fps = 1000000 / dt; // FPS = 1e6 / dt
    }

    if (millis() - timer_fps > 20)
    {
      flush_fps = instant_fps;
      timer_fps = millis();
    }

    vTaskDelay(delay); // Yield to FreeRTOS to avoid blocking other tasks
  }
}
