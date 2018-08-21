
#include "rfid_node.h"

RFID_NODE rfid_node = { 0 };

void app_init(){
  mcpwm_example_config(&rfid_node);
}

void app_main()
{
  app_init();

  rfid_node.cap_queue = xQueueCreate(1, sizeof(capture)); 
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
  //*
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

