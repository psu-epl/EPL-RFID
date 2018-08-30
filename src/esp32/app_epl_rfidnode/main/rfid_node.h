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
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "driver/ledc.h"
#include "soc/rtc.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#define AP_SSID "rfid_pi"
#define AP_PW "pimustdie"

#define BLINK_GPIO            10
#define BLINK_DELAY           1000 // ms if using pdMS_TO_TICKS 
#define INPUT_CAPTURE_DELAY   100 // ms if using pdMS_TO_TICKS 

#define CAP0_INT_EN           BIT(27)  //Capture 0 interrupt bit

#define GPIO_CAP0_IN          23   //Set GPIO 23 as  CAP0
#define CAP_SIG_NUM           1

#define LEDC_HS_TIMER         LEDC_TIMER_0
#define LEDC_HS_MODE          LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO      18
#define LEDC_HS_CH0_CHANNEL   LEDC_CHANNEL_0

#define LEDC_TEST_CH_NUM      1
#define LEDC_TEST_DUTY        2

#define OFF    0
#define ON     1

typedef struct{
  int level; 
  xQueueHandle cap_queue;
}RFID_NODE;

typedef struct {
  uint32_t capture_signal;
  mcpwm_capture_signal_t sel_cap_signal;
} capture;

//xQueueHandle cap_queue;

extern void gpio_test_signal(void *arg);
extern void disp_captured_signal(void *arg);
extern void input_capture_config(void *arg);
extern void pwm_config(void *arg);
#endif
