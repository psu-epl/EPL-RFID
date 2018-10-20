

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

//ESP32 WROOM HSPI Pins
#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK 14

//RC522 Chip Select
#define RC522_PIN_CS 15

//MF522 commands for the CommandReg
#define PCD_IDLE 0x00       // no action, cancels current command execution
#define PCD_AUTHENT 0x0E    // perform the MIFARE standard authentication as a reader
#define PCD_RECEIVE 0x08    // activate the receiver circuits
#define PCD_TRANSMIT 0x04   // transmits data from the FIFO buffer
#define PCD_TRANSCEIVE 0x0C // transmits data from the FIFO buffer & activate receive after transmission
#define PCD_RESET 0x0F      // softreset command
#define PCD_CALCCRC 0x03    // activates the CRC co-processor or perform a selftest
#define PCD_NOCHANGE 0x05   // no command change. To modify the CommandReg register bits

/** MF522 registers */
// command and status
#define CommandReg 0x01    // starts and stops command execution
#define ComIEnReg 0x02     // enable and disable interrupt request control bits
#define DivlEnReg 0x03     // enable and disable interrupt request control bits
#define ComIrqReg 0x04     // interrupt request bits
#define DivIrqReg 0x05     // interrupt request bits
#define ErrorReg 0x06      // error bits showing the error status of last command
#define Status1Reg 0x07    // communication status bits
#define Status2Reg 0x08    // receiver and transmitter status bits
#define FIFODataReg 0x09   // input and output of 64 byte FIFO buffer
#define FIFOLevelReg 0x0A  // number of bytes stored in the FIFO buffer
#define WaterLevelReg 0x0B // level for FIFO underflow and overflow warning
#define ControlReg 0x0C    // miscellaneus control reigsters
#define BitFramingReg 0x0D // adjustments for bit-oriented frames
#define CollReg 0x0E       // bit position of the first bit-collision detected on RF int.
// command
#define ModeReg 0x11        // defines general modes for transmitting and receiving
#define TxModeReg 0x12      // defines transmission data rate and framing
#define RxModeReg 0x13      // defines reception data rate and framing
#define TxControlReg 0x14   // controls the logical behavior of the antenna driver TX1/ TX2
#define TxASKReg 0x15       // controls the setting of the transmission modulation
#define TxSelReg 0x16       // selects the internal sources for the antenna driver
#define RxSelReg 0x17       // selects the internal receiver settings
#define RxThresholdReg 0x18 // selects thresholds for the bit decoder
#define DemodReg 0x19       // defines demodulator settings
#define MiTXReg 0x1C        // controls some MIFARE communication transmit parameters
#define MiRXReg 0x1D        // controls some MIFARE communication receive parameters
#define SerialSpeedReg 0x1F // selects the speed for the serial UART interface
// configuration
#define CRCResultRegM 0x21     // shows the MSB value for the CRC calculation
#define CRCResultRegL 0x22     // shows the LSB value for the CRC calculation
#define ModWidthReg 0x24       // controls the ModWidth settings
#define RFCfgReg 0x26          // configures the receiver gain
#define GsNReg 0x27            // selects the conductance of the antenna on TX1/TX2 modulation
#define CWGsCfgReg 0x28        // defines the conductance of the p-driver output
#define ModGsCfgReg 0x29       // defines the conductance of the p-driver output
#define TModeReg 0x2A          // defines settings for the internal timer
#define TPrescalerReg 0x2B     // defines settings for the internal timer
#define TReloadRegH 0x2C       // defines the 16 bit timer reload value
#define TReloadRegL 0x2D       // defines the 16 bit timer reload value
#define TCounterValueRegH 0x2E // shows the 16 bit timer reload value
#define TCounterValueRegL 0x2F // shows the 16 bit timer reload value
// test register
#define TestSel1Reg 0x31     // general test signal configuration
#define TestSel2Reg 0x32     // general test signal configuration and PRBS control
#define TestPinEnReg 0x33    // enables pin output driver on pins D1 to D7
#define TestPinValueReg 0x34 // defines values for D1 to D7 when used as I/O bus
#define TestBusReg 0x35      // shows the status of the internal test bus
#define AutoTestReg 0x36     // controls the digital self test
#define VersionReg 0x37      // shows the software version
#define AnalogTestReg 0x38   // controls the pins AUX1 and AUX2
#define TestDAC1Reg 0x39     // defines the test value for testDAC1
#define TestDAC2Reg 0x3A     // defines the test value for testDAC2
#define TestADCReg 0x3B      // shows the value of ADC I and Q channels

