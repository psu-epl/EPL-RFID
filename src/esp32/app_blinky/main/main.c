/**
 * @brief this is a modified version of the stock blinky app which
 * @brief actually builds and flashes properly
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
//#include "sdkconfig.h"

#define BLINK_GPIO		10
#define BLINK_DELAY 	500 // ms if using pdMS_TO_TICKS 
#define OFF						0
#define ON						1

void blink_task(void *pvParameter)
{
  /**
   * Configure the IOMUX register for pad BLINK_GPIO (some pads are
   * muxed to GPIO on reset already, but some default to other
   * functions and need to be switched to GPIO. Consult the
   * Technical Reference for a list of pads and their default
   * functions.)
   */
  gpio_pad_select_gpio(BLINK_GPIO);
  
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  
  while(1) 
  {
    /// Blink off (output low) 
    gpio_set_level(BLINK_GPIO,OFF);
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY));
  
    /// Blink on (output high) 
    gpio_set_level(BLINK_GPIO,ON);
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY));
  }
}

void app_main()
{
  xTaskCreate(
		&blink_task, 
		"blink_task", 
		configMINIMAL_STACK_SIZE, 
		NULL, 
		5, 
		NULL
	);
}
