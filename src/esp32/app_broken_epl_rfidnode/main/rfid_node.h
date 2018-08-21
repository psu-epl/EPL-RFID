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
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#define AP_SSID "rfid_pi"
#define AP_PW "pimustdie"

#define BLINK_GPIO            10
#define BLINK_DELAY           1000 // ms if using pdMS_TO_TICKS 
#define INPUT_CAPTURE_DELAY   100 // ms if using pdMS_TO_TICKS 

#define CAP0_INT_EN BIT(27)  //Capture 0 interrupt bit

#define GPIO_PWM0A_OUT 19   //Set GPIO 19 as PWM0A
#define GPIO_CAP0_IN   23   //Set GPIO 23 as  CAP0
#define CAP_SIG_NUM     1

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

extern void blink_task(void *vpRFID_NODE);
extern void input_capture_task(void *vpRFID_NODE);
extern void disp_captured_signal(void *arg);
extern void mcpwm_config(RFID_NODE *pRFID_NODE);
extern void gpio_test_signal(void *arg);

#endif