// Commands sent to the PICC.

// The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4)
#define PICC_CMD_REQA 0x26   // REQuest command Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
#define PICC_CMD_WUPA 0x52    // Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
#define PICC_CMD_CT 0x88      // Cascade Tag. Not really a command, but used during anti collision.
#define PICC_CMD_SEL_CL1 0x93 // Anti collision/Select, Cascade Level 1
#define PICC_CMD_SEL_CL2 0x95 // Anti collision/Select, Cascade Level 2
#define PICC_CMD_SEL_CL3 0x97 // Anti collision/Select, Cascade Level 3
#define PICC_CMD_HLTA 0x50   // HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
#define PICC_CMD_RATS 0xE0    // Request command for Answer To Reset.
// The commands used for MIFARE Classic (from http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf, Section 9)
// Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
// The read/write commands can also be used for MIFARE Ultralight.
#define PICC_CMD_MF_AUTH_KEY_A = 0x60 // Perform authentication with Key A
#define PICC_CMD_MF_AUTH_KEY_B = 0x61 // Perform authentication with Key B
#define PICC_CMD_MF_READ = 0x30      // Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
#define PICC_CMD_MF_WRITE = 0xA0     // Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
#define PICC_CMD_MF_DECREMENT = 0xC0  // Decrements the contents of a block and stores the result in the internal data register.
#define PICC_CMD_MF_INCREMENT = 0xC1  // Increments the contents of a block and stores the result in the internal data register.
#define PICC_CMD_MF_RESTORE = 0xC2    // Reads the contents of a block into the internal data register.
#define PICC_CMD_MF_TRANSFER = 0xB0   // Writes the contents of the internal data register to a block.
                                       // The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
                                       // The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
#define PICC_CMD_UL_WRITE = 0xA2       // Writes one 4 byte page to the PICC.

// Return codes from the functions
enum StatusCode
{
    STATUS_OK,             // Success
    STATUS_ERROR,          // Error in communication
    STATUS_COLLISION,      // Collission detected
    STATUS_TIMEOUT,        // Timeout in communication.
    STATUS_NO_ROOM,        // A buffer is not big enough.
    STATUS_INTERNAL_ERROR, // Internal error in the code. Should not happen ;-)
    STATUS_INVALID,        // Invalid argument.
    STATUS_CRC_WRONG,      // The CRC_A does not match
    STATUS_MIFARE_NACK     // A MIFARE PICC responded with NAK.
};



void write_reg(uint8_t add, uint8_t data);
void write_reg_array(uint8_t add, uint8_t *data, uint8_t data_len);

uint8_t read_reg(uint8_t add);
void read_reg_array(uint8_t add, uint8_t *data, uint8_t data_len);

void set_bits(uint8_t reg, uint8_t mask);
void clear_bits(uint8_t reg, uint8_t mask);

bool RC522_detect_card();
uint8_t RC522_read_card_uid();

uint8_t RC522_communicate_with_card(uint8_t command, uint8_t irq, uint8_t *data_out, uint8_t data_out_len, uint8_t *data_in, uint8_t *data_in_len);
uint8_t RC522_REQA_or_WUPA(uint8_t card_command, uint8_t *bufferATQA, uint8_t *bufferSize);
void RC522_antenna_on();
void RC522_antenna_off();

void RC522_reset();
void RC522_init();
