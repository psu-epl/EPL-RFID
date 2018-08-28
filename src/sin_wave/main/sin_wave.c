/* LEDC (LED Controller) fade example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#include "soc/rtc_io_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc.h"

#include "driver/dac.h"



#define LEDC_TIMER             LEDC_TIMER_0
#define LEDC_CHANNEL           LEDC_CHANNEL_0
#define LEDC_HS_CH0_GPIO       (18)

#define CW_FREQUENCY_STEP 3



void app_main()
{
    // Enable tone generator common to both DAC channels 1 and 2
    SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL1_REG, SENS_SW_TONE_EN);
    // Enable / connect tone tone generator on / to channel 1
    SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN1_M);
    // Invert MSB, otherwise part of the waveform will be inverted
    SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_INV1, 2, SENS_DAC_INV1_S);
    // Set the frequency of waveform on CW output
    SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL1_REG, SENS_SW_FSTEP, CW_FREQUENCY_STEP, SENS_SW_FSTEP_S);

    dac_output_enable(DAC_CHANNEL_1);

}
