/*
 * value.c
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
 * initial version for value block handling
 */

#include "value.h"

/* check for valid value block
 * 
 * return code:
 * V_OK : ok
 * V_NOT_VALID : error
 */

int validate_value_blck(uint8_t *vbuff)
{
	int i, status = V_OK ;
	
	// check the value bytes
	for (i = 0 ; i< 4 ;i++)
	{
		if (vbuff[i] != vbuff[i+8] || vbuff[i] + vbuff[i+4] != 0xff)
			status = V_NOT_VALID;
	}
	
	// check the block bytes
	if (vbuff[12] != vbuff[14]
	|| vbuff[12] + vbuff[13] != 0xff
	|| vbuff[12] + vbuff[15] !=0xff)	status = V_NOT_VALID;

	return(status);
}

/* get value block number to use
 * 
 * vbuff : to store the content of the value block
 * vblock: to store the value block.
 * 
 * return code:
 * V_OK : ok
 * V_ERROR : general error
 */
int get_value_block(int *vblock, uint8_t *vbuff)
{
	int addr;
	
    // wait for card
    if (get_card_info() != TAG_OK)   return(V_ERROR);

	p_printf(RED, "**********************************\n");
	p_printf(RED, "* Do NOT remove card from reader *\n");
	p_printf(RED, "**********************************\n");
	
    // get value block (check it is NOT a trailer)
	p_printf (YELLOW,"Please provide the value block number to use. \n");
	p_printf (YELLOW,"Max %d. (0 is exit) : ", max_blocks);
	if ((addr=get_block_number(max_blocks, 1)) == 0x0)  close_out(0);
	
	// read the value block from card
	if (value_from_card(addr, vbuff) != V_OK) return(V_ERROR);
	
	// return value block number
	*vblock = addr;
	
	// return OK
	return(V_OK);
}   

/* read a specific value block from the card 
 * 
 * blck : block to read
 * vbuff : store content of block
 * 
 * return code
 * V_OK : OK
 * V_NOT_VALID : error no permission
 * V_ERROR : read error
 */

int value_from_card(int blck, uint8_t *vbuff)
{
    int     i, status;
    uint8_t	buff[MAXRLEN];
    char	ch;
    char	auth = 'A'; 

	// get block permission. 0 = do not wait for card (again)
    if ((ch = get_card_permission(blck,0)) == 0xff) return(V_ERROR);

	// if set to restricted level we need to use KEYB in case we want to increment
	if (ch == 6) auth = 'B';
	
    // as we use keyA for access, bit 1 should not be set, unless the value is 1
    // see MF1IC520 card description.

    else if ((ch & 0x01) && (ch != 1) )
    {
        p_printf(RED,"\nThe block %d has no read permission. (%x) \n", blck,ch);
        PcdHalt();
        return(V_NOT_VALID);
    }

	else if (ch == 0)
	{
		p_printf(YELLOW,"\n****************** WARNING ************************ \n"); 
		p_printf(RED,"Block %d has unrestricted permission (%d) for a value block.\n",blck, ch);
		p_printf(YELLOW,"************** END WARNING ************************\n\n"); 
	}		

	// value block should have permission 6 or 1
	else if (ch != 6 && ch != 1)
	{
		p_printf(YELLOW,"\n****************** WARNING ************************ \n"); 
		p_printf(RED,"Block %d has incorrect permissions (%d) for a value block.\n",blck, ch);
		p_printf(YELLOW,"************** END WARNING ************************\n\n"); 
	}
	
	if(auth == 'A')
		status = authorize(SN, blck, PICC_AUTHENT1A);
	else
		status = authorize(SN, blck, PICC_AUTHENT1B);
    
    // access memory block
    if (status != TAG_OK) 
    {
		PcdHalt();
		return(V_NOT_VALID);
	}
    
    // read block
    if (PcdRead(blck,buff) == TAG_OK)
		for(i = 0; i<16; i++)	vbuff[i]= buff[i];

	else
	{
		p_printf(RED, "Error during reading block %d\n",blck);
		PcdHalt();
		return(V_ERROR);
	}
    
    return(V_OK);
}

