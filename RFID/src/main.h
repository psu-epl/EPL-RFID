/*
 * main.h
 *
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * The program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. 
 * 
 *  Created on: 26.09.2013
 *      Author: alexs
 * 
 * version 1.50 / paulvh / July 2017
 * Fixed number of bugs and code clean-up.
 * 
 * version 1.60 / paulvh / Augustus 2017
 * 	Fixed number of bugs and code clean-up.
 * 	included command line options
 * 	included support for value block
 * 
 * version 1.65.3 / paulvh / october 2017
 *  included read and write message over multiple data blocks
 *  write_block() call added
 *  read_block() call added
 *  read_block_raw() call added
 *  calc_sector_num() and calc_trailer() removed
 *  updated some error and changes for quality
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <bcm2835.h>
#include <ctype.h>
#include <wiringPi.h>

#ifndef MAIN_H_
#define MAIN_H_

#include "rfid.h"
#include "config.h"
#include "rfid.h"
#include "value.h"
#include "rc522.h"

// color display enable
#define RED 	1
#define GREEN 	2
#define YELLOW  3
#define BLUE	4
#define WHITE	5

#define REDSTR "\e[1;31m%s\e[00m"
#define GRNSTR "\e[1;92m%s\e[00m"
#define YLWSTR "\e[1;93m%s\e[00m"
#define BLUSTR "\e[1;34m%s\e[00m"

/////////////////////////////////////////////////////////////////////
//supporting routines main
/////////////////////////////////////////////////////////////////////

int init_13();
void * send_card_no(void* card_string);
int Led_On();

/* setup signals */
void set_signals();

/* catch signals to close out correctly */
void signal_handler(int sig_num);

/* perform init /reset BCM and GPIO */
uint8_t HW_init(uint32_t spi_speed, uint8_t gpio);
void close_out(int ex_code);

/* print message in color */
void p_printf (int level, char *format, ...);

/* read value from config file*/
int get_config_file();

/* display usage info */
void usage(char * str);

/* read trailer of a specific block from the card */
int read_trailer_block(int *t_addr, int addr, uint8_t * buff);

/* get block number
 * check whether a valid was provided on the command line with -B option */
int get_block_number(int blocks, int chk_trail);

/* get single digital number*/
int get_number();

/* get string */
void get_str(char * buf, int len);

/* get single (uppercase) character */
char get_charc();

/* check that string has valid number sequence */
int check_num(char *arg);

/* check the received message to write, allocate memory and return
 * pointer */
char * set_message(char *arg, int option, int *length);

/* convert a line with ascii to bin */
int ascii_to_hex(char *line, char *data, int filter_eof);

/////////////////////////////////////////////////////////////////////
//routines to access card
/////////////////////////////////////////////////////////////////////

/*authorize access to a block and read keyA or KeyB from config file*/
int authorize(uint8_t *pSnr, int block, int auth_key);

/* perform action on UID of card */
int uid_action();

/* perform action based on content specific block
 * action is defined in rc522.conf as [@code] */
int block_action();

/* find & action based on card UID or block content number */
int perform_action(char * act_str);

/* change access to a block (could be trailer as well...)*/
int change_block_access();

/* wait for card and obtain UID */
int get_card_info();

/* obtainblock permission */
int get_card_permission(int addr, int read_card);

/* display data or value block access permission */
int disp_access_perm(int access, uint8_t *buff);

/* display trailer access permission */
int disp_trailer_perm(int access, uint8_t *buff);

/* display card detail information */
void disp_card_details();

/* calculate the access bits for a block */
int calc_access_bits(int access, uint8_t *buff);

/* update access bits on the trailer */
int update_access_bits_on_card(char ch, int addr);

/* set the access bit for a block */
void set_access_bits(char ch, int block, uint8_t *buff);

/* update KEYA or KEYB for a block */
int key_upd();

/*read content of card and store in temp file */
int read_card_to_file();

/* write/read a specific block*/
int write_block(int addr, uint8_t *uvalue, int check_empty, int read_after_write);
int read_block_raw(int addr, uint8_t *mess);
int read_block(int addr, unsigned char *mess, int disp);

/*read of write content to a specific block on the card*/
int read_from_card();
int write_to_card();

/* write a message in batch mode */
int write_message(uint8_t *mess, int start_sector, int start_block, int len);

/////////////////////////////////////////////////////////////////////
//global variables
/////////////////////////////////////////////////////////////////////
extern uint8_t debug;			// enable debug messages
extern int use_vblock;			// value block to use (-B option)
extern int max_blocks;			// max_blocks on card
extern uint8_t SN[10];          // serial number of card
extern uint8_t SN_len;          // length of serial number (4/7/10)
extern int use_vblock;			// value block to use (-B option)

/**
 * For details see MF1ICS50_rev5_3.pdf
 * 
 * Each sector (max 16) has 4 blocks. (block 0 - 3)
 * Block 3 is the trailer and contains
 * 6 bytes KEYA
 * 4 bytes access bits
 * 6 bytes KEYB (optional)
 * 
 *  
 * The access bytes are stored in BYTE 6 ,7 8 in the trailer
 *  Cx-y =  access control bit x for block y 
 * [Cx-y] = inverted access control bit x for block y (between brackets)
 * 
 * 
 *     bit  7      6      5       4      3      2     1      0    
 * BYTE 6 [C2-3] [C2-2] [C2-1] [C2-0] [C1-3] [C1-2] [C1-1] [C1-0]
 * BYTE 7  C1-3   C1-2   C1-1   C1-0  [C3-3] [C3-2] [C3-1] [C3-0]
 * BYTE 8  C3-3   C3-2   C3-1   C3-0   C2-3   C2-2   C2-1   C2-0
 * BYTE 9  not used
 * 
 * FOR THE TRAILER ONlY:
 * It will indicate whether read/write access is with KEYA, KEYB, either KEYA or KEYB 
 * It is depending on the combination of the bits C1-3 , C2-3, C3-3
 * If new delivered the status bits are  0  0 1 and as such KEYA can be used to read and write all.
 * 
 * FOR BLOCK 0,1, 2
 * Depending on access bits for each block the read/write access is either with KEYA, 
 * KEYB, either KEYA or KEYB of never. It also indicates whether the block is a data or value block
 * 
 * For details see MF1ICS50_rev5_3.pdf
 * 
 * To extract the correct access bits the:
 * 
 * BxByCz = BLOCK x, from BYTE y, CONTROLbit z
 * 
 */ 

#define B0B7C1 0X10 // B0 = block 0, B7 = Byte 7, C1 = condition 1
#define B0B8C2 0X01
#define B0B8C3 0X10

#define B1B7C1 0X20
#define B1B8C2 0X02
#define B1B8C3 0X20

#define B2B7C1 0X40
#define B2B8C2 0X04
#define B2B8C3 0X40

#define TB7C1 0X80  // T = Trailer, B7 = Byte 7, C1 = condition 1
#define TB8C2 0X08
#define TB8C3 0X80


#endif /* MAIN_H_ */
