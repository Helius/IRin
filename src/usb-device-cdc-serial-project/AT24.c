/// работа с EEPROM AT24xx на i2c интерфейсе
/**\file
функции для чтения и записи в EEPROM AT24xx на i2c шине
*/
#include "i2csw.h"
#include "AT24.h"

//*****************************************************************************
// пишем строку по адресу в еепром, возвращаем 1 если что не так
int write_eeprom (int adr, char *str, int len)
{ 
  int count = 0;
  int addr_lo = adr ;//младший байт адреса
  int addr_hi = 0xA0;

  while (i2c_write_string(addr_hi, addr_lo, str, len) && (count++ < 8));
  
  if(count == 8) 
		return 1;
  else 
		return 0;
}

//*****************************************************************************
// читаем строку по адресу из еепром, возвращаем 1 если что не так
int read_eeprom (int adr, char *str, int len)
{
	int count = 0;
	int addr_lo = adr;//младший байт адреса
	int addr_hi = 0xA0;

	while(i2c_read_string(addr_hi, addr_lo, str, len) && (count++ < 8));
	if(count == 8) 
		return 1;
	else 
		return 0;   
}
