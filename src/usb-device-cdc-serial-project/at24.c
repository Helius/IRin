#include "i2csw.h"
#include "at24.h"

// need to add support other at24xx (they have different byte addressing mode)

/*
NOTE!!!
AT24C16A using i2c address byte A0 A1 A2 lower bit as P0 P1 P2 - high part of page address, with next after i2c address it is 11 bit (2^11 = 2048 byte)
*/

//*****************************************************************************
int at24_write_page (int page_adr, char * data, int len)
{ 
	int res = 0;
	i2c_start ();
	// at24c16a use A0 A1 A2 line for addresing memory too
	res |= i2c_putbyte (EEPROM_ADR | ((page_adr>>7)&0x0E));
	if (res)
		return res;
	res |= i2c_putbyte (page_adr);
	for (int i = 0; i < len; i++)
		res |= i2c_putbyte (data[i]);
	i2c_stop ();
	return res;
	
}

//*****************************************************************************
int at24_read_page (int page_adr, char * data, int len)
{
	int res = 0;
	i2c_start ();
	res |= i2c_putbyte (EEPROM_ADR | ((page_adr>>7)&0x0E));
	if (res) 
		return res;
	res |= i2c_putbyte (page_adr);
	i2c_start ();
	res |= i2c_putbyte (EEPROM_ADR + EEPROM_RW_BIT);
	for (int i = 0; i < len-1; i++)
		data[i] = i2c_getbyte (1);
	data[len-1] = i2c_getbyte (0);
	i2c_stop ();
	return res;
}
