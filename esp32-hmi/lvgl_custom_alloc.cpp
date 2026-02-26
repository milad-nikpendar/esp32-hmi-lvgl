#include <Arduino.h>
#include "lvgl_functions.h"

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM
#include "esp_heap_caps.h"

#ifdef USE_PSRAM
#define MALLOC_CAPS (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) // uncomment this line to use PSRAM (if available)
#else
#define MALLOC_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) // uncomment this line to use internal RAM
#endif //USE_PSRAM

void lv_mem_init(void) {

}

void lv_mem_monitor_core(lv_mem_monitor_t * mon_p){

}

void *lv_malloc_core(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAPS);
}

void lv_free_core(void *ptr) {
    heap_caps_free(ptr);
}

void *lv_realloc_core(void *ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAPS);
}

void *lv_calloc_core(size_t n, size_t size) {
    return heap_caps_calloc(n, size, MALLOC_CAPS);
}
#endif
