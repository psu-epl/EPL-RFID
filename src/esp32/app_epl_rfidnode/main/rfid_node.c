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
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);

  mcpwm_pin_config_t pin_config = {
    .mcpwm0a_out_num = GPIO_PWM0A_OUT,
    .mcpwm0b_out_num = GPIO_PWM0B_OUT//,
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
  pwm_config.frequency = 1000;    //frequency = 1000Hz
  pwm_config.cmpr_a = 60.0;       //duty cycle of PWMxA = 60.0%
  pwm_config.cmpr_b = 50.0;       //duty cycle of PWMxb = 50.0%
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

  vTaskDelete(NULL);
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
      printf("CAP0 : %d us\n", current_cap_value[0]);
    }
  }
}


extern void https_get_task(void *vpRFID_NODE)
{
    char buf[512];
    int ret, len;

    ESP_LOGI(TAG, "Something weird");

    while(1) {
        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
           esp_tls_cfg_t cfg = {
            .cacert_pem_buf  = server_root_cert_pem_start,
            .cacert_pem_bytes = server_root_cert_pem_end - server_root_cert_pem_start,
        };

        struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, &cfg);

        if(tls != NULL) {
            ESP_LOGI(TAG, "Connection established...");
        } else {
            ESP_LOGE(TAG, "Connection failed...");
            goto exit;
        }

        size_t written_bytes = 0;
        do {
            ret = esp_tls_conn_write(tls,
                                     REQUEST + written_bytes,
                                     strlen(REQUEST) - written_bytes);
            if (ret >= 0) {
                ESP_LOGI(TAG, "%d bytes written", ret);
                written_bytes += ret;
            } else if (ret != MBEDTLS_ERR_SSL_WANT_READ  && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
                ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
                goto exit;
            }
        } while(written_bytes < strlen(REQUEST));

        ESP_LOGI(TAG, "Reading HTTP response...");

        do
        {
            len = sizeof(buf) - 1;
            bzero(buf, sizeof(buf));
            ret = esp_tls_conn_read(tls, (char *)buf, len);

            if(ret == MBEDTLS_ERR_SSL_WANT_WRITE  || ret == MBEDTLS_ERR_SSL_WANT_READ)
                continue;

            if(ret < 0)
           {
                ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
                break;
            }

            if(ret == 0)
            {
                ESP_LOGI(TAG, "connection closed");
                break;
            }

            len = ret;
            ESP_LOGD(TAG, "%d bytes read", len);
            /* Print response directly to stdout as it is read */
            for(int i = 0; i < len; i++) {
                putchar(buf[i]);
            }
        } while(1);

    exit:
        esp_tls_conn_delete(tls);
        putchar('\n'); // JSON output doesn't have a newline at end

        static int request_count;
        ESP_LOGI(TAG, "Completed %d requests", ++request_count);

        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}