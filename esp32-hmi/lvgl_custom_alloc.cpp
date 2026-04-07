#include <Arduino.h>
#include "lvgl_functions.h"

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM
#include "esp_heap_caps.h"

#ifdef USE_PSRAM
#define MALLOC_CAPS (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) // uncomment this line to use PSRAM (if available)
#else
#define MALLOC_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) // uncomment this line to use internal RAM
#endif

// LVGL initialization memory functions for ESP32
void lv_mem_init(void)
{
#ifdef USE_PSRAM
    psramFound();
#else
// No initialization needed for ESP32 heap
#endif // USE_PSRAM
}

// LVGL heap monitor function for ESP32
void lv_mem_monitor_core(lv_mem_monitor_t *mon_p)
{
    size_t total_free = heap_caps_get_free_size(MALLOC_CAPS);
    size_t largest_free = heap_caps_get_largest_free_block(MALLOC_CAPS);
    size_t total_alloc = heap_caps_get_total_size(MALLOC_CAPS) - total_free;

    mon_p->total_size = total_free + total_alloc;
    mon_p->free_cnt = 0; // Not available in ESP32 heap
    mon_p->free_size = total_free;
    mon_p->free_biggest_size = largest_free;
    mon_p->used_cnt = 0; // Not available in ESP32 heap
    mon_p->max_used = 0; // Not available in ESP32 heap
    mon_p->used_pct = (mon_p->total_size > 0) ? (total_alloc * 100 / mon_p->total_size) : 0;
    mon_p->frag_pct = (mon_p->free_size > 0) ? ((mon_p->free_size - mon_p->free_biggest_size) * 100 / mon_p->free_size) : 0;
}

void *lv_malloc_core(size_t size) { return heap_caps_malloc(size, MALLOC_CAPS); }                           // LVGL memory allocation functions for ESP32
void lv_free_core(void *ptr) { heap_caps_free(ptr); }                                                       // LVGL memory reallocation and zero-initialized allocation functions for ESP32
void *lv_realloc_core(void *ptr, size_t new_size) { return heap_caps_realloc(ptr, new_size, MALLOC_CAPS); } // LVGL memory allocation functions for ESP32
void *lv_calloc_core(size_t n, size_t size) { return heap_caps_calloc(n, size, MALLOC_CAPS); }              // LVGL memory allocation functions for ESP32

#endif // LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM
