#include "i2csw.h"
#include "at24.h"

// need to add support other at24xx (they have different byte addressing mode)

/*
NOTE!!!
AT24C16A using i2c address byte A0 A1 A2 lower bit as P0 P1 P2 - high part of page address, with next after i2c address it is 11 bit (2^11 = 2048 byte)
*/

//*****************************************************************************
// return 1 if eeprom not respons, otherwise 0
int at24_write_page (int page_adr, char * data, int len)
{ 
	int res = 0;
	int cnt = 0;

	do {
		i2c_start ();
		// at24c16a use A0 A1 A2 bit for addresing memory too
		res = i2c_putbyte (EEPROM_ADR | ((page_adr>>7)&0x0E));
	} while (res && (cnt < 20));
	
	if (res) return 1;
	
	res |= i2c_putbyte (page_adr);
	for (int i = 0; i < len; i++)
		res |= i2c_putbyte (data[i]);
	i2c_stop ();
	return res;
}

//*****************************************************************************
// return 1 if eeprom not respons, otherwise 0
int at24_read_page (int page_adr, char * data, int len)
{
	int cnt = 0;
	int res = 0;

	do {
		i2c_start ();
		// at24c16a use A0 A1 A2 bit for addresing memory too
		res = i2c_putbyte (EEPROM_ADR | ((page_adr>>7)&0x0E));
	} while (res && (cnt < 20));

	if (res) return 1;
	
	res |= i2c_putbyte (page_adr);
	i2c_start ();
	res |= i2c_putbyte (EEPROM_ADR + EEPROM_RW_BIT);
	for (int i = 0; i < len-1; i++)
		data[i] = i2c_getbyte (1);
	data[len-1] = i2c_getbyte (0);
	i2c_stop ();
	return res;
}
