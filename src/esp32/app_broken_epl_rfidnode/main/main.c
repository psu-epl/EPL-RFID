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
  return ESP_OK;
}

static void init_wifi()
{
  tcpip_adapter_init();
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
  //nvs_flash_init();
 // init_pins();
 // init_wifi();
//  init_mcpwm(&node);
}

void app_main(void)
{
  app_init();
  node.cap_queue = xQueueCreate(1, sizeof(capture)); 

/*
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
		//&input_capture_task, 
    &disp_captured_signal,
		"disp_captured_signal", 
		configMINIMAL_STACK_SIZE, 
		&node,//NULL, 
		5, 
		NULL
	);
//*/
//*
  xTaskCreate(
    gpio_test_signal, 
    "gpio_test_signal", 
    configMINIMAL_STACK_SIZE, 
    &node, 
    5, 
    NULL
  );
//*/
//*
  xTaskCreate(
    mcpwm_config, 
    "mcpwm_config", 
    configMINIMAL_STACK_SIZE, 
    &node, 
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

