
#include "rfid_node.h"

RFID_NODE rfid_node = { };

void app_init(){
  input_capture_config(&rfid_node);
  pwm_config(&rfid_node);
  rfid_node.cap_queue = xQueueCreate(10, sizeof(capture)); 
  //rfid_node.timer.timer_queue = xQueueCreate(10, sizeof(timer_event_t));
  rfid_node.timer.timer_idx = TIMER_0;
  rfid_node.timer.auto_reload = RELOAD; 
  //rfid_node.timer.auto_reload = RELOAD; 
  rfid_node.timer.timer_interval_sec = TIMER_INTERVAL0_SEC;
  rfid_node.timer.timer_queue = xQueueCreate(10, sizeof(timer_event_t));
  rfid_timer_init(&rfid_node.timer);
  //timer_init(TIMER_0, TEST_WITH_RELOAD, TIMER_INTERVAL0_SEC);
}

void app_main()
{
  app_init();
  //* 
  xTaskCreate(
    disp_captured_signal, 
    "disp_captured_signal", 
    4096, 
    &rfid_node, 
    5, 
    NULL
  );
  //*/
  /*
  xTaskCreate(
    gpio_test_signal, 
    "gpio_test_signal", 
    4096, 
    NULL, 
    5, 
    NULL
  );
  //*/
}

