/**
 * @brief Template for the EPL RFID project
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK 14
#define PIN_NUM_CS 15

#define INTENSITY_REG 0x0A
#define SCANLIMIT_REG 0x0B
#define SHUTDOWN_REG 0x0C
#define DECODE_REG 0x09

#define DEFAULT_INTENSITY 0x0F  //Full intensity
#define DEFAULT_NUM_DIGITS 0x07 //All digits

static spi_device_handle_t spi;

void all_off();
int display(uint8_t digit, uint8_t num);
void write_out(uint8_t add, uint8_t data);

void app_main(void)
{
  esp_err_t ret;
  spi_bus_config_t buscfg = {
      .miso_io_num = -1,
      .mosi_io_num = PIN_NUM_MOSI,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
  };

  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = 10 * 1000 * 1000, //Clock out at 10 MHz
      .mode = 0,                          //SPI mode 0
      .spics_io_num = PIN_NUM_CS,         //CS pin
      .queue_size = 7,                    //We want to be able to queue 7 transactions at a time
      .command_bits = 0,                  //The number of command bits for the device
      .address_bits = 8                   //The number of bit used for the address
  };

  //Initialize the SPI bus
  ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0); //The 0 is to disable DMA
  ESP_ERROR_CHECK(ret);

  //Attach the LCD to the SPI bus
  ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
  ESP_ERROR_CHECK(ret);

  write_out(SHUTDOWN_REG, 0x01);                //Exit shutdown mode;
  write_out(DECODE_REG, 0xFF);                  //Code B decode for digits 7–0
  write_out(INTENSITY_REG, DEFAULT_INTENSITY);  //set intensity
  write_out(SCANLIMIT_REG, DEFAULT_NUM_DIGITS); //set number of digits to light
  all_off();
  for (int i = 1; i < 9; ++i)
  {
    all_off();
    display(i, i);
    ets_delay_us(1000000);
  }
}

void write_out(uint8_t add, uint8_t data)
{
  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t)); //Zero out the transaction
  t.addr = add;
  t.tx_buffer = (void *)&data;
  t.length = 8;
  ret = spi_device_transmit(spi, &t); //Transmit!
  assert(ret == ESP_OK);              //Should have had no issues.
}

int display(uint8_t digit, uint8_t num)
{

  if (digit < 1 || digit > 8 || num > 15)
    return 0;
  write_out(digit, num);
  return 0;
}

void all_off()
{
  for (int i = 1; i < 9; ++i)
    write_out(i, 0x0F);
}
