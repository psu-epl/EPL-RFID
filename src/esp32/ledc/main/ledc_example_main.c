#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0

#define LEDC_TEST_CH_NUM       (1)
#define LEDC_TEST_DUTY         2

void app_main()
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
