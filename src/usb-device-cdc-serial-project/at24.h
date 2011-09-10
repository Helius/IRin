#ifndef _EEPR_AT24_H_
#define _EEPR_AT24_H_

//**************************** Config section *********************************
#define EEPROM_MODEL  AT24C16
#define EEPROM_ADR    0xA0
#define EEPROM_RW_BIT 1
//*****************************************************************************

#if  (EEPROM_MODEL == AT24C16)
	#define AT24_PAGE_LEN  16
	#define AT24_PAGE_NMB  128
#elif (EEPROM_MODEL == AT24C08)
	#define AT24_PAGE_LEN  16
	#define AT24_PAGE_NMB  64
#elif (EEPROM_MODEL == AT24C04)
	#define AT24_PAGE_LEN  16
	#define AT24_PAGE_NMB  32
#elif (EEPROM_MODEL == AT24C02)
	#define AT24_PAGE_LEN  8
	#define AT24_PAGE_NMB  32
#endif

// write to eeprom, return 0 if successful, otherwise 1 
int at24_write (int page_adr, char * data, int len);

// read from eeprom, return 0 if successful, otherwise 1 
int at24_read (int page_adr, char * data, int len);

#endif
