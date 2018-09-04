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
#include "esp_log.h"


#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"




#define AP_SSID "rfid_pi" 
#define AP_PW "pimustdie" 

#define BLINK_GPIO            10
#define BLINK_DELAY           1000 // ms if using pdMS_TO_TICKS 
#define INPUT_CAPTURE_DELAY   100 // ms if using pdMS_TO_TICKS 

#define CAP0_INT_EN BIT(27)  //Capture 0 interrupt bit

#define GPIO_PWM0A_OUT 19   //Set GPIO 19 as PWM0A
#define GPIO_PWM0B_OUT 18   //Set GPIO 18 as PWM0B
#define GPIO_CAP0_IN   23   //Set GPIO 23 as  CAP0
#define CAP_SIG_NUM     1

#define OFF    0
#define ON     1


/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "localhost"
#define WEB_SERVER_IP "192.168.0.6"
#define WEB_PORT "3001"
#define WEB_URL "https://192.168.0.6:3001/api/user-access"

static const char *TAG = "example";

static const char *REQUEST = "POST " WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: esp-idf 1.0 esp32\r\n"
    "station-id: node4\r\n"
    "station-state: Enabled\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "a\r\n";
extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");


extern const uint8_t key_start[] asm("_binary_localhost_key_start");
extern const uint8_t key_end[]   asm("_binary_localhost_key_end");


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
extern void init_mcpwm(RFID_NODE *pRFID_NODE);
extern void https_get_task(void *vpRFID_NODE);

#endif
