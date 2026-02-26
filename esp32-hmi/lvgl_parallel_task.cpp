#include "lvgl_functions.h"

#ifdef INIT_PARALLEL_RTOS

#include <LovyanGFX.hpp>

// ---------------------------
// 1) Custom LGFX class (Panel + Bus as members)
// ---------------------------
class LGFX : public lgfx::LGFX_Device
{
public:
    LGFX(void)
    {

        // ---------------------------
        // Bus config (Parallel 8-bit, LCD_CAM)
        // ---------------------------
        {
            auto cfg = _bus_instance.config(); // get modifiable config

            cfg.port = 0;              // LCD_CAM port
            cfg.freq_write = 40000000; // write freq
            cfg.freq_read = 8000000;   // read freq

            cfg.pin_wr = TFT_WR; // WR pin
            cfg.pin_rd = TFT_RD; // RD pin
            cfg.pin_rs = TFT_DC; // DC pin

            cfg.pin_d0 = TFT_D0; // D0
            cfg.pin_d1 = TFT_D1; // D1
            cfg.pin_d2 = TFT_D2; // D2
            cfg.pin_d3 = TFT_D3; // D3
            cfg.pin_d4 = TFT_D4; // D4
            cfg.pin_d5 = TFT_D5; // D5
            cfg.pin_d6 = TFT_D6; // D6
            cfg.pin_d7 = TFT_D7; // D7

            _bus_instance.config(cfg); // apply config
        }

        // ---------------------------
        // Panel config (ST7796)
        // ---------------------------
        {
            auto cfg = _panel_instance.config(); // get modifiable config

            cfg.pin_cs = TFT_CS;   // CS pin
            cfg.pin_rst = TFT_RST; // RST pin
            cfg.pin_busy = -1;     // no busy pin

            cfg.memory_width = TFT_HOR_RES;  // memory width
            cfg.memory_height = TFT_VER_RES; // memory height
            cfg.panel_width = TFT_HOR_RES;   // panel width
            cfg.panel_height = TFT_VER_RES;  // panel height

            cfg.offset_x = 0; // offset x
            cfg.offset_y = 0; // offset y

            cfg.invert = true;      // no invert
            cfg.readable = false;   // ST7796 read disabled
            cfg.rgb_order = false;  // RGB order
            cfg.dlen_16bit = false; // 8-bit bus
            cfg.bus_shared = false; // not shared

            _panel_instance.config(cfg); // apply config
        }

        // ---------------------------
        // Wire bus to panel, panel to LGFX_Device
        // ---------------------------
        _panel_instance.setBus(&_bus_instance); // attach bus to panel
        setPanel(&_panel_instance);             // attach panel to LGFX_Device
    }

private:
    lgfx::Bus_Parallel8 _bus_instance;  // bus member
    lgfx::Panel_ST7796 _panel_instance; // panel member
};

static LGFX lcd; // global display object

// ---------------------------
// 2) LVGL flush queue
// ---------------------------
QueueHandle_t tft_flush_queue;

// ---------------------------
// 3) TFT task (RTOS)
// ---------------------------
void tft_task(void *pvParameters)
{
    tft_flush_req_t req;

    lcd.init();                    // init display
    lcd.setRotation(TFT_ROTATION); // set rotation

    lcd.setColorDepth(lgfx::rgb565_2Byte);
    lcd.setSwapBytes(true);

    lcd.initDMA();    // enable DMA on bus
    lcd.startWrite(); // keep bus locked for fast transfers

    lcd.fillScreen(TFT_BLACK); // clear screen

    for (;;)
    {
        if (xQueueReceive(tft_flush_queue, &req, portMAX_DELAY) == pdTRUE)
        {
            uint32_t w = req.area.x2 - req.area.x1 + 1; // width
            uint32_t h = req.area.y2 - req.area.y1 + 1; // height

            // DMA-capable transfer via LovyanGFX bus
            lcd.pushImage(
                req.area.x1,                 // x
                req.area.y1,                 // y
                w,                           // width
                h,                           // height
                (const uint16_t *)req.px_map // pixel buffer
            );

            // ensure DMA finished before telling LVGL we're done
            lcd.waitDMA();

            lv_disp_flush_ready(req.disp); // notify LVGL
        }
    }
    lcd.endWrite();
}

#endif // INIT_PARALLEL_RTOS
