//#include <wiringPi.h>

char VERSION[] = "1.65.3";
#include "main.h"
#include <memory.h>
#define SIZE 200

//#define LED 0
// command line options
uint8_t debug=0;			// trigger for debug messages
int	NoColor = 0;		// disables color output (if set)
int UpdateTrailer = 0;	// if set, will allow to update access permission trailer
int use_vblock= 0xff;	// block if provided (e.g. on command line)

// initialize values
uint8_t use_gpio=0;         // was valid GPIO for reset ?
uint8_t gpio=255;           // GPIO for hard-reset
uint8_t loop = 10, NotEndless = 0; // main loop
uint32_t spi_speed = 1000L; // speed for SP (4 < >125) overruled by config file value
char  fmem_path[255];     // to hold the directory for memory save (from config file)
char  save_mem=0;         // trigger (or disable if issues in config file) to save card info

// card information (globally used)
uint8_t SN[10];             // UID / serial number of card
uint16_t CType=0;           // card type/ ATAQ response
uint8_t SN_len=0;           // length of UID / serial  number ( 4 / 7 / 10)

uint8_t SAK[3];				// SAK (select acknowledgement)
int max_blocks=64;    	// # of block on a card (default is classic = 64)
uint8_t page_step=0;        // increment to read the card

char card_str[25];

struct string {
  char *ptr;
  size_t len;
};

int init_13()
{
  uint8_t gpio=255;           // GPIO for hard-reset
  uint32_t spi_speed = 1000L; // speed for SP (4 < >125) overruled by config file value

  // must be run as root to open /dev/mem in BMC2835
  if (getuid() != 0)
  {
      p_printf(RED, "Must be run as root.\n");
      exit(1);
  }

  Led_On();

  // catch signals
  set_signals();

  /* read /etc/rc522.conf */
  if (get_config_file()) exit(1);

  /* set BCM2835 Pins correct */
  if (HW_init(spi_speed,gpio)) close_out(1);

  /* initialise the RC522 */
  InitRc522();

  /* read & set GID and UID from config file */
  if (read_conf_uid()!= 0) close_out(1);

  digitalWrite(3, 0);

}

int Led_On()
{
  if(wiringPiSetup() == -1)
  {
	  printf("setup wiringPi failed !\n");
	  return -1;
  }
  printf("LED On\n");
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
	digitalWrite(3, HIGH);
	digitalWrite(4, HIGH);
  bcm2835_delay(500);
	digitalWrite(3, LOW);
	digitalWrite(4, LOW);
  bcm2835_delay(500);
  return 0;
}

/* Display in color
 * @param format : Message to display and optional arguments
 *                same as printf
 * @param level :  1 = RED, 2 = GREEN, 3 = YELLOW 4 = BLUE 5 = WHITE
 *
 *  if NoColor was set, output is always WHITE.
 */
void p_printf (int level, char *format, ...)
{
  char* col;
  int	coll=level;
  va_list arg;

  //allocate memory
  col = malloc(strlen(format) + 20);

  if (NoColor) coll = WHITE;

  switch(coll)
  {
	case RED:
		sprintf(col,REDSTR, format);
		break;
	case GREEN:
		sprintf(col,GRNSTR, format);
		break;
	case YELLOW:
		sprintf(col,YLWSTR, format);
		break;
	case BLUE:
		sprintf(col,BLUSTR, format);
		break;
	default:
		sprintf(col,"%s",format);
  }

  va_start (arg, format);
	vfprintf (stdout, col, arg);
	va_end (arg);

	fflush(stdout);

  // release memory
  free(col);
}


/* perform init BCM and GPIO */
uint8_t HW_init(uint32_t spi_speed, uint8_t gpio)
{
  if (!bcm2835_init())
  {
    p_printf(RED,"Can not initialise for BCM2835 access\n");
    exit(1);
  }

  // HARD reset RC522 (only with valid gpio from config)
  if (use_gpio)
  {
    bcm2835_gpio_fsel(gpio, BCM2835_GPIO_FSEL_OUTP);

    // NRSTPD = low is OUT/ power down (PD)]
    bcm2835_gpio_write(gpio, LOW);
    bcm2835_delay(500);

    // NRSTPD = positve edge is reset + run
    bcm2835_gpio_write(gpio, HIGH);
    bcm2835_delay(500);
  }
  else
  {
	  p_printf(RED,"GPIO in config file must be lower than 28\n");
	  exit(1);
  }

  bcm2835_spi_begin();    // set pins 19, 21, 23, 24, 26

  // most significant bit is sent first
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);

  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default

  bcm2835_spi_setClockDivider((uint16_t)(250000L/spi_speed));	  // set speed
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // using SPI0
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      //
  return(0);
}

/* reset and terminate program */
void close_out(int ex_code)
{
  if (use_gpio) bcm2835_gpio_write(gpio, LOW); // set reset
  bcm2835_spi_end();
  bcm2835_close();
  close_config_file();
  exit(ex_code);
}

void usage(char * str)
{
  p_printf(GREEN,"Will only handle MIFARE Crypto-1 (Classic or plus cards)\n\n");

  p_printf(YELLOW, "Usage:\n%s [options]:           (version %s)\n"
  "\t  -a  perform action based on content specific block\n"
  "\t  -b  blocknumber to use, instead of asked by the program\n"
  "\t  -c  change access rights for specific block\n"
  "\t  -d  enable debug mode\n"
  "\t  -e  extra delay between card-access calls for slow cards\n"
  "\t  -h  this help text\n"
  "\t  -k  change authentication key on a specific sector\n"
  "\t  -n  no color in messages\n"
  "\t  -p  perform action on card number (specified in configuration file)\n"
  "\t  -r  read a card block\n"
  "\t  -s  save content all card blocks in a file\n"
  "\t  -t  combined with '-c' allows update trailer access rights\n"
  "\t      (USE WITH CARE AND ONLY IF YOU REALLY KNOW WHAT YOU ARE DOING)\n"
  "\t  -w  write to card block\n",str,VERSION);

  p_printf(GREEN,"\nFOLLOWING IS FOR VALUE BLOCK HANDLING ONLY:\n");

  p_printf(YELLOW,
	"\t  -C  Create a value block\n"
	"\t  -D  Decrement a value block\n"
	"\t  -I  Increment a value block\n"
	"\t  -R  Remove a value block\n"
	"\t  -S  Show a value block\n"
	"\t  -V#  Value to use with value block\n");

	p_printf(GREEN,"\nFOLLOWING IS FOR WRITE & READ FROM COMMAND LINE ONLY:\n");

	p_printf(YELLOW,
	"\t  -N# Starting Sector (default = 1)\n"
	"\t  -B# Starting block (default = 0)\n"
	"\t      OR\n"
	"\t  -A# Starting address (default = 4)\n\n"
	"\t  -M  Message with characters to write\n"
	"\t      OR\n"
	"\t  -H  Message with Hex-bytes to write\n\n"
	"\t      OR\n"
	"\t  -G# Get bytes from the starting point, display as hex characters\n"
	"\t  -T# same as -G, but display as (readable) characters\n");
}

/* calculate the access bits for a block
 * access: block number
 * buff  : trailer information from the card
 *
 * format as specified in the datasheet:
 *
 * C1 C2 C3 (be aware C3 = bit 0 !!!)
 *
 * return code:
 * OK :    block access bits (0 -7)
 * error : 0xff
 */
int calc_access_bits(int access, uint8_t *buff)
{
	int	tmp;
							// offset  -------------- access bits --------------------
                               // C1                      C2                           C3
  if (access == 0)         tmp = (buff[7] & B0B7C1) >> 2 |(buff[8] & B0B8C2) << 1 |(buff[8] & B0B8C3) >> 4;
  else if (access == 1)    tmp = (buff[7] & B1B7C1) >> 3 |(buff[8] & B1B8C2)      |(buff[8] & B1B8C3) >> 5;
  else if (access == 2)    tmp = (buff[7] & B2B7C1) >> 4 |(buff[8] & B2B8C2) >> 1 |(buff[8] & B2B8C3) >> 6;
  else if (access == 3) 	 tmp = (buff[7] & TB7C1)  >> 5 |(buff[8] & TB8C2)  >> 2 |(buff[8] & TB8C3 ) >> 7;
  else tmp = 0xff;

  return(tmp);
}

/* set the access bit for a block
 * ch : choice 0 - 7 (0x30 - 0x37)
 * block : block number to set ( 0 - 3 )
 * buff : pointer to trailer content as read before
 *
 * returns the updated access bits in the buff.
 *
 * format as specified in the datasheet:
 * C1 C2 C3 (be aware C3 = bit 0 !!!)
 */
