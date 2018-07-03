/*
 * config.h
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
 *  Created on: 05.09.2013
 *      Author: alexs
 * 
 * version 1.51 / paulvh / July 2017
 * Fixed number of bugs and code clean-up.
 */

#include "main.h"

#ifndef CONFIG_H_
#define CONFIG_H_

#define default_config_file "/etc/RC522.conf"

/////////////////////////////////////////////////////////////////////
//routines to handle config file
/////////////////////////////////////////////////////////////////////

// read User ID and Group ID from config 
int read_conf_uid();

// open config file
int open_config_file(char *);

//close config file
void close_config_file();

// find specific in config file
int find_config_param(char * param_name, char * param_val, int val_len) ;

// read for alternative KeyA of KeyB on block
int read_conf_key(int addr, int auth_key);

// append string to config-file
int add_to_config(char *buf);

/////////////////////////////////////////////////////////////////////
//GLOBAL variables in config.c
/////////////////////////////////////////////////////////////////////

extern uint8_t KEYA[6];
extern uint8_t KEYB[6];
extern char config_file[255];

#endif /* CONFIG_H_ */
