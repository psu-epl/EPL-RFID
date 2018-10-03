#include "rfid_node.h"
#include <inttypes.h>

//static mcpwm_dev_t *MCPWM[1] = {&MCPWM0};
static mcpwm_dev_t *pMCPWM = &MCPWM0;

static void mcpwm_gpio_initialize()
{
  mcpwm_pin_config_t pin_config = {
    .mcpwm_cap0_in_num = GPIO_CAP0_IN,
  };
  mcpwm_set_pin(MCPWM_UNIT_0, &pin_config);
  gpio_pulldown_en(GPIO_CAP0_IN); 
}

/**
 * @brief display on interrupt
 */
extern void disp_captured_signal(void *arg)
{
  //uint32_t current_cap_value = 0;
  //uint32_t previous_cap_value = 0;
  uint64_t current_cap_value = 0;
  uint64_t previous_cap_value = 0;
  capture evt;
  while (1) {
    xQueueReceive(((RFID_NODE *)arg)->cap_queue, &evt, portMAX_DELAY);
    if (evt.sel_cap_signal == MCPWM_SELECT_CAP0) {
      current_cap_value = evt.capture_signal - previous_cap_value;
      previous_cap_value = evt.capture_signal;
      // something is likely fucked here!!!!!!!!!!!!!!!!!!!!!!!!!!!
      current_cap_value = 
        // so if system ticks are 1ms WTF!!!! then rtc_clk_apb_freq_get()
        // is garbage because it is based off system ticks and we are 
        // actually working directly with the hardware. there may also be 
        // issues using uint64_t because the MCU is cool with this but
        // FreeRTOS maybe no
        // also i may have had too many beers
        (current_cap_value / 10000) * (10000000000 / rtc_clk_apb_freq_get());
      printf("CAP0 : %" PRIu64 " us \n", current_cap_value);
    }
  }
}

/**
 * @brief this is ISR handler function, 
 */
static void IRAM_ATTR isr_handler(void *arg)
{
  uint32_t mcpwm_intr_status;
//  uint64_t timer_value = 0;
  capture evt;
  mcpwm_intr_status = pMCPWM->int_st.val; //Read interrupt status
  //Check for interrupt on rising edge on CAP0 signal
  if (mcpwm_intr_status & CAP0_INT_EN) {     
   /* 
    evt.capture_signal = 
      mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0);     
    evt.sel_cap_signal = MCPWM_SELECT_CAP0;
   //*/
    timer_get_counter_value(
      TIMER_GROUP_0,
      ((RFID_NODE*)arg)->timer.timer_idx,
      //&timer_value
      &evt.capture_signal
    );

    xQueueSendFromISR(((RFID_NODE *)arg)->cap_queue, &evt, NULL);
  }
  //TIMERG0.int_clr_timers.t0 = 1; // TODO: figure out timer reset
  pMCPWM->int_clr.val = mcpwm_intr_status;
}

/**
 * @brief Configure whole MCPWM module
 */
extern void input_capture_config(void *arg)
{
  mcpwm_gpio_initialize();
  mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, MCPWM_POS_EDGE, 0);
  
  pMCPWM->int_ena.val = CAP0_INT_EN;
  mcpwm_isr_register(MCPWM_UNIT_0, isr_handler, arg, ESP_INTR_FLAG_IRAM, NULL); 
}

extern void pwm_config(void *arg)
{
/*
   ledc_timer_config_t ledc_timer = {
     .duty_resolution = LEDC_TIMER_2_BIT, // resolution of PWM duty
     .freq_hz = 125e3,                      // frequency of PWM signal
     .speed_mode = LEDC_HS_MODE,           // timer mode
     .timer_num = LEDC_HS_TIMER            // timer index
   };
//*/
  ledc_timer_config_t ledc_timer = {
     .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
     .freq_hz = 50,                      // frequency of PWM signal
     .speed_mode = LEDC_HS_MODE,           // timer mode
     .timer_num = LEDC_HS_TIMER            // timer index
   };

   ledc_timer_config(&ledc_timer);

   ledc_channel_config_t ledc_channel = {
     .channel    = LEDC_HS_CH0_CHANNEL,
     .duty       = LEDC_TEST_DUTY,
     .gpio_num   = LEDC_HS_CH0_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .intr_type = LEDC_INTR_DISABLE,
     .timer_sel  = LEDC_HS_TIMER
   };

   ledc_channel_config(&ledc_channel);

   ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, LEDC_TEST_DUTY);
   ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

void IRAM_ATTR timer_isr(void *para)
{
  //int timer_idx = (int)para;
  rfid_timer_t *timer = (rfid_timer_t *)para;
  int timer_idx = timer->timer_idx;
  
  /* Retrieve the interrupt status and the counter value
     from the timer that reported the interrupt */
  uint32_t intr_status = TIMERG0.int_st_timers.val;
  TIMERG0.hw_timer[timer_idx].update = 1;
  uint64_t timer_counter_value = 
      ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32
      | TIMERG0.hw_timer[timer_idx].cnt_low;
  
  /* Prepare basic event data
     that will be then sent back to the main program task */
  timer_event_t evt;
  evt.timer_group = 0;
  evt.timer_idx = timer_idx;
  evt.timer_counter_value = timer_counter_value;
  
  /* Clear the interrupt
     and update the alarm time for the timer with without reload */
  if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {
    evt.type = RELOAD;
    TIMERG0.int_clr_timers.t0 = 1;
  } else {
    evt.type = -1; // not supported even type
  }
  
  /* After the alarm has been triggered
    we need enable it again, so it is triggered the next time */
  TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
  
  /* Now just send the event data back to the main program task */
  xQueueSendFromISR(timer->timer_queue, &evt, NULL);
}

//extern void timer_init(int timer_idx,bool auto_reload, double timer_interval_sec)
extern void rfid_timer_init(rfid_timer_t *timer)
{
  /* Select and initialize basic parameters of the timer */
  timer_config_t config;
  config.divider = TIMER_DIVIDER;
  config.counter_dir = TIMER_COUNT_UP;
  config.counter_en = TIMER_PAUSE;
  config.alarm_en = TIMER_ALARM_EN;
  config.intr_type = TIMER_INTR_LEVEL;
  config.auto_reload = timer->auto_reload;
  timer_init(TIMER_GROUP_0, timer->timer_idx, &config);

  /* Timer's counter will initially start from value below.
     Also, if auto_reload is set, this value will be automatically reload on alarm */
  timer_set_counter_value(TIMER_GROUP_0, timer->timer_idx, 0x00000000ULL);

  /* Configure the alarm value and the interrupt on alarm. */
  timer_set_alarm_value(
    TIMER_GROUP_0, 
    timer->timer_idx, 
    timer->timer_interval_sec * TIMER_SCALE
  );
  timer_enable_intr(TIMER_GROUP_0, timer->timer_idx);
  timer_isr_register(
    TIMER_GROUP_0, 
    timer->timer_idx, 
    timer_isr,
    (void *)timer, 
    ESP_INTR_FLAG_IRAM, 
    NULL
  );

  timer_start(TIMER_GROUP_0, timer->timer_idx);
}