void set_access_bits(char ch, int block, uint8_t *buff)
{
  switch(ch)     // update Byte 7 & 8
  {
    case 0x30:
      if (block == 0) {
          buff[7] = buff[7] & ~B0B7C1;
          buff[8] = buff[8] & ~B0B8C2;
          buff[8] = buff[8] & ~B0B8C3;
      }
      else if (block == 1){
          buff[7] = buff[7] & ~B1B7C1;
          buff[8] = buff[8] & ~B1B8C2;
          buff[8] = buff[8] & ~B1B8C3;
      }
      else if (block == 2){
          buff[7] = buff[7] & ~B2B7C1;
          buff[8] = buff[8] & ~B2B8C2;
          buff[8] = buff[8] & ~B2B8C3;
      }
      else if (block == 3){
          buff[7] = buff[7] & ~TB7C1;
          buff[8] = buff[8] & ~TB8C2;
          buff[8] = buff[8] & ~TB8C3;
      }
      break;
    case 0x31:
      if (block == 0) {
          buff[7] = buff[7] & ~B0B7C1;
          buff[8] = buff[8] & ~B0B8C2;
          buff[8] = buff[8] |  B0B8C3;
      }
      else if (block == 1){
          buff[7] = buff[7] & ~B1B7C1;
          buff[8] = buff[8] & ~B1B8C2;
          buff[8] = buff[8] |  B1B8C3;
      }
      else if (block == 2){
          buff[7] = buff[7] & ~B2B7C1;
          buff[8] = buff[8] & ~B2B8C2;
          buff[8] = buff[8] |  B2B8C3;
      }
      else if (block == 3){
          buff[7] = buff[7] & ~TB7C1;
          buff[8] = buff[8] & ~TB8C2;
          buff[8] = buff[8] |  TB8C3;
      }
      break;
    case 0x32:
      if (block == 0) {
          buff[7] = buff[7] & ~B0B7C1;
          buff[8] = buff[8] |  B0B8C2;
          buff[8] = buff[8] & ~B0B8C3;
      }
      else if (block == 1){
          buff[7] = buff[7] & ~B1B7C1;
          buff[8] = buff[8] |  B1B8C2;
          buff[8] = buff[8] & ~B1B8C3;
      }
      else if (block == 2){
          buff[7] = buff[7] & ~B2B7C1;
          buff[8] = buff[8] |  B2B8C2;
          buff[8] = buff[8] & ~B2B8C3;
      }
      else if (block == 3){
          buff[7] = buff[7] & ~TB7C1;
          buff[8] = buff[8] |  TB8C2;
          buff[8] = buff[8] & ~TB8C3;
      }
      break;
    case 0x33:
      if (block == 0) {
          buff[7] = buff[7] & ~B0B7C1;
          buff[8] = buff[8] |  B0B8C2;
          buff[8] = buff[8] |  B0B8C3;
      }
      else if (block == 1){
          buff[7] = buff[7] & ~B1B7C1;
          buff[8] = buff[8] |  B1B8C2;
          buff[8] = buff[8] |  B1B8C3;
      }
      else if (block == 2){
          buff[7] = buff[7] & ~B2B7C1;
          buff[8] = buff[8] |  B2B8C2;
          buff[8] = buff[8] |  B2B8C3;
      }
      else if (block == 3){
          buff[7] = buff[7] & ~TB7C1;
          buff[8] = buff[8] |  TB8C2;
          buff[8] = buff[8] |  TB8C3;
      }
      break;
    case 0x34:
      if (block == 0) {
          buff[7] = buff[7] |  B0B7C1;
          buff[8] = buff[8] & ~B0B8C2;
          buff[8] = buff[8] & ~B0B8C3;
      }
      else if (block == 1){
          buff[7] = buff[7] |  B1B7C1;
          buff[8] = buff[8] & ~B1B8C2;
          buff[8] = buff[8] & ~B1B8C3;
      }
      else if (block == 2){
          buff[7] = buff[7] |  B2B7C1;
          buff[8] = buff[8] & ~B2B8C2;
          buff[8] = buff[8] & ~B2B8C3;
      }
      else if (block == 3){
          buff[7] = buff[7] |  TB7C1;
          buff[8] = buff[8] & ~TB8C2;
          buff[8] = buff[8] & ~TB8C3;
      }
      break;
    case 0x35:
      if (block == 0) {
          buff[7] = buff[7] |  B0B7C1;
          buff[8] = buff[8] & ~B0B8C2;
          buff[8] = buff[8] |  B0B8C3;
      }
      else if (block == 1){
          buff[7] = buff[7] |  B1B7C1;
          buff[8] = buff[8] & ~B1B8C2;
          buff[8] = buff[8] |  B1B8C3;
      }
      else if (block == 2){
          buff[7] = buff[7] |  B2B7C1;
          buff[8] = buff[8] & ~B2B8C2;
          buff[8] = buff[8] |  B2B8C3;
      }
      else if (block == 3){
          buff[7] = buff[7] |  TB7C1;
          buff[8] = buff[8] & ~TB8C2;
          buff[8] = buff[8] |  TB8C3;
      }
      break;
    case 0x36:
      if (block == 0) {
          buff[7] = buff[7] |  B0B7C1;
          buff[8] = buff[8] |  B0B8C2;
          buff[8] = buff[8] & ~B0B8C3;
      }
      else if (block == 1){
          buff[7] = buff[7] |  B1B7C1;
          buff[8] = buff[8] |  B1B8C2;
          buff[8] = buff[8] & ~B1B8C3;
      }
      else if (block == 2){
          buff[7] = buff[7] |  B2B7C1;
          buff[8] = buff[8] |  B2B8C2;
          buff[8] = buff[8] & ~B2B8C3;
      }
      else if (block == 3){
          buff[7] = buff[7] |  TB7C1;
          buff[8] = buff[8] |  TB8C2;
          buff[8] = buff[8] & ~TB8C3;
      }
      break;
    case 0x37:
      if (block == 0) {
          buff[7] = buff[7] | B0B7C1;
          buff[8] = buff[8] | B0B8C2;
          buff[8] = buff[8] | B0B8C3;
      }
      else if (block == 1){
          buff[7] = buff[7] | B1B7C1;
          buff[8] = buff[8] | B1B8C2;
          buff[8] = buff[8] | B1B8C3;
      }
      else if (block == 2){
          buff[7] = buff[7] | B2B7C1;
          buff[8] = buff[8] | B2B8C2;
          buff[8] = buff[8] | B2B8C3;
      }
      else if (block == 3){
          buff[7] = buff[7] | TB7C1;
          buff[8] = buff[8] | TB8C2;
          buff[8] = buff[8] | TB8C3;
      }
      break;
  }

  // calc inverted in BUF 6 and 7
  buff[6] = buff[7] >> 4;             // C1
  buff[6] = buff[6] | (buff[8] << 4); // C2
  buff[6] = ~buff[6];                 // invert

  buff[7] = buff[7] & 0xf0;
  buff[7] = buff[7] | ((~buff[8] >> 4) & 0x0f);   //~c3
}


/* get block number
 * check whether a valid was provided on the command line with -B option
 * blocks : maximum blocknunber
 * chk_trail : check for trailer (1) or not (0)
 */

int get_block_number(int blocks, int chk_trail)
{
	int blocknr;

	do
	{
		// was provided on the command line ?
		if (use_vblock != 0xff)
		{
			blocknr = use_vblock;
			p_printf(GREEN,"%d\n", blocknr);
		}

		else
		{
			blocknr = get_number();
		}

        if (blocknr == 0)  return(0);

		if (blocknr > blocks)
		{
			p_printf(RED,"Block number is to high %2d. Maximum %d\n",blocknr, blocks);
			blocknr = -1;
		}

		else if (blocknr < 0)
		{
			p_printf(RED,"Block can't be below 0.\n");
			blocknr = -1;
		}

        // check for trailer ?
		else if (chk_trail)
		{
			if ((blocknr == 3) || (! ((blocknr-3) % 4)))
			{
				p_printf(RED,"Can not use a trailer location.\n");
				blocknr = -1;
			}
		}

		if (blocknr == -1)
		{
			p_printf(YELLOW,"Try again\n");
			use_vblock = 0xff; // reset potential command line provided block
		}

	} while (blocknr == -1);

	return(blocknr);
}

/* get single digital number */
int	get_number()
{
	int 	num,ch;

	scanf("%d", &num);
	
    // flush any input pending
    while ((ch=getchar()) != '\n' && ch != EOF);
    
    return(num);
}

/* get single (uppercase) character */
char	get_charc()
{
	char 	charc; 
	int		ch;
	
	scanf("%c", &charc);

    // flush any input pending
    while ((ch=getchar()) != '\n' && ch != EOF);
    
    return(toupper(charc));
}

/* get string 
 * buf : ponter to store string
 * len : maximum len of string
 */
void	get_str(char * buf, int len)
{
	// clear memory
	memset(buf,0,len);
	
	// get input
	fgets(buf, len, stdin);

	// terminate with 0 instead of \n
	buf[strlen(buf) -1 ] = 0;
}

