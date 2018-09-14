#include "rc522.h"
#include "esp_log.h"



#define READER13 "13.5 MHz reader"

static spi_device_handle_t spi;


void app_main(void)
{
  esp_err_t ret;
  spi_bus_config_t buscfg = {
      .miso_io_num = PIN_NUM_MISO,
      .mosi_io_num = PIN_NUM_MOSI,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
  };

  //Device configuration struct
  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = 10 * 1000 * 1000, //Clock out at 10 MHz
      .mode = 0,                          //SPI mode 0
      .spics_io_num = RC522_PIN_CS,         //CS pin
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

  RC522_init();
}



/*
Note:
When reading and writing using ESP-IDF we can 
either use the .addr or set it 0 and include the address
in the data being sent.
*/

//Write out to a register
void write_reg(uint8_t add, uint8_t data)
{
  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t)); //Zero out the transaction
  t.addr = add<<1;             //MSB == 0 is for writing from Datasheet section 8.1.2.3.
  t.tx_buffer = (void *)&data;
  t.length = 8;              //This is the length of the tx_buffer in bits
  ret = spi_device_transmit(spi, &t); //Transmit!
  assert(ret == ESP_OK);              //Should have had no issues.
}

//Read register and return its value
uint8_t read_reg(uint8_t add)
{
  esp_err_t ret;
  spi_transaction_t t;
  uint8_t buffer;
  memset(&t, 0, sizeof(t)); //Zero out the transaction
  t.addr = (0x80|(add<<1));             //MSB == 1 is for reading from Datasheet section 8.1.2.3.
  t.length = 8;
  t.rx_buffer = (void *)&buffer;
  ret = spi_device_transmit(spi, &t); //Transmit!
  assert(ret == ESP_OK);              //Should have had no issues.
  /*
    We need the second transmit because
    on the transmit we don't get anything from RC522
    this is defined section 8.1.2.1
  */
  ret = spi_device_transmit(spi, &t); //Transmit!
  assert(ret == ESP_OK);              //Should have had no issues.
  return buffer;
}

void RC522_init()
{
    RC522_reset();


    //Setup timeout timer
    write_reg(TModeReg,0x80); //Timer start automatically at end of transmission

    write_reg(TPrescalerReg,0xA9); // Tprescaler LSB 7 bits of divider
    write_reg(TReloadRegL,0xE8); //time reload value, lower 8 bits 
    write_reg(TReloadRegH,0x03); // timer reload value, higher 8 bits

    /* forces a 100% ASK modulation independent of modGsPReg  (Type-A)
     * ASK = Amplitude Shift Keying */
    write_reg(TxASKReg,0x40);
    ESP_LOGI(READER13,"Done Initializing");


}

//Perform soft reset on the reader
void RC522_reset()
{
    /*
        See section 9.3.1.2 of the datasheet about Softreset
    */
    write_reg(CommandReg,PCD_RESET); // Softreset the chip;
    while(read_reg(CommandReg) & 0x10); //Wait to exit PowerDown
    ESP_LOGI(READER13,"Powered up and ready");

}


/* Antenna Control 
 Section 8.6.3 of the datasheet 
*/
void RC522_antenna_on()
{
    //Check if it's not on
    if (!(read_reg(TxControlReg) & 0x03))
            set_bits(TxControlReg, 0x03);
    ESP_LOGI(READER13,"Antenna is on");
}

void RC522_antenna_off()
{
    clear_bits(TxControlReg, 0x03);
    ESP_LOGI(READER13,"Antenna is on");

}

//Clear bits marked by the mask in a given register
void clear_bits(uint8_t reg, uint8_t mask)
{
    uint8_t temp = read_reg(reg);
    write_reg(reg, temp & ~mask);
}

//Set bits marked by the mask in a given register
void set_bits(uint8_t reg, uint8_t mask)
{
    uint8_t temp = read_reg(reg);
    write_reg(reg, temp | mask);
}