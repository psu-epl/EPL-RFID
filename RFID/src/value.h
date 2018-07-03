/*
 * value.h
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
 * version 1.60 / paulvh / Augustus 2017
 * initial version of value block handling
 */

#include "main.h"

#ifndef value_H_
#define value_H_

/////////////////////////////////////////////////////////////////////
//status flags
/////////////////////////////////////////////////////////////////////

#define V_OK			0
#define V_NOT_EMPTY		1
#define V_NOT_VALID		2
#define V_ERROR			3


/////////////////////////////////////////////////////////////////////
//routines to handle block value
/////////////////////////////////////////////////////////////////////

/* creates an initial value block*/
int create_value_block(double setvalue);

/* get value block number to use */
int get_value_block(int *vblock, uint8_t *vbuff);

/* change block-value */
int change_block_value(uint8_t action, double setvalue);

/* increment a block value */
int increment_value_block();

/* decrement a block value */
int decrement_value_block();

/* remove a block value and reset to 0x0 */
int remove_value_block();

/* sets a value in value block*/
int set_blck_value(int blck, double value);

/* show content of value block */
int show_value_block(int *block, double * value, int halt);

/* check for valide value block */
int validate_value_blck(uint8_t *vbuff);

/* calculate value from valid value block content */
int extract_value(uint8_t *vbuff, double *val);

/* read value from a specific block */
int value_from_card();

/* write value to a specific block */
int value_to_card();


#endif /* value_H_ */