/* update security keyA or B on specific sector */
int key_upd()
{
    int             addr = -1, t_addr;
    int             tmp, tmp1, sct_num = -1, new_key[6];
    unsigned int	i=0;
    unsigned char   ch, key = 'c', buff[50];
    char            buf[50], *p;
           
    do
    {
        p_printf (YELLOW,"Do you want to provide s = sector or b = block number for key-update (e = exit) ");
        ch = get_charc();
		
		// check response
        if (ch == 'E')  close_out(0);
        else if (ch == 'S') i = 1;
        else if (ch == 'B') i = 2;
        else p_printf(RED,"Invalid entry  'e', 's' or 'b'\n");

    } while (i == 0);

    if (i == 2)
    {
		p_printf (YELLOW,"Provide block where to update sector key.\n");
        p_printf (YELLOW,"(0 = return) : ");
        if ((addr = get_block_number(0xff,0)) == 0) return(0);

        // determine sct_num
        sct_num=addr/4;
    }
    else
    {
        do    // get sector
        {
            p_printf (YELLOW,"Please provide the sector to update key.\n");
            p_printf (YELLOW,"Max %d.  (0 = return) : ", max_blocks/4);
            sct_num=get_number();

            if (sct_num == 0)  return(0);

            if ((sct_num < 0) || (sct_num > max_blocks/4))
            {
                p_printf(RED,"Invalid sector-number. Please try again.\n");
                sct_num = -1;
            }

        } while (sct_num == -1);
        
        addr = sct_num * 4;
     }

    do    // get key A or B
    {
        p_printf (YELLOW,"Which key do you want to update A = KEYA or B = KEYB (E = return) : ");
        key=get_charc();

        if (key == 'E') return(0);

        if ((key != 'A') &&  (key != 'B'))
            p_printf(RED,"Invalid key. Please try again.\n");

    } while ((key != 'A') &&  (key != 'B'));

    // get the new key
    for (i=0 ; i < 6; i++)
    {
        do
        {
            p_printf(YELLOW,"Provide authentication byte %2d of 6 : 0x",i+1);

            tmp = scanf("%x", &new_key[i]);

            if (tmp == 0) p_printf(RED,"Invalid entry. Try again.\n");

            // flush any input pending
            while ((tmp1 = getchar()) != '\n' && tmp1 != EOF);

        } while (tmp == 0);

    }
    
    // get confirmation, show block range
    i = sct_num * 4;
    
    p_printf(RED,"******** IMPORTANT TO DOUBLE CHECK ***************\n"
    "You are about to make a change to KEY%c:\n"
    "Sector %d, with the block range of %d - %d\n",key, sct_num, i,i+3);


    for (i=0 ; i < 6; i++)  p_printf(YELLOW,"0x%02x ",new_key[i]);
    
    i=0;
    do
    {
        p_printf(YELLOW,"\nIs the correct (yes, no or exit) ? ");
        get_str(buf, sizeof(buf));

        if (strcasecmp(buf,"no") == 0)
            return(0);

        else if (strcasecmp(buf,"yes") == 0)
             i=1;

        else if (strcasecmp(buf,"exit") == 0)
             close_out(0);

        else
            p_printf(RED,"Invalid answer. Please type 'yes' or 'no' or 'exit' \n");

    }   while (i == 0);

    // wait for card
    if (get_card_info() != TAG_OK)    return(1);

    p_printf(RED,
    "**********************************************\n"
    "  DO NOT REMOVE YOUR CARD FROM THE READER !!\n"
    "**********************************************\n\n");

    // check block fit
    if (addr >= max_blocks) {
        p_printf(RED,"This card has only %d blocks. Can not read from %d.\n",max_blocks-1, addr);
        PcdHalt();
        return(0);
    }
	
	// determine access rights : access trailer
	if (read_trailer_block(&t_addr, addr, buff) != TAG_OK) return(TAG_ERR);

    /* as we authenticate with ONE key and if we do NOT want to update the other, we have
     * to get the current valid other key for this sector first to write back.
     * 
     * Depending on the access BITS, KEYA might be able to read KEYB, but that is not a given. 
     * 
     * Under NO condition the KEYA can be read with the Trailer (always zero) 
     * 
     * Although KEYB is always shown when reading, just to be sure we try to obtain KEYB 
     * (either using default or as stored in the config file) to prevent access issues later.
     * 	
     */

    if (key == 'A')
    {
        
        if (read_conf_key(t_addr,PICC_AUTHENT1B) != 0){
            p_printf(RED,"Error during obtaining current KEYB for sector %d\n",sct_num);
            return(1);
        }
    }

    // add new key
    for (i = 0; i < sizeof(KEYA); i++)
    {
		if (key == 'A') 
		{
			buff[i] = new_key[i];
			buff[i+10] = KEYB[i];
		
		}
		else
		{
			buff[i] = KEYA[i];			// we have to write KEYA (always reads as zero's)
			buff[i+10] = new_key[i];
		}
	}
	
    if (debug)
    {
        p_printf(YELLOW, "New trailer to be written : ");
        for (i = 0;i < 16; i++)  p_printf(YELLOW,"%02x",buff[i]);
        p_printf(WHITE,"\n");
    }

    // update trailer
    if (PcdWrite(t_addr, buff) != TAG_OK)
    {
        p_printf(RED,"Error during update trailer.\n");
        PcdHalt();
        return(1);
    }

    // read back to double check
    if (debug)
    {   
		if (PcdRead(t_addr,buff) == TAG_OK)
			p_printf(YELLOW, "Trailer %d has been updated to the following content %s\n",t_addr, buff);
		else
			p_printf(RED,"Error during reading trailer after write\n");
    }

    // close / disconnect the card
    PcdHalt();
    
    p_printf(GREEN,
    "KEY%c has been updated. You can remove the card\n"
	"Make sure to write down the new code KEY%c : ",key, key);

    for (i = 0 ; i < 6; i++)  p_printf(YELLOW,"%2x ",new_key[i]);
    
    p_printf(RED,"\nOtherwise access to this sector is lost and can not be recovered\n");

	// try to add an entry to bottom of configuration file
	
    p=buf;

    // create search key
    *(p++) = '#';               // set as comment
    *(p++) = '[';               // start with [
    sprintf(p,"%02d",sct_num);  // add sector
    p += 2;
    *(p++) = key;               // add key
    *(p++) = '{';               // add {

    // serial number
    for(i = 0; i < SN_len;i++)
    {
        sprintf(p,"%02x",SN[i]);
        p += 2;
    }
    *(p++) = '}';               // add separator

    for(i = 0; i < 6;i++)       // add new key
    {
        sprintf(p,"%02x ",new_key[i]);
        p += 3;
    }
    p--;
    *(p++) = ']';               // add trailer
    *(p++) = 0;

    if (add_to_config(buf)){
        p_printf(RED,"Error during adding to config file %s: %s\n",config_file, buf);
       
        p_printf(RED,"Write down this code and add manually (without leading #)\n");
        return(1);
    }

    p_printf(GREEN,"An entry %s has been added to the config file %s\n", buf, config_file);
    p_printf(YELLOW,"It is the last line and you need to remove the '#' manually to make it valid.\n");
    return(0);
}

/* Get authorization to access memory
 * snr = buffer with serial number
 * block = block to get access to
 * auth_key = use KEYA (PICC_AUTHENT1A) or KEYB (PICC_AUTHENT1B)
 */

int authorize(uint8_t *pSnr, int block, int auth_key)
{
    int 	status=0, i, get_key = 1, sct_num=0;
    uint8_t *p;

	// need to prevent repeat authentication key check
	static int prev_sct_num=0xff;
	static int prev_auth_key=0xff;
	static uint8_t prev_sn[10];

    // determine sct_num
    sct_num = block/4;

    // do we need to retrieve the key again ?
    if (sct_num == prev_sct_num)            // same block before ?
    {
        if (auth_key == prev_auth_key)    	// for same key (A or B) ?
        {
            
            for (i=0; i<SN_len; i++)		// same card serial number?
            {
                if (prev_sn[i] != pSnr[i])
                        break;
			}
			
            if (i == SN_len) get_key = 0;	// if all true no need to reload
        }
    }
    //printf("%s :get key %d, sct_num %d, block %d\n",__func__, get_key,sct_num,block);
   
    // if key was NOT obtained before
    if (get_key)
    {
            status = read_conf_key(block, auth_key);

            if (status) {
                p_printf(RED,"Issue with obtaining authentication key !!!! \n");
                return(status);
            }
            else       // save for later check
            {
                prev_sct_num = sct_num;
                prev_auth_key = auth_key;
                p=pSnr;
                for (i=0;i <SN_len;i++)
                    prev_sn[i]= *(p++);
            }
    }

    if (auth_key == PICC_AUTHENT1A)
        status = PcdAuthState(PICC_AUTHENT1A,block,KEYA,pSnr);

    else
        status = PcdAuthState(PICC_AUTHENT1B,block,KEYB,pSnr);

    if (status != TAG_OK)
    {
        p_printf(RED,"Authentification error !!!! \n");
        prev_auth_key = 0xff;      // force recapture of key next time
    }
    
    return(status);
}

/* read the config file information 
 * This is performed once at the beginning of the program
 */

