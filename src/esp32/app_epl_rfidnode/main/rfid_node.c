#include "rfid_node.h"

static mcpwm_dev_t *MCPWM[1] = {&MCPWM0};

static void mcpwm_example_gpio_initialize()
{
  mcpwm_pin_config_t pin_config = {
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
extern void input_capture_config(void *arg)
{
  mcpwm_example_gpio_initialize();
  mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, MCPWM_POS_EDGE, 0);
  
  MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN;
  mcpwm_isr_register(MCPWM_UNIT_0, isr_handler, arg, ESP_INTR_FLAG_IRAM, NULL); 
}

extern void pwm_config(void *arg)
{
   int ch;

   ledc_timer_config_t ledc_timer = {
     .duty_resolution = LEDC_TIMER_2_BIT, // resolution of PWM duty
     .freq_hz = 125e3,                      // frequency of PWM signal
     .speed_mode = LEDC_HS_MODE,           // timer mode
     .timer_num = LEDC_HS_TIMER            // timer index
   };

   ledc_timer_config(&ledc_timer);

   ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
     {
       .channel    = LEDC_HS_CH0_CHANNEL,
       .duty       = LEDC_TEST_DUTY,
       .gpio_num   = LEDC_HS_CH0_GPIO,
       .speed_mode = LEDC_HS_MODE,
       .intr_type = LEDC_INTR_DISABLE,
       .timer_sel  = LEDC_HS_TIMER
     },
   };

   for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
     ledc_channel_config(&ledc_channel[ch]);
   }

   ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, LEDC_TEST_DUTY);
   ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
}
