#include "rfid_node.h"

static mcpwm_dev_t *MCPWM[1] = {&MCPWM0};

static void mcpwm_example_gpio_initialize()
{
  mcpwm_pin_config_t pin_config = {
    .mcpwm0a_out_num = GPIO_PWM0A_OUT,
    .mcpwm_cap0_in_num   = GPIO_CAP0_IN,
  };
  mcpwm_set_pin(MCPWM_UNIT_0, &pin_config);
  gpio_pulldown_en(GPIO_CAP0_IN); 
}

/**
 * @brief Set gpio 12 as our test signal 
 */
extern void gpio_test_signal(void *arg)
{
  printf("intializing test signal...\n");
  gpio_config_t gp;
  gp.intr_type = GPIO_INTR_DISABLE;
  gp.mode = GPIO_MODE_OUTPUT;
  gp.pin_bit_mask = GPIO_SEL_12;
  gpio_config(&gp);
  while (1) {
    //period of test signal is 20ms
    gpio_set_level(GPIO_NUM_12, 1); //Set high
    vTaskDelay(pdMS_TO_TICKS(10));  //delay of 10ms
    gpio_set_level(GPIO_NUM_12, 0); //Set low
    vTaskDelay(pdMS_TO_TICKS(10));  //delay of 10ms
  }
}

/**
 * @brief display on interrupt
 */
extern void disp_captured_signal(void *arg)
{
  uint32_t *current_cap_value = (uint32_t *)malloc(sizeof(CAP_SIG_NUM));
  uint32_t *previous_cap_value = (uint32_t *)malloc(sizeof(CAP_SIG_NUM));
  capture evt;
  while (1) {
    xQueueReceive(((RFID_NODE *)arg)->cap_queue, &evt, portMAX_DELAY);
    if (evt.sel_cap_signal == MCPWM_SELECT_CAP0) {
      current_cap_value[0] = evt.capture_signal - previous_cap_value[0];
      previous_cap_value[0] = evt.capture_signal;
      current_cap_value[0] = 
        (current_cap_value[0] / 10000) * (10000000000 / rtc_clk_apb_freq_get());
      printf("CAP0 : %d us\n", current_cap_value[0]);
    }
  }
}

/**
 * @brief this is ISR handler function, 
 */
static void IRAM_ATTR isr_handler(void *arg)
{
  uint32_t mcpwm_intr_status;
  capture evt;
  mcpwm_intr_status = MCPWM[MCPWM_UNIT_0]->int_st.val; //Read interrupt status
  //Check for interrupt on rising edge on CAP0 signal
  if (mcpwm_intr_status & CAP0_INT_EN) {     
    evt.capture_signal = 
      mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0);     
    evt.sel_cap_signal = MCPWM_SELECT_CAP0;
    xQueueSendFromISR(((RFID_NODE *)arg)->cap_queue, &evt, NULL);
  }
  MCPWM[MCPWM_UNIT_0]->int_clr.val = mcpwm_intr_status;
}

/**
 * @brief Configure whole MCPWM module
 */
extern void mcpwm_example_config(void *arg)
{
  mcpwm_example_gpio_initialize();

  mcpwm_config_t pwm_config;
  pwm_config.frequency = 1000;    //frequency = 1000Hz
  pwm_config.cmpr_a = 50.0;       //duty cycle of PWMxA = 60.0%
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
  
  mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, MCPWM_POS_EDGE, 0);
  
  MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN;
  mcpwm_isr_register(MCPWM_UNIT_0, isr_handler, arg, ESP_INTR_FLAG_IRAM, NULL); 
//  vTaskDelete(NULL);
}