int get_config_file()
{
    char    str[255]; 
    
    if (open_config_file(config_file)!=0) 
    {
        p_printf(RED,"Can't open config file! (need read/write permissions)\n");
        return(1);
    }
    
    // GPIO is expected to be less than 28 (for different PI-models)
    if (find_config_param("GPIO=",str,sizeof(str)-1)==1) 
    {
        gpio=(uint8_t)strtol(str,NULL,10);

        if (gpio < 28)  use_gpio=1;
    }

    if (find_config_param("SPI_SPEED=",str,sizeof(str)-1)==1) {
        spi_speed = (uint32_t) strtoul(str,NULL,10);
        if (spi_speed > 125000L) spi_speed=125000L;
        if (spi_speed < 4) spi_speed=4;
    }

    if (find_config_param("NEW_TAG_PATH=",fmem_path,sizeof(fmem_path)-1)) 
    {
        if (fmem_path[strlen(fmem_path)-1]!='/')
            sprintf(&fmem_path[strlen(fmem_path)],"/");

        if (strlen(fmem_path)>=240) {
            p_printf(RED,"Too long path for tag dump files!\n can not save to file");
            save_mem = 0;
            return(1);
         }
    }
    else     // if NEW_TAG_PATH was not found
    {
		// if requested to save memory to file from command line
		if (save_mem)
		{
			p_printf(RED,"Can not save card content due to missing 'NEW_TAG_PATH' in config file\n");
			save_mem = 0;
		}
	}

    if (find_config_param("LOOP=",str,sizeof(str)-1)==1) {
        loop=(uint8_t)strtol(str,NULL,10);

        if (loop == 0)
        {      
			loop = 1;               // 1.12 check for zero !
			NotEndless = 0;         // Endless
		}
    }
    
    return(0);
}

/* wait and read card permission for block
 * addr : block to get access bits for  
 * read_card: 1 >= read card, 0 >= 0 do not (has been done already 
 * 
 * return
 * OK : block permission ( 0 - 7)
 * ERROR : 0xff
 */
int get_card_permission(int block, int read_card)
{
    int             t_addr;
    unsigned char   ch;
    uint8_t         buff[20];

    // wait for card
    if (read_card)
        if (get_card_info() != TAG_OK)    return(0xff);

    // check block fit
    if (block >= max_blocks) 
    {
        p_printf(RED,"This card has only %d blocks. Can not access %d.\n",max_blocks-1, block);
        PcdHalt();
        return(0xff);
    }

	// determine access rights : access trailer
	if (read_trailer_block(&t_addr, block, buff) != TAG_OK) return(0xff);
    
    ch = calc_access_bits(3 -(t_addr-block), buff);
    
    if(ch == 0xff)
	{
        p_printf(RED,"Issue with analysing trailer.\n");
        PcdHalt();
    }
    
    return(ch);
}

/* perform action based on UID of card */
int uid_action()
{
	int  	tmp;
	char    *p, sn_str[23];         // to hold [serial] as a string
	
	// wait for card
	while (get_card_info() != TAG_OK) usleep(5000);

	// create serial string
	p=sn_str;                       
	*(p++)='[';                     // start with [
	for (tmp=0;tmp<SN_len;tmp++) {  // add serial number
		sprintf(p,"%02x",SN[tmp]);
		p+=2;
	}
	*(p++)=']';                     // close out with ]
	*(p++)=0;

	//for debugging
	if (debug)
		p_printf(WHITE,"Tag: type=%04x SNlen=%d SN=%s\n",CType,SN_len,sn_str);

	// find match to serial number string in config file
	return(perform_action(sn_str));
}

/* perform action based on content specific block
 * action is defined in rc522.conf as [@code] */

int block_action()
{
    int             tmp, addr = -1;
    unsigned char   ch;
    char         	*p, buff[16];
    char    		act_str[23];         // to hold block look up

    // get valid block
	p_printf (YELLOW,"Please provide the block number to read action-code.\n");
	p_printf (YELLOW,"Max %d.  (0 = exit) : ", max_blocks-1);
	if ((addr =get_block_number(max_blocks-1,1)) == 0)  close_out(0);

    if ((ch = get_card_permission(addr,1)) == 0xff) return(1);

    /* As KEYA is used for access, bit 1 should not be set, unless the value is 1
     * value 0 = default (With KEYA you can to everything except read KEYA)
     * 
     * As it was able to authenticate with KEYA, it might be able to read with KEYB instead (access == 0x3)
     * 
     * see MF1IC520_rev5_3.pdf card description.
     */

    if ((ch & 0x01) && (ch != 1) )
    {
		// use KEYB to read instead ?
		if (ch == 0x3 || ch == 0x5)
		{
			// access memory block with KEYB
			if (authorize(SN, addr,PICC_AUTHENT1B) != TAG_OK)
			{
				PcdHalt();
				return(1);
			}			
		}
		else
		{	
	        p_printf(RED,"Block %d has no read permission. (code: %x) \n", addr,ch);
	        PcdHalt();
	        return(1);
		}
	}
	else
	{
        // access memory block with KEYA
		if (authorize(SN, addr,PICC_AUTHENT1A) != TAG_OK)
		{
			PcdHalt();
			return(1);
		}
	}
	
    // read block
    if (read_tag_str(addr, (unsigned char *) buff) != TAG_OK)
    {
        p_printf(RED,"Error during reading %d\n",addr);
        PcdHalt();
        return(1);
    }

    PcdHalt();

    // create block content action string
    p=act_str;
    *(p++)='[';                     // start with [
    *(p++)='@';                     // add @ to make distinction to sen.
    for (tmp=0;tmp<16;tmp++) {      // add block content

       if (buff[tmp] == '0')       // value must be more then 0
            continue;

       *(p++)=buff[tmp];
    }
    *(p++)=']';                     // close out with ]
    *(p++)=0;

    if (debug)
    {
        p_printf(YELLOW, "\nBlock : %d has the following content %s.\n",addr,buff);
        p_printf(YELLOW, "\nCommand string is %s.\n",act_str);
    }

    if (strlen(act_str) < 3)
    {
        p_printf(RED,"Not enough action-code characters on block : %s.\n",act_str);
        return(1);
    }
    
    // now search and execute command
    return(perform_action(act_str));
}

/* display card information 
 * standard only the serial number is output to stderr, so it can be redirected to another
 * application
 * with the -d option, more card details are shown
 */
void* send_card_no(void* card_string)
{
	//int card_no;
	//char *card_string;
	//card_string = malloc(sizeof (char*) *23);
	int tmp;
	char* p, sn_str[23];         // to hold [serial] as a string

	// wait for card
	while (get_card_info() != TAG_OK) usleep(5000);

	// create and display serial string
	p=sn_str;
  //	*(p++)='[';                     // start with [
	for (tmp=0;tmp<SN_len;tmp++) {  // add serial number
		sprintf(p,"%02x",SN[tmp]);
		p+=2;
	}
  //	*(p++)=']';                     // close out with ]
	*(p++)=0;
	strcpy(card_string, sn_str);
	// display card information
/*	if (debug)
		p_printf(GREEN,"Tag: type: %04x SNlen: %d SN: %s maxblocks: %d\n",CType,SN_len,sn_str, max_blocks);
	else
		fprintf(stderr,"MINE:%s\n",sn_str);
*/
	//card_no = atoi(card_string);
	strcpy(card_str, card_string);
  PcdHalt();
	return 0;	
}



void disp_card_details()
{
	int 	tmp;
	char    *p, sn_str[23];         // to hold [serial] as a string
	
	// wait for card
	while (get_card_info() != TAG_OK) usleep(5000);

	// create and display serial string
	p=sn_str;                       
	*(p++)='[';                     // start with [
	for (tmp=0;tmp<SN_len;tmp++) {  // add serial number
		sprintf(p,"%02x",SN[tmp]);
		p+=2;
	}
	*(p++)=']';                     // close out with ]
	*(p++)=0;

	// display card information
	if (debug)
		p_printf(GREEN,"Tag: type: %04x SNlen: %d SN: %s maxblocks: %d\n",CType,SN_len,sn_str, max_blocks);
	else
		fprintf(stderr,"OK:%s\n",sn_str);
}

/* display access permission 
 * access = block number (0 - 3)
 * buff = trailer content information
 * 
 * This is information is only valid for data and value blocks
 * 
 * For block 3 it will call for disp_trailer_perm()
 * 
 * return code :
 * 0 = OK
 * 1 = ERROR
 */

