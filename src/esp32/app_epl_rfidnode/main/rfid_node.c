#include "rfid_node.h"

static mcpwm_dev_t *MCPWM[1] = {&MCPWM0};

static void blink(int *level)
{
  gpio_set_level(BLINK_GPIO, *level);
  *level = !(*level);
}

extern void blink_task(void *vpRFID_NODE)
{
  int level = ((RFID_NODE *)vpRFID_NODE)->level;
  while(1) 
  {
    blink(&level);    
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY));
  }
}

static void mcpwm_gpio_initialize()
{
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
  
  mcpwm_pin_config_t pin_config = {
    .mcpwm0a_out_num = GPIO_PWM0A_OUT,
    .mcpwm_cap0_in_num = GPIO_CAP0_IN
 //   .mcpwm0b_out_num = GPIO_PWM0B_OUT//,
  };
  mcpwm_set_pin(MCPWM_UNIT_0, &pin_config);
  
  gpio_pulldown_en(GPIO_CAP0_IN);    //Enable pull down on CAP0   signal
}

static void IRAM_ATTR isr_handler(void *pvoid)
{
  uint32_t mcpwm_intr_status;
  capture evt;
  mcpwm_intr_status = MCPWM[MCPWM_UNIT_0]->int_st.val; //Read interrupt status
  //Check for interrupt on rising edge on CAP0 signal
  if (mcpwm_intr_status & CAP0_INT_EN) { 
    //get capture signal counter value  
    evt.capture_signal = 
        mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0); 
    evt.sel_cap_signal = MCPWM_SELECT_CAP0;
    xQueueSendFromISR(((RFID_NODE *)pvoid)->cap_queue, &evt, NULL);
  }
  MCPWM[MCPWM_UNIT_0]->int_clr.val = mcpwm_intr_status;
}

extern void init_mcpwm(RFID_NODE *pRFID_NODE)
{
  mcpwm_gpio_initialize();
  mcpwm_config_t pwm_config;
 // pwm_config.frequency = 125000;    
  pwm_config.frequency = 1000;    
  //pwm_config.frequency = 1000;    //frequency = 1000Hz
  pwm_config.cmpr_a = 20.0;    
//  pwm_config.cmpr_b = 50.0;       //duty cycle of PWMxb = 50.0%
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);  

  //capture signal on rising edge, prescale = 0 
  //i.e. 800,000,000 counts is equal to one second
  mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, MCPWM_POS_EDGE, 0);     
  
  //Enable interrupt on  CAP0, CAP1 and CAP2 signal
  MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN;// | CAP1_INT_EN | CAP2_INT_EN;
  
  //Set ISR Handler
  mcpwm_isr_register(MCPWM_UNIT_0, isr_handler,(void *)pRFID_NODE, ESP_INTR_FLAG_IRAM, NULL);
  
  //vTaskDelete(NULL);
}

extern void input_capture_task(void *pvoid)
{
  uint32_t *current_cap_value = (uint32_t *)malloc(sizeof(CAP_SIG_NUM));
  uint32_t *previous_cap_value = (uint32_t *)malloc(sizeof(CAP_SIG_NUM));
  capture evt;
  while (1) {
    xQueueReceive(((RFID_NODE *)pvoid)->cap_queue, &evt, portMAX_DELAY);
    if (evt.sel_cap_signal == MCPWM_SELECT_CAP0) {
      current_cap_value[0] = evt.capture_signal - previous_cap_value[0];
      previous_cap_value[0] = evt.capture_signal;
      current_cap_value[0] = 
        (current_cap_value[0]/10000)*(10000000000/rtc_clk_apb_freq_get());
//      printf("CAP0 : %d us\n", current_cap_value[0]);
    }
  }
}

