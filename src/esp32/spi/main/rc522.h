

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"



//MF522 commands for the CommandReg
#define PCD_IDLE              0x00		// no action, cancels current command execution
#define PCD_AUTHENT           0x0E		// perform the MIFARE standard authentication as a reader
#define PCD_RECEIVE           0x08		// activate the receiver circuits
#define PCD_TRANSMIT          0x04		// transmits data from the FIFO buffer
#define PCD_TRANSCEIVE        0x0C		// transmits data from the FIFO buffer & activate receive after transmission
#define PCD_RESET             0x0F		// softreset command
#define PCD_CALCCRC           0x03		// activates the CRC co-processor or perform a selftest
#define PCD_NOCHANGE          0x05      // no command change. To modify the CommandReg register bits


/** MF522 registers */
// command and status
#define     CommandReg            0x01	// starts and stops command execution
#define     ComIEnReg             0x02	// enable and disable interrupt request control bits
#define     DivlEnReg             0x03	// enable and disable interrupt request control bits
#define     ComIrqReg             0x04	// interrupt request bits
#define     DivIrqReg             0x05	// interrupt request bits
#define     ErrorReg              0x06	// error bits showing the error status of last command
#define     Status1Reg            0x07	// communication status bits
#define     Status2Reg            0x08	// receiver and transmitter status bits
#define     FIFODataReg           0x09	// input and output of 64 byte FIFO buffer
#define     FIFOLevelReg          0x0A	// number of bytes stored in the FIFO buffer
#define     WaterLevelReg         0x0B	// level for FIFO underflow and overflow warning
#define     ControlReg            0x0C	// miscellaneus control reigsters
#define     BitFramingReg         0x0D	// adjustments for bit-oriented frames
#define     CollReg               0x0E	// bit position of the first bit-collision detected on RF int.
// command
#define     ModeReg               0x11	// defines general modes for transmitting and receiving
#define     TxModeReg             0x12	// defines transmission data rate and framing
#define     RxModeReg             0x13	// defines reception data rate and framing
#define     TxControlReg          0x14	// controls the logical behavior of the antenna driver TX1/ TX2
#define     TxASKReg              0x15	// controls the setting of the transmission modulation
#define     TxSelReg              0x16	// selects the internal sources for the antenna driver
#define     RxSelReg              0x17	// selects the internal receiver settings
#define     RxThresholdReg        0x18	// selects thresholds for the bit decoder
#define     DemodReg              0x19	// defines demodulator settings
#define     MiTXReg               0x1C	// controls some MIFARE communication transmit parameters
#define     MiRXReg               0x1D	// controls some MIFARE communication receive parameters
#define     SerialSpeedReg        0x1F	// selects the speed for the serial UART interface
// configuration
#define     CRCResultRegM         0x21	// shows the MSB value for the CRC calculation
#define     CRCResultRegL         0x22	// shows the LSB value for the CRC calculation
#define     ModWidthReg           0x24	// controls the ModWidth settings
#define     RFCfgReg              0x26	// configures the receiver gain
#define     GsNReg                0x27	// selects the conductance of the antenna on TX1/TX2 modulation
#define     CWGsCfgReg            0x28	// defines the conductance of the p-driver output
#define     ModGsCfgReg           0x29  // defines the conductance of the p-driver output
#define     TModeReg              0x2A  // defines settings for the internal timer
#define     TPrescalerReg         0x2B  // defines settings for the internal timer
#define     TReloadRegH           0x2C  // defines the 16 bit timer reload value
#define     TReloadRegL           0x2D  // defines the 16 bit timer reload value
#define     TCounterValueRegH     0x2E  // shows the 16 bit timer reload value
#define     TCounterValueRegL     0x2F  // shows the 16 bit timer reload value
// test register
#define     TestSel1Reg           0x31  // general test signal configuration
#define     TestSel2Reg           0x32  // general test signal configuration and PRBS control
#define     TestPinEnReg          0x33	// enables pin output driver on pins D1 to D7
#define     TestPinValueReg       0x34	// defines values for D1 to D7 when used as I/O bus
#define     TestBusReg            0x35	// shows the status of the internal test bus
#define     AutoTestReg           0x36	// controls the digital self test
#define     VersionReg            0x37	// shows the software version
#define     AnalogTestReg         0x38	// controls the pins AUX1 and AUX2
#define     TestDAC1Reg           0x39	// defines the test value for testDAC1
#define     TestDAC2Reg           0x3A  // defines the test value for testDAC2
#define     TestADCReg            0x3B	// shows the value of ADC I and Q channels



//ESP32 WROOM HSPI Pins
#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK 14

//RC522 Chip Select
#define RC522_PIN_CS 15




void write_out(uint8_t add, uint8_t data);
uint8_t read_reg(uint8_t add);