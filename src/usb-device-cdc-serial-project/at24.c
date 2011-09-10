#include "i2csw.h"
#include "at24.h"

// need to add support other at24xx (they have different byte addressing mode)

/*
NOTE!!!
AT24C16A using i2c address byte A0 A1 A2 lower bit as P0 P1 P2 - high part of page address, with next after i2c address it is 11 bit (2^11 = 2048 byte)
*/

//*****************************************************************************
// write data to eeprom inside one page, return 0 on successful and 1 if not.
// (we remember, that we can't write more byte than one page at one time, see datasheet)
int at24_write_bound (int adr, char * data, int len)
{ 
	int res = 0;
	int cnt = 0;

	do {
		i2c_start ();
		// at24c16a use A0 A1 A2 bit for addresing memory too
		res = i2c_putbyte (EEPROM_ADR | ((adr>>7)&0x0E));
	} while (res && (cnt < 20));
	
	if (res) return 1;
	
	res |= i2c_putbyte (adr);
	for (int i = 0; i < len; i++)
		res |= i2c_putbyte (data[i]);
	i2c_stop ();
	return res;
}

//*****************************************************************************
int at24_write (int adr, char * data, int len)
{
	//write from adr to current page end
	int tmp_len = AT24_PAGE_LEN - (adr % AT24_PAGE_LEN);
	if (at24_write_bound (adr, data, tmp_len))
		return 1;
	len -= tmp_len;
	adr += tmp_len;
	data += tmp_len;
	// now write all entire page
	while (len > AT24_PAGE_LEN) {
		if (at24_write_bound (adr, data, AT24_PAGE_LEN))
			return 1;
		len -= AT24_PAGE_LEN;
		adr += AT24_PAGE_LEN;
		data += AT24_PAGE_LEN;
	}
	// in the end we write tail smaller than page_size
	if ((len > 0) && at24_write_bound (adr, data, len % AT24_PAGE_LEN))
			return 1;
	return 0;
}

//*****************************************************************************
// return 1 if eeprom not respons, otherwise 0
int at24_read (int adr, char * data, int len)
{
	int cnt = 0;
	int res = 0;

	do {
		i2c_start ();
		// at24c16a use A0 A1 A2 bit for addresing memory too
		res = i2c_putbyte (EEPROM_ADR | ((adr>>7)&0x0E));
	} while (res && (cnt < 20));

	if (res) return 1;
	
	res |= i2c_putbyte (adr);
	i2c_start ();
	res |= i2c_putbyte (EEPROM_ADR + EEPROM_RW_BIT);
	for (int i = 0; i < len-1; i++)
		data[i] = i2c_getbyte (1);
	data[len-1] = i2c_getbyte (0);
	i2c_stop ();
	return res;
}