/* write to specific block on the card 
 * blck : blpock address to write to
 * vbuff : content to write 
 * 
 * return code :
 * V_ERROR : error
 * V_OK : OK 
 */
int value_to_card(int blck, uint8_t * vbuff)
{
    unsigned char   buff[40];

    // perform write
    if (PcdWrite(blck, vbuff) != TAG_OK)
    {
        p_printf(RED,"Error during writing.\n");
        PcdHalt();
        return(V_ERROR);
    }

	if (debug)
	{
		// read back to double check
		if (read_tag_str(blck,buff) == TAG_OK)
			p_printf(GREEN, "Block %d: now has the following content %s.\n",blck, buff);
		else
			p_printf(RED, "Error during reading block %d\n",blck);
	}

    return(V_OK);
}

/* sets a value in value block
 * blck = value block number
 * value = value to set
 * 
 * format to sent to card
 * 
 * byte 0 -3 value (LSB byte 0, MSB byte 3)
 * byte 4 -7 inverted value
 * byte 8 -11 value ( same as byte 0 -3) 
 * byte 12 block address 
 * byte 13 inverted block address
 * byte 14 block address (same as byte 14)
 * byte 15 inverted block addrress (same as byte 13)
 * 
 * return code
 * V_OK : ok
 * V_NOT_VALID : could not create valid value block
 * V_ERROR : general error
 */
int set_blck_value(int blck, double value)
{
	uint8_t	vbuff[MAXRLEN];
	int i;
	
	// set value bytes
	vbuff[3] = vbuff[11] = ((long long) value & (long long) 0xff000000) >> 24;
	vbuff[2] = vbuff[10] = ((long long) value & (long long) 0xff0000) >> 16;
	vbuff[1] = vbuff[9]  = ((long long) value & (long long) 0xff00) >> 8;
	vbuff[0] = vbuff[8]  = ((long long) value & (long long) 0xff);		
	
	// set invert value to byte 4- 7
	for (i =0; i< 4; i++) vbuff[4 + i] = ~vbuff[i];
		
	// set blck (bytes 12 & 14)
	vbuff[12] = vbuff[14] = (uint8_t) blck;
	
	// set invert blck (byte 13 & 15
	vbuff[13] = vbuff[15] = ~vbuff[12];

	// This should ALWAYS be good... but this is a double check
	if (validate_value_blck(vbuff) != V_OK)
	{
		p_printf(RED,"Error during creating value block\n");
		PcdHalt();
		return(V_NOT_VALID);
	}	

	if (debug)
	{
		p_printf(GREEN, "DEBUG: constructed value block: ");
		for (i=0 ; i < 16; i++)
			p_printf(YELLOW, "%02x.",(int) vbuff[i]);
		p_printf(WHITE,"\n");
	}

	// write to card
	if (value_to_card(blck, vbuff) != V_OK) 
	{
		PcdHalt();
		return(V_ERROR);
	}
	
	return(V_OK);
}

/* creates an initial value block
 * setvalue = incitial value to set
 * 
 * return code
 * V_OK : ok
 * V_ERROR : general error
 * V_NOT_EMPTY : requested block had content
 * V_NOT_VALID : not a valid value block
 */
 
