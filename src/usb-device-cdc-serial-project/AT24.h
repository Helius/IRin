#ifndef _EEPR_AT24_H_
#define _EEPR_AT24_H_

//запись в память по адресу пакета длинной len
int write_eeprom (int adr, char *str, int len);
//чтение из памяти по адресу пакета длинной len
int read_eeprom (int adr, char *str, int len);

#endif
