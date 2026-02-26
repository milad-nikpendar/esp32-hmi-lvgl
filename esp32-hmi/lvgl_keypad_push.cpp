#include "lvgl_functions.h"

#include "../config.h"
#ifdef BuzzerPin
#include <initBuzzer.h>
// extern Buzzer beep;  // Buzzer object
extern RtosBuzzer beep; // Buzzer object
#endif                  // BuzzerPin

#ifdef USE_KEYPAD_READ

// Check if a specific key is pressed based on analog value
bool is_push(int value)
{
  int a = analogRead(Analog_KEY); // Read the analog value
  if (abs(a - value) < 50)
  {
    delay(10);                  // Debounce delay
    a = analogRead(Analog_KEY); // Read again
    return abs(a - value) < 50; // Confirm the key press
  }
  return false;
}

// Process keypad input
lv_key_t get_pressed_key()
{
  if (is_push(1083))
    return LV_KEY_NEXT;
  if (is_push(1612))
    return LV_KEY_PREV;
  if (is_push(2028))
    return LV_KEY_ENTER;
  if (is_push(2540))
    return LV_KEY_UP;
  if (is_push(2890))
    return LV_KEY_DOWN;
  return (lv_key_t)0;
}

/* initialize the keypad */
void hmi_keypad_init()
{
  pinMode(Analog_KEY, INPUT);
  pinMode(21, INPUT);
}

/* Read the touchpad */
void hmi_keypad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  static bool last_key_state = false;

  /* read the keypad */
  lv_key_t key = get_pressed_key();

  if ((uint8_t)key != 0)
  {
    data->state = LV_INDEV_STATE_PRESSED;
    data->key = key;

    if (!last_key_state)
    {
      last_key_state = true;
      beep.pulse(3200, 35); // 3200hz for 60ms
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED; // touch is released
    last_key_state = false;
  }
  data->continue_reading = false;
}
#endif // USE_KEYPAD_READ