int create_value_block(double setvalue)
{
	int		t_addr, vblock,i;
	uint8_t vbuff[MAXRLEN];
	char	buff[20];
	unsigned char ch;
	
	p_printf(BLUE, "Create value block.\n");
	
	// get block content
	if (get_value_block(&vblock, vbuff) != V_OK) return(V_ERROR);
	
	// check for any content
	for (i = 0; i< 16 ; i++)
		if (vbuff[i] != 0x0) break;
		
	// if any content than check for overwrite
	if (i != 16 )
	{
		// check this is valid value block
		if (validate_value_blck(vbuff) == V_OK)
		{
			p_printf(RED,"Block %d is already valid value block.\n", vblock);
			p_printf(YELLOW,"First remove value block with -R option\n");
			PcdHalt();
			return(V_NOT_EMPTY);
		}
		
		p_printf(RED,"Block is not empty.\n");

		// display block content
	    p_printf(GREEN,"\nBlock %d has content: ",vblock);
	
		for(i = 0; i<16; i++)	p_printf(YELLOW,"%02x.",vbuff[i]);
	
		// ask to continue?
		do
	    {
	        p_printf(YELLOW,"\n\nAre you sure to overwrite this block %d (yes, no or exit) ? ", vblock);
	        get_str(buff, sizeof(buff));
	
	        if (strcasecmp(buff,"no") == 0)  
	        {
				PcdHalt();
				return(V_NOT_EMPTY);
			}
	        else if (strcasecmp(buff,"exit") == 0)  
	        {
				PcdHalt();
				close_out(0);
			}
	        else if (strcasecmp(buff,"yes") != 0)
	            p_printf(RED,"Invalid answer. Please type 'yes' or 'no' or 'exit' \n");
	
	    }   while (strcasecmp(buff,"yes") != 0);
		
	}

	// read trailer
	if (read_trailer_block(&t_addr, vblock, vbuff) != TAG_OK) return(TAG_ERR);

	// obtain the access rights
	ch = calc_access_bits(3 -(t_addr-vblock), vbuff);
	
	// if error
	if(ch == 0xff)
	{
        p_printf(RED,"Issue with analysing trailer.\n");
        PcdHalt();
        return(V_ERROR);
    }

	if(ch != 0)
	{
		p_printf(RED,"The block %d has no write permission. (%x) \n", vblock,ch);
		p_printf(YELLOW,"First change access rights with -c option\n");
		PcdHalt();
		return(V_NOT_VALID);
	}
	
	/* create block with value provided. This has to be done before
	 * creating restricted access rights with KeyA*/
	if(set_blck_value(vblock, setvalue) != V_OK) return(V_ERROR);

	p_printf(GREEN,"Setting access rights to level 6 \n");
	
	// update to level 6 on the block
	if (update_access_bits_on_card('6', vblock) != TAG_OK) return (TAG_ERR);
		
	p_printf(GREEN,"Value block %d has been created with value %06.0f (decimal)\n", vblock,setvalue);
	
	PcdHalt();
	return(V_OK);
}

/* calculate value from valid value block content 
 * vbuff : input content from value block
 * val : return the extracted value
 * 
 * return code
 * V_OK : ok
 * V_NOT_VALID : invalid value block
 */
int extract_value(uint8_t *vbuff, double *val)
{
	// check this valid value block
	if (validate_value_blck(vbuff) != V_OK)
	{
		p_printf(RED,"Block is an invalid value block.\n");
		PcdHalt();
		return(V_NOT_VALID);
	}
	
	// extract value
	*val = (long long) vbuff[0];
	*val = (long long) *val | ((long long) vbuff[1]) << 8;
	*val = (long long) *val | ((long long) vbuff[2]) << 16;
	*val = (long long) *val | ((long long) vbuff[3]) << 24;
	
	return(V_OK);
}

/* show content of value block 
 * block : pointer to store the selected block that was shown
 * value : pointer to store value
 * halt : 1 = halt card after showing, 0 = do NOT halt card
 * 
 * return code:
 * V_OK = ok
 * V_ERROR  = error during obtaining block and content
 * V_NOT_VALID = invalid value block
 */
int show_value_block(int *block, double * value, int halt)
{
	uint8_t vbuff[MAXRLEN];
	int 	vblock;
	double  val=0;
		
	p_printf(BLUE, "Show value on block.\n");
	
	// get value block content
	if (get_value_block(&vblock, vbuff)) return(V_ERROR);
	
	if (extract_value(vbuff, &val) != V_OK) return(V_NOT_VALID);
		
	// display value block
	p_printf(YELLOW,"Value block %d has a value of %06.0f\n", vblock, val);
	
	// return block value
	*block = vblock;
	*value = val;
	
	if (halt) PcdHalt();
	
	return(V_OK);
}

/* remove a valid value block and reset to 0x0 
 * 
 * return code :
 * V_OK : ok
 * V_ERROR : general error
 */
