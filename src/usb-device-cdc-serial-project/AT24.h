#ifndef _EEPR_AT24_H_
#define _EEPR_AT24_H_

//������ � ������ �� ������ ������ ������� len
int write_eeprom (int adr, char *str, int len);
//������ �� ������ �� ������ ������ ������� len
int read_eeprom (int adr, char *str, int len);

#endif
