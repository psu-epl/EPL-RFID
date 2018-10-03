#ifndef _RFID_NODE_
#define _RFID_NODE_

#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_types.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "driver/ledc.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "soc/rtc.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#include "soc/timer_group_struct.h"
#include "xtensa/hal.h"

#define AP_SSID "rfid_pi"
#define AP_PW "pimustdie"

#define CAP0_INT_EN           BIT(27)  //Capture 0 interrupt bit
#define GPIO_CAP0_IN          23   //Set GPIO 23 as  CAP0
#define CAP_SIG_NUM           1

#define LEDC_HS_TIMER         LEDC_TIMER_0
#define LEDC_HS_MODE          LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO      18
#define LEDC_HS_CH0_CHANNEL   LEDC_CHANNEL_0

#define LEDC_TEST_CH_NUM      1
#define LEDC_TEST_DUTY        4000
//#define LEDC_TEST_DUTY        2

//#define TIMER_DIVIDER         16  //  Hardware timer clock divider
//
// Setting this to 80 means when we divide the system clock by the 
// divider we should have 1us ticks to count (80x10^6Hz/80) = 1MHz -> 1us period
#define TIMER_DIVIDER         80  //  Hardware timer clock divider
// convert counter value to seconds
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)
#define TIMER_INTERVAL0_SEC   (3.0) // sample test interval for the first timer
#define NO_RELOAD   0        // testing will be done without auto reload
#define RELOAD      1        // testing will be done with auto reload

typedef struct {
  int type;  // the type of timer's event
  int timer_group;
  int timer_idx;
  uint64_t timer_counter_value;
}timer_event_t;

typedef struct {
  int timer_idx;
  bool auto_reload;
  double timer_interval_sec;
  xQueueHandle timer_queue;
}rfid_timer_t;

#define OFF    0
#define ON     1

typedef struct{
  int level;
  rfid_timer_t timer;
  xQueueHandle cap_queue;
}RFID_NODE;

typedef struct {
  //uint32_t capture_signal;
  uint64_t capture_signal;
  mcpwm_capture_signal_t sel_cap_signal;
} capture;

//xQueueHandle cap_queue;

//extern void timer_init(int timer_idx,bool auto_reload, double timer_interval_sec);
extern void rfid_timer_init(rfid_timer_t *timer);
extern void disp_captured_signal(void *arg);
extern void input_capture_config(void *arg);
extern void pwm_config(void *arg);
#endif
