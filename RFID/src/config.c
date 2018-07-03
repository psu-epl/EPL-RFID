/*
 * config.c
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
 * version 1.1 / paulvh / April 2016
 * fix in read_conf_key as sscanf expects int* and not uint8_t (char *) for %x
 *
 * version 1.51 / paulvh / July 2017
 * Fixed number of bugs and code clean-up.
 * 
 */

#include "config.h"

char config_file[255]=default_config_file;
FILE *fdconfig=NULL;
char str[255];

// needed for authentication
uint8_t KEYA[6];
uint8_t KEYB[6];

// default authentication values
uint8_t KEYA_def[6]= {0xFF,0XFF,0XFF,0XFF,0XFF,0XFF};
uint8_t KEYB_def[6]= {0xFF,0XFF,0XFF,0XFF,0XFF,0XFF};


// obtain the User and group ID for potential child program to execute
int read_conf_uid() 
{
    char user[5];
    uid_t uid;
    gid_t gid;

    if (find_config_param("UID=",user,sizeof(user))<=0) {

        if  (getuid() < 100) 
        {
            printf("UID must be set!\n");
            return -1;
        }
    }
    else
    {
        uid=(int)strtol(user,NULL,10);

        if (uid < 100) 
        {
            printf("Invalid UID: %s\n",user);
            return -1;
        }
        setuid(uid);
    }
    
    if (find_config_param("GID=",user,sizeof(user)) == 1) 
    {
        gid=(int)strtol(user,NULL,10);
        setgid(gid);
    }

    return 0;
}
/*
 * Read KeyA or KeyB for a specific sector on a specific card
 * if not found default KeyA or KeyB will be used
 *
 * Format:
 * Header = [xxy{
 *      xx =  Sectornumber (between 00 - 15)
 *       y =  A or B (whether KEYA or KEYB)
 * card serialnumber (* = applies to all cards, otherwise specific card)
 * seperator = }
 * key (6 bytes seperated by space)
 * Trailer = ]
 * example : [09A{c319eba4}aa bb cc dd ee 09]
 */

int read_conf_key(int addr, int auth_key)
{
    char    s_key[10];          // search key
    char    result[50];         // search result
    char    conv[25];           // needed for conversion
    char    *p,*pp;
    int     tmp;
    int     sector=0;           // sector 0 -15
    int     keyfound=0;         // catch a match
    int     t_key[6];           // fix for sscanf // version 1.1

    // determine sector
	sector = addr/4;

	if (auth_key == PICC_AUTHENT1A)
	{
		// set default key
		for (tmp=0; tmp < 6; tmp++) KEYA[tmp] = KEYA_def[tmp];
		
		// create search key
		sprintf(s_key,"[%02dA{",sector);
	}
	else
	{
		// set default key
		for (tmp=0; tmp < 6; tmp++) KEYB[tmp] = KEYB_def[tmp];
        
        // create search key   
		sprintf(s_key,"[%02dB{",sector);
	}
    
    // try to search key for sector
    if (find_config_param(s_key,result,sizeof(result)))
    {
        // search key was found. 
        if (result[0] == '*'){      // for all cards with this sector address 
            keyfound = 1;
            p = &result[1];         // skip *
        }
        else  // try exact serial number match
        {
            // determine length of serial number
            for (tmp = 0; tmp < 20; tmp++) 
            {
                if (result[tmp] == '}')	// end serial number
                {        
                    p = &result[tmp];
                    break;
                }
			}
	
			// same length?
            if (tmp == SN_len * 2) 
            {          
				pp=conv;
				
				// conv serial number to ascii
				for (tmp=0;tmp<SN_len;tmp++) {  
					sprintf(pp,"%02x",SN[tmp]);
					pp+=2;
				}
				
				// check for match of serial numbers
				for (tmp = 0; tmp < (SN_len*2) ; tmp++) 
				{

					if (conv[tmp] != result[tmp])
						break;			
				}
				
				// if match set for keyfound
                if (tmp == SN_len *2) keyfound=1;
			}
        }

        if (debug)
        {
            if (keyfound) p_printf(GREEN, "Match found for KEY, sector %d\n",sector);
            else p_printf(YELLOW, "Using default key for sector %d\n",sector);
		}
		
        if (keyfound){                  // match found

            // check seperator for correct syntax
            if (*(p++) != '}') 
            {
				p_printf(RED,"invalid syntax for sector authorisation key\n");
				return(1);
			}
			
			// copy key (max 12 + 5 spaces)
            for (tmp=0; tmp < 20; tmp++){

                if (*(p) == ']'){       // end of authorisation KEY ?

                    if (conv[tmp-1] == 0x20)	// remove trailing space
                        tmp = tmp -1;

                    conv[tmp] = 0x0;	// terminate
                    break;
                }

                conv[tmp] = *(p++);
            }

            if (tmp == 17)              // version 1.1
            {
                sscanf(conv,"%x %x %x %x %x %x",&t_key[0],&t_key[1],&t_key[2],&t_key[3],&t_key[4],&t_key[5]);

                // set new authorisation key
                for (tmp=0; tmp<6; tmp++){

                    if (auth_key == PICC_AUTHENT1A) KEYA[tmp] = (uint8_t) t_key[tmp];
                    else KEYB[tmp] = (uint8_t) t_key[tmp];
                }
            }
            else
            {
                p_printf(RED, "Invalid key: not correct length %d => %s.\n",tmp,conv);
                return(1);
            }
        }

        if (debug)
        {
            if (auth_key == PICC_AUTHENT1A)
            { 
				p_printf(GREEN,"Key A set to: ");
				for (tmp=0; tmp < 6; tmp++)  p_printf(YELLOW,"0x%x ",KEYA[tmp]);
            }
            else 
            {
				p_printf(GREEN, "Key B set to: ");
				for (tmp=0; tmp < 6; tmp++)  p_printf(YELLOW,"0x%x ",KEYB[tmp]);
			}
            
            p_printf(WHITE,"\n");
        }
    }
    else if (debug) 
		p_printf(GREEN,"Using default authentication key.\n");

    return(0);
}