int disp_access_perm(int access, uint8_t *buff)
{
	int	tmp;
	
	if (access == 3) return(disp_trailer_perm(access, buff));
	
	if (access < 0 || access > 2) 
	{
		p_printf(RED,"can only display access permission for data and value blocks. \nCurrent request is block %d\n", access);
		PcdHalt();
        return(1);
	}
		
	tmp = calc_access_bits(access, buff);
	
	if (tmp == 0xff)
    {
        p_printf(RED,"Issue with analysing trailer.\n");
        PcdHalt();
        return(1);
    }

    p_printf (GREEN,"\nCurrent access permission for this block:\n ");
    p_printf (YELLOW,"\tread\twrite\tincrement\tdecrement,\n\t\t\t\t\ttransfer,\n\t\t\t\t\trestore\n");
    if (tmp == 0){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 0.\tkeyA|B\tkeyA|B\tkeyA|B\t\tkeyA|B\t   (transport configuration)\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  0.\tkeyA|B\tkeyA|B\tkeyA|B\t\tkeyA|B\t(transport configuration)\n");

    if (tmp == 1){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 1.\tkeyA|B\tnever\tnever\t\tDecrement\t(value block)\n");
        p_printf(YELLOW,"| \t\t\tNO recharge possible, Decrement only \n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
    {
        p_printf(WHITE,"  1.\tkeyA|B\tnever\tnever\t\tDecrement\t(value block)\n");
		p_printf(WHITE,"  \t\t\tNO recharge possible, Decrement only \n");
    }
    if (tmp == 2){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 2.\tkeyA|B\tnever\tnever\t\tnever\t\t(read/write block)\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  2.\tkeyA|B\tnever\tnever\t\tnever\t\t(read/write block)\n");

    if (tmp == 3){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 3.\tkeyB\tkeyB\tnever\t\tnever\t\t(read/write block)\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  3.\tkeyB\tkeyB\tnever\t\tnever\t\t(read/write block)\n");

    if (tmp == 4){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 4.\tkeyA|B\tnever\tnever\t\tnever\t\t(read/write block)\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  4.\tkeyA|B\tnever\tnever\t\tnever\t\t(read/write block)\n");

     if (tmp == 5){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 5.\tkeyB\tnever\tnever\t\tnever\t\t(read/write block)\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  5.\tkeyB\tnever\tnever\t\tnever\t\t(read/write block)\n");

     if (tmp == 6){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 6.\tKeyA|B\tKeyB\tKeyB\t\tKeyA|B\t\t(Value block)\n");
        p_printf(YELLOW,"| \t\t\tRecharge only with KeyB \n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  6.\tKeyA|B\tKeyB\tKeyB\t\tKeyA|B\t\t(Value block)\n");
        p_printf(WHITE,"  \t\t\tRecharge / increment with KeyB\n");

     if (tmp == 7){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 7.\tnever\tnever\tnever\t\tnever\t\t(read/write block)\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  7.\tnever\tnever\tnever\t\tnever\t\t(read/write block)\n");

	return(0);
}

/* display trailer access rights 
 * can only display the trailer access permission
 * 
 * return code :
 * 0 = OK
 * 1 = ERROR
 */
int disp_trailer_perm(int access, uint8_t *buff)
{
	int	tmp;
	
	if (access != 3) 
	{
		p_printf(RED,"can only display access permissions for trailer. \nCurrent request is block %d\n", access);
		PcdHalt();
        return(1);
	}
		
	tmp = calc_access_bits(access, buff);
	
	if (tmp == 0xff)
    {
        p_printf(RED,"Issue with analysing trailer.\n");
        PcdHalt();
        return(1);
    }

    p_printf (GREEN,"\nCurrent access permission for trailer:\n ");
    p_printf (YELLOW,"\t|---- KEYA -----|--Access bits--|-- KEYB ---| \n");
    p_printf (YELLOW,"\tread\twrite\tread\twrite\tread\twrite\n");
    if (tmp == 0){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 0.\tnever\tkeyA\tkeyA\tnever\tkeyA\tkeyA \n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  0.\tnever\tkeyA\tkeyA\tnever\tkeyA\tkeyA \n");

    if (tmp == 1){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 1.\tnever\tkey A\tkeyA\tkeyA\tkeyA\tKeyA\t transport config\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
    {
        p_printf(WHITE,"  1.\tnever\tkey A\tkeyA\tkeyA\tkeyA\tKeyA\t transport config\n");
    }
    if (tmp == 2){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 2.\tnever\tnever\tkeyA\tnever\tkeyA\tnever \n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  2.\tnever\tnever\tkeyA\tnever\tkeyA\tnever \n");

    if (tmp == 3){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 3.\tnever\tkeyB\tkeyA|B\tkeyB\tnever\tkeyB\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  3.\tnever\tkeyB\tkeyA|B\tkeyB\tnever\tkeyB\n");

    if (tmp == 4){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 4.\tnever\tkeyB\tkeyA|B\tnever\tnever\tKeyB\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  4.\tnever\tkeyB\tkeyA|B\tnever\tnever\tKeyB\n");

     if (tmp == 5){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 5.\tnever\tnever\tkeyA|B\tkeyB\tnever\tnever\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  5.\tnever\tnever\tkeyA|B\tkeyB\tnever\tnever\n");

     if (tmp == 6){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 6.\tnever\tnever\tkeyA|B\tnever\tnever\tnever\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  6.\tnever\tnever\tkeyA|B\tnever\tnever\tnever\n");

     if (tmp == 7){
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
        p_printf(YELLOW,"| 7.\tnever\tnever\tkeyA|B\tnever\tnever\tnever\n");
        p_printf(WHITE,"----------------------------------------------------------------------------\n");
    } else
        p_printf(WHITE,"  7.\tnever\tnever\tkeyA|B\tnever\tnever\tnever\n");

	return(0);
}

/* update access bits on the trailer
 * access : wanted access level
 * addr = block address
 * 
 * 
 * return code :
 * OK : TAG_OK
 * error : TAG_ERR
 */
int update_access_bits_on_card(char access, int addr)
{
	int 		 t_addr;
	unsigned int i; 
	uint8_t		 buff[MAXRLEN*2];

	// read current trailer 
	if (read_trailer_block(&t_addr, addr, buff) != TAG_OK) return(TAG_ERR);

	// set the acccess bit and reverse in buffer
	set_access_bits(access,3 -(t_addr-addr),buff);
  
    // add keyA to trailer content (always reads as zero)
    for (i = 0; i< sizeof(KEYA);i++) buff[i] = KEYA[i];

    if (debug)
    {
        p_printf(GREEN,"New trailer to be written : ");
        for (i = 0;i < 16;i++) p_printf(WHITE,"%02x",buff[i]);
        p_printf(WHITE,"\n");
    }
   
    // update trailer
    if (PcdWrite(t_addr, buff) != TAG_OK){
        p_printf(RED,"Error during writing trailer %d.\n", t_addr);
        PcdHalt();
        return(TAG_ERR);
    }

    // read back to double check
    if (debug)
    {   
		read_tag_str(t_addr,buff);
		p_printf(YELLOW, "Trailer %d has been read and has the following content %s\n",t_addr, buff);
    }
    
    return(TAG_OK);
}

/* change access conditions for a specific block from the card 
 * 
 * changing access permission for the trailer is tricky and will only be allowed
 * if '-t' was provided on the command line (next to '-c' option)
 */

int change_block_access() 
{
    int             addr = -1, t_addr;
    unsigned char   ch;
    uint8_t         buff[20];

	// get valid block
	p_printf (YELLOW,"Please provide the block to update access rights.\n");
	p_printf (YELLOW,"Max %d. (0 = exit) : ", max_blocks-1);
	if ((addr = get_block_number(max_blocks-1,0)) == 0 ) close_out(0);

    // wait for card
    if (get_card_info() != TAG_OK)    return(TAG_ERR);

    p_printf(RED,
    "**********************************************\n"
    "  DO NOT REMOVE YOUR CARD FROM THE READER !!\n"
    "**********************************************\n\n");

    // check block fit
    if (addr >= max_blocks) 
    {
        p_printf(RED,"This card has only %d blocks. Can not read from %d.\n",max_blocks-1, addr);
        PcdHalt();
        return(TAG_ERR);
    }
    
	// determine access rights : access trailer
	if (read_trailer_block(&t_addr, addr, buff) != TAG_OK) return(TAG_ERR);
	
	/* display access permission */
	if (disp_access_perm(3 -(t_addr-addr), buff)) return(1);
   
    // check with trailer update !!!
    if ((addr == 3) || (! ((addr-3) % 4)))
    {
		if (UpdateTrailer)
		{
			p_printf (RED,"\n\tcareful !! careful !! careful !!\n");
			p_printf (RED,"\n\tdangerous !! dangerous !! dangerous !!\n");
			p_printf (RED,"\tYou are about to change access to a trailer\n");
			p_printf (RED,"\tSTOP IF YOU DO NOT KNOW THE IMPACT\n\n");
			p_printf (RED,"\tcareful !! careful !! careful !!\n");
		}
		else
		{
			p_printf (RED,"\nCan not change access rights on trailer.\n");
			return(TAG_OK);
		}
    }
    
    ch = '8';
    do          // Ask new access right
    {
        p_printf (YELLOW,"\nDo you want to change ?\nEnter 'n' for NO change or choose number of access right wanted. ");
        ch =get_charc();

        if ((ch == 'n') || (ch == 'N')){
            PcdHalt();
            p_printf(GREEN,"Abort update. You can remove the card\n");
            return(TAG_OK);
        }

        if ((ch < '0') || (ch > '7')){
            p_printf (RED,"Invalid entry. Try again. %d\n", ch);
            ch = '8';
        }
    }  while (ch == '8');

    //p_printf(WHITE,"before Byte 6: %02x Byte 7: %02x Byte 8 %02x\n", buff[6], buff[7], buff[8]);

	// update the access bits in the trailer
	if (update_access_bits_on_card(ch, addr) != TAG_OK) return(TAG_ERR);
	
    // close / disconnect the card
    PcdHalt();
    p_printf(GREEN,"Permissions have been updated. You can remove the card\n");
    return(TAG_OK);
}

/* read a block of 16 bytes from the card 
 * addr : block address to read
 * mess : buffer to store read content
 * 
 * return
 * Ok : TAG_OK
 * Error = TAG_ERR
 */
int read_block_raw(int addr, uint8_t *mess)
{
	unsigned char 	ch;
	 
    if ((ch = get_card_permission(addr,1)) == 0xff) return(1);

    // as we use keyA for access, bit 1 should NOT be set, unless the value is 1
    // see MF1IC520 card description.

    if ((ch & 0x01) && (ch != 1) )
    {
        p_printf(RED,"The block %d has no read permission. (%x) \n", addr,ch);
        PcdHalt();
        return(TAG_ERR);
    }

    // access memory block
    if (authorize(SN, addr,PICC_AUTHENT1A) != TAG_OK) return(TAG_ERR);

    // read block
    if (read_tag_raw(addr, mess) != TAG_OK)
	{
		p_printf(RED, "Error during reading block %d\n",addr);
		return(TAG_ERR);
	}

    return(TAG_OK);
}



/* read a block from the card and convert content to ascii
 * addr : block address to read
 * mess : buffer to store read content
 * disp : if 1 it will display the content
 * 
 * return
 * Ok : TAG_OK
 * Error = TAG_ERR
 */
int read_block(int addr, unsigned char *mess, int disp)
{
	unsigned char 	ch;
	int 	i; 
	 
    if ((ch = get_card_permission(addr,1)) == 0xff) return(1);

    // as we use keyA for access, bit 1 should NOT be set, unless the value is 1
    // see MF1IC520 card description.

    if ((ch & 0x01) && (ch != 1) )
    {
        p_printf(RED,"The block %d has no read permission. (%x) \n", addr,ch);
        PcdHalt();
        return(TAG_ERR);
    }

    // access memory block
    if (authorize(SN, addr,PICC_AUTHENT1A) != TAG_OK) return(TAG_ERR);

    // read block
    if (read_tag_str(addr,mess) == TAG_OK)
    {
	   if (disp)
	   {
		    // display block
		    p_printf(GREEN,"\nBlock %d has content: ",addr);
		
			for(i = 0; i<32; i++)
			{
				if(i> 0 &&  ! (i%2)) p_printf(WHITE,".");
				
				p_printf(YELLOW,"%c",mess[i]);
			}
			
			p_printf(WHITE,"\n");
			
			if ((addr == 3) || (! ((addr-3) % 4)))
				p_printf (RED,"Be aware that you can NEVER read KEYA value (always zero's) from the trailer.\n");

		}
		
    }
	else
	{
		p_printf(RED, "Error during reading block %d\n",addr);
		return(TAG_ERR);
	}

    return(TAG_OK);
}


/* read trailer of a specific block from the card 
 * addr    : block address to get trailer
 * *t_addr : to return the trailer address
 * *buff   : return content of trailer
 * 
 * return
 * TAG_OK
 * TAG_ERR
 */
int read_trailer_block(int *t_addr, int addr, uint8_t *buff)
{
	// determine trailer address
	*t_addr = (4 * (addr/4)) + 3;

	// if not authorized yet
	if (PcdCheckAuth())
	{
		// authorize access to trailer
		if (authorize(SN, *t_addr,PICC_AUTHENT1A) != TAG_OK) return(TAG_ERR);
	}
	
	// read trailer
    if (PcdRead(*t_addr,buff) != TAG_OK)
    {       
        p_printf(RED,"Error during reading trailer %d for block %d\n",*t_addr, addr);
        return(TAG_ERR);
    }
    
    return(TAG_OK);
}

/* read a specific block from the card */
int read_from_card()
{
    int     		addr = -1, tmp;
	unsigned char 	buff[35];
	
    // get valid block (reading block 0 is allowed)
    p_printf (YELLOW,"Please provide the block (Max %d) to read : ", max_blocks-1);
    addr = get_block_number(max_blocks-1,0);

	// read block and display content
	tmp = read_block(addr, buff, 1);
	
    // close / disconnect the card
    PcdHalt();
    return(tmp);
}

/* write a specific block
 * addr: block address
 * value : value to write (16 bytes)
 * check_empty : if 1 will read and check for empty first
 * read_after_write : if 1 will read and display content after write
 * 
 * return
 * ok : TAG_OK
 * error : TAG_ERR
 * stopped : TAG_NOTAG
 */
 
int write_block(int addr, uint8_t *uvalue, int check_empty, int read_after_write)
{    
    unsigned char   ch;
    unsigned char 	buff[35];
    char			ans[6];
    int 			i;
        
    if ((ch = get_card_permission(addr,1)) == 0xff)  return(TAG_ERR);

    // as we use keyA for access, the only value is 0x0
    if (ch != 0)
    {
        p_printf(RED,"Block %d has no write permission. (%x) \n", addr,ch);
        return(TAG_ERR);
    }

    // access memory block
    if (authorize(SN, addr,PICC_AUTHENT1A) != TAG_OK) return(TAG_ERR);

	// check on empty before writing ?
	if (check_empty)
	{
		// first read
	    if (read_tag_str(addr, buff) == TAG_OK)
	    {
			// check for any content
			for (i = 0; i< 32 ; i++)
				if (buff[i] != 0x30) break;

			// if any content than ask to overwrite
			if (i != 32 )
			{
				p_printf(RED,"Block is not empty.\n");
	
				// display block
			    p_printf(GREEN,"\nBlock %d has content: ",addr);
	
				for(i = 0; i < 32 ; i++)
				{
					if(i> 0 &&  !(i%2)) p_printf(WHITE,".");
					
					p_printf(YELLOW,"%c",buff[i]);
				}
	
				// ask to continue?
				do
			    {
			        p_printf(YELLOW,"\n\nAre you sure to overwrite this block %d (yes, no or exit) ? ", addr);
			        get_str(ans, sizeof(ans));
	
			        if (strcasecmp(ans,"no") == 0)  return(TAG_NOTAG);
			
			        else if (strcasecmp(ans,"exit") == 0)  close_out(0);
			
			        else if (strcasecmp(ans,"yes") != 0)
			            p_printf(RED,"Invalid answer. Please type 'yes' or 'no' or 'exit' \n");
			
			    } while (strcasecmp(ans,"yes") != 0);
			
			}
	    }
		else
		{
			p_printf(RED, "Error during reading block %d\n",addr);
			return(TAG_ERR);
		}
	}

    // perform write
    if (PcdWrite(addr, uvalue) != TAG_OK)
    {
        p_printf(RED,"Error during writing to address %d.\n",addr);
        return(TAG_ERR);
    }
    
	if (read_after_write)
	{
		// read back to show result
		if (read_tag_str(addr, buff) == TAG_OK)
		{
		    p_printf(BLUE, "\nBlock %d: now has the following content:\n",addr, buff);
		
			for(i = 0; i < 32 ; i++)
			{
				if(i> 0 &&  ! (i%2)) p_printf(WHITE,".");
				
				p_printf(YELLOW,"%c",buff[i]);
			}
			p_printf(BLUE,"\n\t------------------------\n\n");
		}
		else
			p_printf(RED,"Error during reading after write block %d\n", addr);
	}

    return(TAG_OK);
}

/* write to specific block on the card */
int write_to_card()
{
    int             value, numb, addr = -1;
    int             i, tmp,tmp1, lp = 1, ret;
    unsigned char   ch;
    uint8_t         uvalue[20];

	// get valid block
	p_printf (YELLOW,"Please provide the block to write to.\n");
	p_printf (YELLOW,"Max %d.  (0 = exit) : ", max_blocks-1);
	
	
	if ((addr=get_block_number(max_blocks-1,1)) == 0) close_out(0);

    do    // get valid value
    {
        // clear buffer
        memset(uvalue, 0x0,sizeof(uvalue));

        p_printf (YELLOW,"How many bytes you like to store (max 16 bytes) :");
        numb=get_number();

        if ((numb < 0) || (numb > 16))
        {
            p_printf(RED,"Incorrect number of bytes. Try again.\n");
            continue;
        }

        for (i=0 ; i < numb; i++)
        {
            do
            {
                p_printf (WHITE, "Provide byte %2d : 0x",i);

                tmp = scanf("%x", &value);
 
                if (tmp == 0)
                    p_printf(RED,"Invalid entry. Try again.\n");
                else
                    uvalue[i] = (uint8_t) value;

                // flush any input pending
                while ((tmp1=getchar()) != '\n' && tmp1 != EOF);

            } while (tmp == 0);
        }

        p_printf (GREEN,"You entered: ");
        for (i=0 ; i< numb; i++)  p_printf (WHITE, "0x%x ",uvalue[i]);

        p_printf(YELLOW,"\nIs this corrrect (y or n) ? ");
        ch = get_charc();
        if (ch == 'Y')  lp = 0;

    } while (lp); // get value

	// write block to address (1: check on empty,1: read after write)
	ret = write_block(addr, uvalue, 1, 1);
		
    // close / disconnect the card
    PcdHalt();
    return(ret);
}


/* read card into a temp-file */

int read_card_to_file()
{
    FILE    * fmem_str;            // saving card information
    char    file_str[255];         // save filename
    char    *p, status;
    int     lp,errcnt=0;
	unsigned char    str[255];      // read config file
    
	// wait for card
	while (get_card_info() != TAG_OK) usleep(5000);

    p=file_str;
    sprintf(p,"%s",fmem_path);      // copy Path
    
    p+=strlen(p);
    
    for (lp=0;lp<SN_len;lp++) {     // add serial number
        sprintf(p,"%02x",SN[lp]);
        p+=2;
    }
    sprintf(p,".txt");              // add extension

    if ((fmem_str=fopen(file_str,"r"))!= NULL) {
        p_printf(RED,"Not saving. File exists %s\n",file_str);
        fclose(fmem_str);
        return(TAG_ERR);
    }                               // Try to create file

    if ((fmem_str=fopen(file_str,"w"))== NULL)
    {
        p_printf(RED,"Not saving. Can not create for writing %s\n",file_str);
        return(TAG_ERR);
    }
    else
    {
                                     // save card memory
        p_printf(RED,
        "\n****************************************\n"
        "  Started reading... do not remove card.\n"
        "****************************************\n\n");


        for (lp = 0; lp < max_blocks; lp+=page_step) {

                p_printf(WHITE,"\rnow reading block %d ",lp);

                if (authorize(SN,lp,PICC_AUTHENT1A) != TAG_OK)
                {
					errcnt++;
					fprintf(fmem_str,"%02d: %s\n",lp,"Authentification error");
					
					// try to recover (expect impact 4 blocks = 1 sector)
					Pcd_stopcrypto1();
					find_tag(&CType);
					select_tag_sn(SN,&SN_len, SAK);
                }
                else
                {
                    if (read_tag_str(lp,str) == TAG_OK)
						fprintf(fmem_str,"%02d: %s\n",lp,str);
						
					else
					{
						errcnt++;
						fprintf(fmem_str,"%02d: %s\n",lp,"Error during reading block");
					}
				}
        }
        
        fclose(fmem_str);

        if(errcnt == 0 )
        {
            p_printf(GREEN,"All done. You can remove card.\n"
            "Content read is saved in %s\n",file_str);
            status = TAG_OK;
        }
        else 
        {
            p_printf(RED, "\r%d error(s) during reading.              \n"
			"Any content read is saved in %s\n",errcnt, file_str);
			status = TAG_ERR;
        }
    }
    return(status);
}

/* perform action based on card UID or block content number 
 * 
 * return
 * OK : 0
 * error : 1
 * no action : 2
 */

int perform_action(char * act_str)
{
    pid_t 	child;                  // execute command
    int 	tmp, i;
    char    str[255];  
									// look for match to action string
    
    if (find_config_param(act_str,str,sizeof(str))>0)
    {
		
		if(debug) p_printf(GREEN,"action %s found for %s\n",str,act_str);
        
        child=fork();               // if found : create child
        
        if (child == 0) 			
        {
            fclose(stdin);
            freopen("","w",stdout);
            freopen("","w",stderr); // execute command
            execl("/bin/sh","sh","-c",str,NULL);
        } 
        
        else if (child > 0) {  		// parent      
            
            i=6000;					// wait max 1 minute
            
            do {
                usleep(10000);
                tmp = wait3(NULL,WNOHANG,NULL);
                i--;
            } while (i>0 && tmp != child);

            if (tmp != child) 
            {
                kill(child,SIGKILL);
                wait3(NULL,0,NULL);
                
                p_printf(RED,"Killed\n");
                return(1);
            }
            else 
            {
                p_printf(BLUE,"Exit\n");
                return(0);
			}
        }
        else
        {
            p_printf(RED,"Can't run child process! (%s %s)\n",act_str,str);
            return(1);
        }
     }
     else
        p_printf(YELLOW, "No action found in %s for number %s\n",config_file,act_str);

     return(2);
}

/* wait for card, read card type and serial number */

int get_card_info()
{
    int 	status;
    int 	cnt = 0;
    
    p_printf(BLUE,"Hold your card for the reader ");

    do
    {
        status = find_tag(&CType);

        if (cnt++ > 3){                 // display keep alive
            cnt = 0;
            p_printf(BLUE,".");
        }

        if (status == TAG_NOTAG) {     // no card :wait)
            usleep(200000);
            continue;
        }
        else if                        // issue with tag reading ?
        (status != TAG_OK && status != TAG_COLLISION)
            continue;
                                        // read serial number & SAK
        status = select_tag_sn(SN, &SN_len, SAK);

    } while (status != TAG_OK);

    p_printf(GREEN,"card found.\n");

	/* determine memory details
	 * but this is very cryptic and hard to exactly discover
	 * SAK = Select acknowledgement has card details in it but still...
	 * nearly mission impossible
	 */
	
	max_blocks= 0;
	
	switch (CType) {
		case 0x4400:          
					
			if (SAK[0] == 0x0)
			{
				p_printf(YELLOW,"Ultralight card \n");
				p_printf(RED, "NOT supported by this program\n");
			}	
			else if (SAK[0] == 0x8)
			{
				p_printf(YELLOW,"Mifare Classic(1K) or PLUS card (2K)\n");
				p_printf(RED,"In case of PLUS card, we will only read the first 1K (64 blocks)\n");
				max_blocks=64;	
				page_step=1;       // increment to read the card
			}
			else if (SAK[0] == 0x9) // 20 blocks (5 sectors x 4 blocks) x 16 bytes
			{
				p_printf(YELLOW,"Mifare MINI(0.3K)\n");	  
				max_blocks=20;
				page_step=1;       // increment to read the card
			}
			else if (SAK[0] == 0x18)
			{
				p_printf(YELLOW,"PLUS card (4K)\n");
				p_printf(RED,"Can only access the first 128 blocks (NOT last 8 sectore with 16 blocks)\n");
				max_blocks=128;
				page_step=1;       // increment to read the card
			}
			break;
		
		case 0x0400:		
			if (SAK[0] == 0x8) // 64 blocks of 16 bytes / Ul card
			{
				p_printf(YELLOW,"Mifare Classic(1K) or SmartMX with MIFARE 1K emulation\n");
				max_blocks=64;	// 
				page_step=1;       // increment to read the card
	        }
	        else if (SAK[0] == 0x9) // 20 blocks (5 sectors x 4 blocks) x 16 bytes
	        {
				p_printf(YELLOW,"Mifare MINI(0.3K)\n");	  
				max_blocks=20;
				page_step=1;       // increment to read the card
	        }
			break;
		
		case 0x0200:		
			if (SAK[0] == 0x18) // 64 blocks of 16 bytes / Ul card
			{
				p_printf(YELLOW,"Mifare Classic(4K) or SmartMX with MIFARE 4K emulation\n");
				p_printf(RED,"Can only access the first 128 blocks (NOT last 8 sectore with 16 blocks)\n");
				max_blocks=127;
				page_step=1;       // increment to read the card
	        }
	        else if (SAK[0] == 0x9) // 20 blocks (5 sectors x 4 blocks) x 16 bytes
	        {
				p_printf(YELLOW,"Mifare MINI(0.3K)\n");	  
				max_blocks=20;
				page_step=1;       // increment to read the card
	        }
			break;
		}
		
		if (max_blocks == 0)
		{
			p_printf(YELLOW,"Card type not known \n)");
			PcdHalt();
			close_out(1);
		}
		
    return(status);
}

/* catch signals to close out correctly 
 * no distinction on signal received */
void signal_handler(int sig_num)
{
	p_printf(YELLOW, "\nreceived signal %d:  stopping RC522 Reader.\n",sig_num);
	close_out(2);
}


/* setup signals */
void set_signals()
{
	struct sigaction act;
	
	memset(&act, 0x0,sizeof(act));
	act.sa_handler = &signal_handler;
	sigemptyset(&act.sa_mask);
	
	sigaction(SIGTERM,&act, NULL);
	sigaction(SIGINT,&act, NULL);
	sigaction(SIGABRT,&act, NULL);
	sigaction(SIGSEGV,&act, NULL);
	sigaction(SIGKILL,&act, NULL);
}

/* check that string has valid number sequence
 *   
 * return 
 * 1 = decimal number, 
 * 2 = hex-number (starting with 0x...)
 * 3 = error
 */
int check_num(char *arg)
{
	int 	i=0,ret = 0, dx=0;
	char	prev_ch=0;
	
	while (arg[i] != 0x0)
	{
		if (arg[i] == 'x' || arg[i] == 'X')
		{
			// validate 0x or 0X header
			if (prev_ch != '0' || i != 1) return(3);
			else dx = 1;
		}
		
		// check for decimal
		else if (arg[i] >= '0' && arg[i] <= '9')
		{
			if (ret != 2 ) ret = 1;
		}
		 
		// check for hex (lower case)
		else if (arg[i] >= 'a' && arg[i] <= 'f')
			ret = 2;

		// check for hex (upper case)
		else if (arg[i] >= 'A' && arg[i] <= 'F')
			ret = 2;			
		
		else
			return(3);
		
		prev_ch= arg[i++];
	}

	// check hexnumbers always start with 0x 
	if (ret == 2)
		if (dx != 1) return(3);
		
	return(ret);
}


/* convert a line with ascii to bin 
 * line : input line with hex character
 * data : output to bytes
 * filter_eof : if 1 end of line characters will be skipped (0x0a, 0x0d)
 * return 
 * OK : number of bytes in data
 * error = 0;
 */
int ascii_to_hex(char *line, char *data, int filter_eof)
{
    int inlen = strlen(line);
    int i, outlen;
    char c;
    uint8_t val[2];
    
    outlen = 0;    
    i = 0;
    
    while (inlen > 0 && *line != '\0')
    {
        inlen--;
        c = *line++;
               
        // if requested skip CR and LF       
        if (filter_eof)
			if (c == 0xd || c == 0xa) continue;
    
        // for each character (2 nibbles)

        if (c >= '0' && c <= '9')
            val[i++] = c - '0';
        else if (c >= 'a' && c <= 'f')
            val[i++] = 10 + c - 'a';
        else if (c >= 'A' && c <= 'F')
            val[i++] = 10 + c - 'A';
        else
        {
			p_printf (RED, "Invalid hex character %c or 0x%02x\n",c,c);
            return 0;
        } 
        
        if (i == 2)
        {
            // create 1 output byte from 2 nibbles
            *data = (val[0] << 4) | val[1];
    
            // increment length
            outlen++;
        
            // next output pos
            data++;
    
            i = 0;
        }
    }

    return(outlen);
}

/* check the received message to write, allocate memory and return
 * pointer
 * 
 * arg : option argument, message to write
 * option : either M = alhpha numeric or H = hex-decimal
 * length = return number of bytes in returned bytes buffer
 * 
 * return:
 * OK : pointer to buffer with bytes to write
 * ERROR : NULL
 * 
 * memory needs to be released after use by caller !
 */
char * set_message(char *arg, int option, int *length)
{
	char	* mess;
	int		len;
	
	// check for valid options
	if (option != 'M' && option != 'H')
	{
		p_printf(RED,"invalid option\n");
		return(NULL);
	}
	
	// get length of message
	len = strlen(arg);

	if (len < 0)
	{
		p_printf(RED,"missing message to write\n");
		return(NULL);
	}
	
	/* allocate memory for message
	 * Has to be on 16 bytes boundary
	 * add 1 extra boundary to be sure have enough memory
	 */
	mess = malloc(16 * ((len/16) + 1));
	
	if (mess == NULL)
	{
		p_printf(RED,"can not allocate memory for message");
		return(NULL);
	}
	
	// initialize to zero's	
	memset(mess, 0x0, 16 * ((len/16) + 1));
	
	// copy message in case of alpha numeric
	if (option == 'M')
	{
		strncpy(mess, arg,len);
	}
	// convert message to hex
	else
	{
		// 0 = do not filter EOF characters out
		len = ascii_to_hex(arg, mess, 0);
		
		if (len == 0)
		{
			p_printf(RED,"Error during converting Hex characters\n");
			
			// release memory
			free(mess);
			
			return(NULL);
		}
	}
	// set length
	*length = len;
	
	return(mess);
}

/* read/get message from starting point and display to stderr.
 * 
 * start_sector: sector to start reading
 * start_block :  block in sector to start
 * len : length of bytes to get
 * disp_as_alpha : 1 = display as readable characters else hex-values.
 * 
 * return
 * ok : TAG_OK
 * error : TAG_ERR
 */
int get_message(int start_sector,int  start_block, int len, int disp_as_alpha)
{
	int	sct = start_sector;
	int blck = start_block;
	int i = 0;
	int addr = (4 * start_sector) + start_block;
	unsigned char *mess;

	p_printf(GREEN, "getting message\n");

	/* allocate memory for message
	 * Has to be on 16 bytes boundary
	 * add 1 extra boundary to be sure have enough memory
	 */
	mess = (unsigned char *) malloc(16 * ((len/16) + 1));

	if (mess == NULL)
	{
		p_printf(RED,"can not allocate memory for getting message");
		return(TAG_ERR);
	}

	// while not end of data
	while (i < len)
	{
		// check it will fit
		if (addr >= max_blocks)
		{
			p_printf(RED, "Block location exceeding card limits (addr = %d)\n", addr);
			return(TAG_ERR);
		}
		
		if (debug)
			p_printf(YELLOW, "Current address: %d (sector: %d, block: %d) %d bytes read sofar\n",addr, sct, blck, i); 
		
		// read block from address
		if (read_block_raw(addr, mess + i) == TAG_ERR)
		{
			p_printf(RED,"Message read failed\n");
			return(TAG_ERR);
		}
		
		// back to normal communication
		Pcd_stopcrypto1();

		i += 16;	// next 16 bytes
		blck++;		// set next block 

		// if the last block (trailer)
		if (blck == 3)
		{
			blck = 0;	// block 0 on next sector
			sct++;			
		}
		
		// set new address
		addr = (4 * sct) + blck;
	}

    // close / disconnect the card
    PcdHalt();
	
	p_printf(GREEN,"DONE ! Total amount of bytes read : %d or %d block(s)\n",i, i/16);
	
	// display to stderr
	for(i = 0; i< len; i++)
	{
		// As Ascii (hopefully) readable characters
		if(disp_as_alpha) fprintf(stderr,"%c", (char) mess[i]);
		
		// display as hex-character
		else fprintf(stderr,"%02x", mess[i]);
	}
	
	// terminate with new line
	fprintf(stderr,"\n");
	
	free(mess);
		
	return(TAG_OK);	
}
/* write a message that could cross multipe data blocks & sectors
 * 
 * it will skip the trailer / admin block if necessary
 * 
 * mess:  message to write
 * start_sector: starting sector
 * start_block : staring block
 * len : length of message 
 * 
 * Return:
 * ok = TAG_OK
 * error = TAG_ERR
 */
int write_message(uint8_t *mess, int start_sector, int start_block, int len)
{
	int	sct = start_sector;
	int blck = start_block;
	int i = 0;
	int addr = (4 * start_sector) + start_block;

	p_printf(GREEN, "writing message\n");

	// while not end of data
	while (i < len)
	{
		// check it will fit
		if (addr >= max_blocks)
		{
			p_printf(RED, "Block location exceeding card limits (addr = %d)\n", addr);
			return(TAG_ERR);
		}
		
		if (debug)
			p_printf(YELLOW, "Current address: %d (sector: %d, block: %d) %d bytes written sofar\n",addr, sct, blck, i); 
		
		// write block to address (0: no check_on_empty, 0: no_read_after_write)
		if (write_block(addr, (mess + (uint8_t) i), 0, 0) == TAG_ERR)
		{
			p_printf(RED,"Badge block write failed\n");
			return(TAG_ERR);
		}
		
		// back to non-crypted communication
		Pcd_stopcrypto1();
	
		i += 16;	// next 16 bytes
		blck++;		// set next block 

		// if the last block in sector (trailer)
		if (blck == 3)
		{
			blck = 0;	// block 0 on next sector
			sct++;			
		}
		
		// set new address
		addr = (4 * sct) + blck;
	}
	
	p_printf(GREEN,"DONE ! Total amount of bytes written : %d or %d block(s)\n",i, i/16);
		
    // close / disconnect the card
    PcdHalt();

	return(TAG_OK);
}
/*
int main() {
  
    int pass = 0; // Pass no Pass
  
    // must be run as root to open /dev/mem in BMC2835
    if (getuid() != 0)
    {
        p_printf(RED,"Must be run as root.\n");
        exit(1);
    }
  
    Led_On();

    // catch signals
	set_signals();
	  
    // read /etc/rc522.conf 
    if (get_config_file()) exit(1);

    // set BCM2835 Pins correct
    if (HW_init(spi_speed,gpio)) close_out(1);
  
    // initialise the RC522
	InitRc522();

    // read & set GID and UID from config file
    if (read_conf_uid()!= 0) close_out(1);


    digitalWrite(3, LOW);	
    while (loop > 0)
    {
	char *card_no;
	card_no = send_card_no();
	printf("Message recieved: %s\n", card_no); 
	// count this card
        if (NotEndless)    loop--;
      
        use_vblock = 0xff;

        // close / disconnect the card
        PcdHalt();        

	int Led_Pin = 0;
	printf("CARD: %s\n",card_str);
	pass = user_access(card_str);
	//pass = 0;
	if (pass){
	  Led_Pin = 4;
          printf("Green!: %d\n", pass);
	  digitalWrite(Led_Pin, HIGH);
  	  bcm2835_delay(1500);
	  digitalWrite(Led_Pin, LOW);
          //Buzz_Once();
        }
        else{
	  Led_Pin = 3;
          printf("Red!: %d\n", pass);
	  digitalWrite(Led_Pin, HIGH);
  	  bcm2835_delay(1500);
	  digitalWrite(Led_Pin, LOW);
          //Buzz_Twice();
        }
        printf("OUT");
       
        p_printf(YELLOW,"Please remove card within 5 seconds\n");
            
        sleep(5);
        
    }   // loop

    return 0;
}*/
