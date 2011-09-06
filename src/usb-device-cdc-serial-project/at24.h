#ifndef _EEPR_AT24_H_
#define _EEPR_AT24_H_

// write to eeprom 
int write_eeprom (int adr, char *str, int len);

// read to eeprom
int read_eeprom (int adr, char *str, int len);

#endif
