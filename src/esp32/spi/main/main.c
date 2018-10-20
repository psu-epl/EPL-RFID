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
    while (1)
    {
        if (RC522_read_card_uid() == STATUS_OK)
        {
            ESP_LOGW(READER13, "Card detected!!!");
        }

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
    t.tx_buffer = (void *)data;
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

bool RC522_detect_card()
{
    uint8_t bufferATQA[2] = {0, 0};
    uint8_t bufferSize = sizeof(bufferATQA);
    uint8_t result;

    bufferSize = 3;
    result = RC522_REQA_or_WUPA(PICC_CMD_REQA, bufferATQA, &bufferSize);
    if (result == STATUS_OK || result == STATUS_COLLISION)
        return true;

    return false;
}

uint8_t RC522_read_card_uid()
{
    uint8_t buffer[20];
    uint8_t bufferSize = sizeof(buffer);
    memset(&buffer, 0, bufferSize);  

    uint8_t back[20];
    uint8_t back_size = sizeof(back);
    memset(&back, 0, back_size);  
    uint8_t result;
    uint8_t anticoll_loop_max = 32; // max 32 bits (ISo standard)


    result = RC522_REQA_or_WUPA(PICC_CMD_REQA, buffer, &bufferSize);
    if (result != STATUS_OK && result != STATUS_COLLISION)
        return result;
    ESP_LOGI(READER13, "buffer one [0]:%x [1]:%x [2]:%x [3]:%x [4]:%x", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);

    write_reg(BitFramingReg, 0x00);
    clear_bits(CollReg, 0x80); // ValuesAfterColl=1 Bits received after collision are cleared.

// Reset baud rates
	write_reg(TxModeReg, 0x00);
	write_reg(RxModeReg, 0x00);
	// Reset ModWidthReg
	write_reg(ModWidthReg, 0x26);

    buffer[0] = PICC_CMD_SEL_CL1;
    buffer[1] =0x20;
    ESP_LOGI(READER13, "before send [0]:%x [1]:%x [2]:%x [3]:%x [4]:%x", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
    result = RC522_communicate_with_card(PCD_TRANSCEIVE, 0x30, buffer, 2, back, &back_size);
    ESP_LOGE(READER13, "resutl %x ",result);

    if (result != STATUS_OK && result != STATUS_COLLISION)
        return result;

    ESP_LOGI(READER13, "buffer two [0]:%x [1]:%x [2]:%x [3]:%x [4]:%x", back[0],back[1],back[2],back[3],back[4]);

    //ESP_LOGI(READER13, "Lower bye %x",bufferATQA[0]);
    // ESP_LOGI(READER13, "higher bye %x",bufferATQA[1]);

    uint8_t uid_size = buffer[0] >> 6;
    return STATUS_OK;
}


uint8_t RC522_communicate_with_card(uint8_t command, uint8_t irq, uint8_t *data_out, uint8_t data_out_len, uint8_t *data_in, uint8_t *data_in_len)
{

    write_reg(CommandReg, PCD_IDLE);                      // Stop any active command.
    write_reg(ComIrqReg, 0x7F);                           // Clear all seven interrupt request bits
    write_reg(FIFOLevelReg, 0x80);                        // FlushBuffer = 1, FIFO initialization
    write_reg_array(FIFODataReg, data_out, data_out_len); //Write data to FIFO
    write_reg(CommandReg, command);                       //Execute the command

    if (command == PCD_TRANSCEIVE) //Start transmission of transceive see 9.3.1.14
        set_bits(BitFramingReg, 0x80);

    uint8_t status = read_reg(ComIrqReg);
    while (!(status & 0x01) && !(status & irq)) //Spin until we get a timeout or the irq happens
        status = read_reg(ComIrqReg);

    if ((status & 0x01) && !(status & irq)) //Check if a timeout happened from the timer and the waiting IRQ didn't happen
     {
       ESP_LOGE(READER13, "IRQ reg %x ",read_reg(ComIrqReg));
       return STATUS_TIMEOUT;
     }


    uint8_t error_code = read_reg(ErrorReg);
    if (error_code & 0x13)
        return STATUS_ERROR;


    //return data
    if (command == PCD_TRANSCEIVE || command == PCD_RECEIVE)
    {
        uint8_t len = read_reg(FIFOLevelReg);

        *data_in_len = len;
        for (int i = 0; i < len; ++i)
        {
            data_in[i] = read_reg(FIFODataReg);
        }
    }

    if (error_code & 0x08) // Check if bit collisions  CollErr
        return STATUS_COLLISION;

    return STATUS_OK;
}


uint8_t RC522_REQA_or_WUPA(uint8_t card_command, uint8_t *bufferATQA, uint8_t *bufferSize)
{
    uint8_t status;
    if (bufferATQA == NULL || *bufferSize < 2)
        return STATUS_NO_ROOM;

    clear_bits(CollReg, 0x80); // ValuesAfterColl=1 Bits received after collision are cleared.
    write_reg(BitFramingReg, 0x07);
    status = RC522_communicate_with_card(PCD_TRANSCEIVE, 0x30, &card_command, 1, bufferATQA, bufferSize);
    if (status != STATUS_OK)
        return status;

    if (*bufferSize > 2) // ATQA must be exactly 16 bits.
        return STATUS_ERROR;

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