int remove_value_block()
{
	uint8_t	vbuff[MAXRLEN];
	int		vblock;
	char	buff[20];
	double	value;
	
	p_printf(BLUE, "Remove value block.\n");
	
	if (show_value_block(&vblock,&value,0) != V_OK) 	return(V_ERROR);
	
	// validate action
    do
    {
        p_printf(YELLOW,"\nAre you sure to remove this block %d (yes, no or exit) ? ", vblock);
        get_str(buff,sizeof(buff));

        if (strcasecmp(buff,"no") == 0)  
        {
			PcdHalt();
			return(V_OK);
		}
        else if (strcasecmp(buff,"exit") == 0)  
        {
			PcdHalt();
			close_out(0);
		}
        else if (strcasecmp(buff,"yes") != 0)
            p_printf(RED,"Invalid answer. Please type 'yes' or 'no' or 'exit' \n");

    }   while (strcasecmp(buff,"yes") != 0);	
	
	// clear all bytes
	memset(vbuff,0x0,sizeof(vbuff));

	// reset access rights to shipping (0)
	if (update_access_bits_on_card('0', vblock) != TAG_OK) return (V_ERROR);
		
	// write to card
	if (value_to_card(vblock, vbuff) != V_OK)	return(V_ERROR);
		
	p_printf(GREEN, "Remove / reset has been completed.\n");
	
	PcdHalt();
	return(V_OK);
}

/* change block-value
 * action : increment, decrement or restore
 * setvalue : value to use for increment or decrement 
 * 
 * Although restore action is supported, one would normally do this after 
 * the increment or decrement command AND before transfer. This call 
 * handles both after each other in a single call. This reduces the chance
 * of error and reading from one value block and transfer in another. 
 * If you like you however... you could take the transfer command out and 
 * make that a seperate call. I would say though:  Keep it simple !!
 *  
 * return code
 * V_OK : ok
 * V_NOT_VALID : incorrect value block content
 * V_ERROR : general error
 */
int change_block_value(uint8_t action, double setvalue)
{
	uint8_t 	vbuff[MAXRLEN];
	int 		vblock;
	double 		cval,aval;
	int 		tmp;
	
	p_printf(BLUE, "update value block.\n");
	
	// get value block content
	if (get_value_block(&vblock, vbuff)) return(V_ERROR);

	// get current value
	if (extract_value(vbuff, &cval) != V_OK) return(V_NOT_VALID);	
	
	p_printf(GREEN, "current value on block %d is %06.0f\n",vblock, cval);
	
	// get value increment/decrement
	if (setvalue != 0)	aval = setvalue;
	
	else if (action == PICC_DECREMENT || action == PICC_INCREMENT)
	{
		do
		{
			if (action == PICC_DECREMENT)
				p_printf(YELLOW, "Provide value to decrement : ");
			
			else if (action == PICC_INCREMENT)
				p_printf(YELLOW, "Provide value to increment : ");
			
			scanf("%lf", &aval);
		
			// flush any input pending
			while ((tmp = getchar()) != '\n' && tmp != EOF);
			
		} while(aval == 0);
	}
	else if (action == PICC_RESTORE)
		aval = 0;
	
	// check there is enough to decrement 
	if (action == PICC_DECREMENT)
	{
		if (cval - aval < 0)
		{
			p_printf(RED, "Not enough to decrement %06.0f.\n",aval);
			PcdHalt();
			return(V_NOT_VALID);
		}
	}
	
	// perform increment, decrement or restore
	if (PcdValue(action, (uint8_t) vblock, aval) != TAG_OK)
	{
		p_printf(RED,"Error during command (increment/decrement/restore)\n");
		PcdHalt();
		return(V_ERROR);
	}
	
	//  Transfer
	if (PcdValue(PICC_TRANSFER, (uint8_t) vblock, 0) != TAG_OK)
	{
		p_printf(RED,"Error during transfer\n");
		PcdHalt();
		return(V_ERROR);
	}
	
	if (action == PICC_DECREMENT)
		p_printf(GREEN,"Value has been adjusted to %06.0f\n",cval - aval);
	else if (action == PICC_INCREMENT)
		p_printf(GREEN,"Value has been adjusted to %06.0f\n",cval + aval);

	PcdHalt();
	return(V_OK);
}
/* increment value block on the card */
int increment_value_block(double setvalue)
{
	return(change_block_value(PICC_INCREMENT, setvalue));
}

/* decrement value block on the card */
int decrement_value_block(double setvalue)
{
	return(change_block_value(PICC_DECREMENT,setvalue));
}
