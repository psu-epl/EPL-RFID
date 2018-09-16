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
        .spics_io_num = RC522_PIN_CS,       //CS pin
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
    uint8_t bufferATQA[3] = {0, 0, 0};
    uint8_t bufferSize = sizeof(bufferATQA);
    uint8_t result;

    while (1)
    {
        bufferSize = 3;
        result = RC522_REQA_or_WUPA(PICC_CMD_REQA, &bufferATQA, &bufferSize);
        if (result == STATUS_OK || result == STATUS_COLLISION)
            ESP_LOGW(READER13, "Card detected!!!");
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
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
    t.addr = add << 1;        //MSB == 0 is for writing from Datasheet section 8.1.2.3.
    t.tx_buffer = (void *)&data;
    t.length = 8;                       //This is the length of the tx_buffer in bits
    ret = spi_device_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);              //Should have had no issues.
}

void write_reg_array(uint8_t add, uint8_t *data, uint8_t data_len)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); //Zero out the transaction
    t.addr = add << 1;        //MSB == 0 is for writing from Datasheet section 8.1.2.3.
    t.tx_buffer = (void *)&data;
    t.length = 8 * data_len;            //This is the length of the tx_buffer in bits
    ret = spi_device_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);              //Should have had no issues.
}

//Read register and return its value
uint8_t read_reg(uint8_t add)
{
    esp_err_t ret;
    spi_transaction_t t;
    uint8_t buffer;
    memset(&t, 0, sizeof(t));     //Zero out the transaction
    t.addr = (0x80 | (add << 1)); //MSB == 1 is for reading from Datasheet section 8.1.2.3.
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

//Read register and return its value
void read_reg_array(uint8_t add, uint8_t *data, uint8_t data_len)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));     //Zero out the transaction
    t.addr = (0x80 | (add << 1)); //MSB == 1 is for reading from Datasheet section 8.1.2.3.
    t.length = 8 * data_len;
    t.rx_buffer = (void *)data;
    ret = spi_device_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);              //Should have had no issues.
    /*
    We need the second transmit because
    on the transmit we don't get anything from RC522
    this is defined section 8.1.2.1
  */
    ret = spi_device_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);              //Should have had no issues.
}

void RC522_init()
{
    RC522_reset();
    /*
       Setup timeout timer
        This timer starts when the PCD stop transmission.
        Later we can get the IRQ Reg to see if the timer went off before a complete command
    */
    write_reg(TModeReg, 0x80); //Timer start automatically at end of transmission

    write_reg(TPrescalerReg, 0xA9); // Tprescaler LSB 7 bits of divider
    write_reg(TReloadRegL, 0xE8);   //time reload value, lower 8 bits
    write_reg(TReloadRegH, 0x03);   // timer reload value, higher 8 bits

    /* forces a 100% ASK modulation independent of modGsPReg  (Type-A)
     * ASK = Amplitude Shift Keying */
    write_reg(TxASKReg, 0x40);
    write_reg(ModeReg, 0x3D);

    RC522_antenna_on();
    ESP_LOGI(READER13, "Done Initializing");
}

uint8_t RC522_REQA_or_WUPA(uint8_t card_command, uint8_t *bufferATQA, uint8_t *bufferSize)
{
    uint8_t status;
    if (bufferATQA == NULL || *bufferSize < 2)
        return STATUS_NO_ROOM;

    clear_bits(CollReg, 0x80); // ValuesAfterColl=1 => Bits received after collision are cleared.
    status = RC522_communicate_with_card(PCD_TRANSCEIVE, 0x30, &card_command, 1, bufferATQA, bufferSize, 7);
    if (status != STATUS_OK)
        return status;

    if (*bufferSize > 2) // ATQA must be exactly 16 bits.
        return STATUS_ERROR;

    return STATUS_OK;
}

uint8_t RC522_communicate_with_card(uint8_t command, uint8_t irq, uint8_t *data_out, uint8_t data_out_len, uint8_t *data_in, uint8_t *data_in_len, uint8_t tx_last_bits)
{

    write_reg(CommandReg, PCD_IDLE);                      // Stop any active command.
    write_reg(ComIrqReg, 0x7F);                           // Clear all seven interrupt request bits
    write_reg(FIFOLevelReg, 0x80);                        // FlushBuffer = 1, FIFO initialization  
    write_reg(FIFODataReg, data_out[0]); //Write data to FIFO
    set_bits(BitFramingReg, tx_last_bits);                // Set the number of bits to be transmitted for the last command
    write_reg(CommandReg, command);                       //Execute the command
    

    if (command == PCD_TRANSCEIVE) //Start transmission of transceive see 9.3.1.14
        set_bits(BitFramingReg, 0x80);

    vTaskDelay(30 / portTICK_PERIOD_MS); //Sleep for 40 ms for the data to be sent out

    uint8_t status = read_reg(ComIrqReg);

    if ((status & 0x01) && !(status & irq)) //Check if a timeout happened from the timer and the waiting IRQ didn't happen
        return STATUS_TIMEOUT;
   
    uint8_t error_code = read_reg(ErrorReg);
    if (error_code & 0x13)
        return STATUS_ERROR;

    //return data
    if (command == PCD_TRANSCEIVE || command == PCD_RECEIVE)
    {
        uint8_t len = read_reg(FIFOLevelReg);

        *data_in_len = len;
        read_reg_array(FIFODataReg, data_in, len);
    }

    if (error_code & 0x08) // Check if bit collisions  CollErr
        return STATUS_COLLISION;

    return STATUS_OK;
}

//Perform soft reset on the reader
void RC522_reset()
{
    /*
        See section 9.3.1.2 of the datasheet about Softreset
    */
    write_reg(CommandReg, PCD_RESET); // Softreset the chip;
    while (read_reg(CommandReg) & 0x10)
        ; //Wait to exit PowerDown
    ESP_LOGI(READER13, "Powered up and ready");
}

/* Antenna Control 
 Section 8.6.3 of the datasheet 
*/
void RC522_antenna_on()
{
    //Check if it's not on
    if (!(read_reg(TxControlReg) & 0x03))
        set_bits(TxControlReg, 0x03);
    ESP_LOGI(READER13, "Antenna is on");
}

void RC522_antenna_off()
{
    clear_bits(TxControlReg, 0x03);
    ESP_LOGI(READER13, "Antenna is on");
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