/* opens the configuration file
 * return 0 for OK , -1 for error
 */
int open_config_file(char * conffile) 
{
    if (fdconfig == NULL) {

        if (access(conffile,W_OK)!=0) return -1;

        if ((fdconfig = fopen(conffile,"r+")) == NULL) return -1;
    }
    
    return 0;
}

/* close open config file */
void close_config_file() 
{
	if (fdconfig != NULL)   fclose(fdconfig);

	fdconfig = NULL;
}

/* find a specific parameter in the config file
 * param_name is search value
 * param_val holds the return value (if found)
 * val_len = max length of param_val allowed
 */
int find_config_param(char * param_name, char * param_val, int val_len) 
{
    int param_found=0,opened=0;
    char * pstr;

	
	if (fdconfig == NULL)
	{
		if (open_config_file(config_file) != 0)
		{
			p_printf(RED,"Config file can not be opened\n");
			return(0);
		}
		
		opened = 1;
	}
	
	// go to beginning of file
    if (fseek(fdconfig, 0L, SEEK_SET)!= 0) return -1;

	// get next line
    while (fgets(str,sizeof(str)-1,fdconfig)!= NULL) {

        // skip comment lines (terminate line on the position for the #)
        if ((pstr = strchr(str,'#')) != NULL) *pstr=0;

		// find match to search value
        if ((pstr = strstr(str,param_name)) != NULL) 
        {
            param_found=1;
            
			// to first point beyond search value. 
            pstr+=strlen(param_name);
            
            // skip leading spaces
            while (isspace(*pstr)) pstr++;
            
            // terminate on trailing spaces as 0x0
            while (isspace(pstr[strlen(pstr)-1])) pstr[strlen(pstr)-1]=0;
            
            // copy the value found to maximum length
            strncpy(param_val,pstr,val_len);

            break;
        }
    }
    
    // close if opened during this call
	if (opened) close_config_file();
    
    return param_found;
}

/* Add null terminate string as line to end of config file, newline will be add */

int add_to_config(char *buf)
{
    int status = 0, opened = 0;

	// if config file is closed
	if (fdconfig == NULL)
	{
		if (open_config_file(config_file) != 0) {
			p_printf(RED,"Can't open config file! (need read/write permissions)\n");
			return(1);
		}
		
		opened = 1;	
	}
	
    // go to end of file
    if (fseek(fdconfig, 0L, SEEK_END)!= 0) return (1);

    // write buffer
    status = fputs(buf,fdconfig);
    if (status == EOF)  return(1);

    // write newline to buffer
    status = fputs("\n",fdconfig);
    if (status == EOF)  return(1);

    // flush
    status = fflush(fdconfig);

	// close if opened
	if(opened) close_config_file();
    
    return(0);
}
