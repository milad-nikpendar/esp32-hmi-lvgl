#include "lvgl_functions.h"

#ifdef INIT_TFT_RTOS
TFT_eSPI tft = TFT_eSPI(TFT_HOR_RES, TFT_VER_RES);
QueueHandle_t tft_flush_queue;
hmi_user_callback_t user_tft_callback = nullptr;

void tft_task(void *pvParameters)
{
    const TickType_t delay = pdMS_TO_TICKS(5);
    tft_flush_req_t req;

    tft.begin();                            // Initialize TFT
    tft.setRotation((uint8_t)TFT_ROTATION); // Set the display rotation
    tft.setSwapBytes(true);                 // Swap the byte order when pushing pixels
    tft.fillScreen(TFT_BLACK);              // Clear the screen

#ifdef USE_DMA
    tft.initDMA(true); // Initialize DMA, set to true to use double buffering
#endif                 // USE_DMA

    for (;;)
    {
        if (xQueueReceive(tft_flush_queue, &req, portMAX_DELAY) == pdTRUE)
        {
            uint32_t w = req.area.x2 - req.area.x1 + 1;
            uint32_t h = req.area.y2 - req.area.y1 + 1;

            tft.startWrite();
            tft.setAddrWindow(req.area.x1, req.area.y1, w, h);
#ifdef USE_DMA
            tft.pushPixelsDMA(req.px_map, w * h);
            tft.dmaWait();
#else
            tft.pushPixels(req.px_map, w * h);
#endif
            tft.endWrite();

            lv_disp_flush_ready(req.disp);
        }

        if (user_tft_callback)
            user_tft_callback();

        vTaskDelay(delay);
    }
}
#endif // INIT_TFT_RTOS
