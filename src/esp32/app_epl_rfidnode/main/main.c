/**
 * @brief Template for the EPL RFID project
 *
 *
 *
 */

#include "rfid_node.h"

RFID_NODE node = { 0 };

esp_err_t event_handler(void *ctx, system_event_t *event)
{
      switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, BIT0);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, BIT0);
        break;
    default:
        break;
    }
  return ESP_OK;
}

static void init_wifi()
{
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  wifi_config_t sta_config = {
    .sta = {
      .ssid = AP_SSID,
      .password = AP_PW,
      .bssid_set = false
    }
  };
  ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  ESP_ERROR_CHECK( esp_wifi_connect() );
}

static void init_pins(void)
{
  gpio_pad_select_gpio(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

static void app_init(void)
{
  ESP_ERROR_CHECK( nvs_flash_init() );
  init_pins();
  init_wifi();
  //init_mcpwm(&node);
}

void app_main(void)
{
  ESP_LOGI(TAG, "Starting the app main");

  app_init();
  node.cap_queue = xQueueCreate(1, sizeof(capture));

//*
  xTaskCreate(
		&blink_task,
		"blink_task",
		configMINIMAL_STACK_SIZE,
		&node,//NULL,
		5,
		NULL
	);
//*/
//*
  xTaskCreate(
		&input_capture_task,
		"input_capture",
		configMINIMAL_STACK_SIZE,
		&node,//NULL,
		5,
		NULL
	);

  xTaskCreate(&https_get_task,
   "https_get_task",
    8192,
    NULL,
    5,
    NULL
    );

//*/
/*
  xTaskCreate(
    &init,
    "pwm_task",
    2048,
    NULL,
    6,
    NULL
  );
//*/

}

