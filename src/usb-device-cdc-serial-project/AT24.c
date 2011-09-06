/// ������ � EEPROM AT24xx �� i2c ����������
/**\file
������� ��� ������ � ������ � EEPROM AT24xx �� i2c ����
*/
#include "i2csw.h"
#include "AT24.h"

//*****************************************************************************
// ����� ������ �� ������ � ������, ���������� 1 ���� ��� �� ���
int write_eeprom (int adr, char *str, int len)
{ 
  int count = 0;
  int addr_lo = adr ;//������� ���� ������
  int addr_hi = 0xA0;

  while (i2c_write_string(addr_hi, addr_lo, str, len) && (count++ < 8));
  
  if(count == 8) 
		return 1;
  else 
		return 0;
}

//*****************************************************************************
// ������ ������ �� ������ �� ������, ���������� 1 ���� ��� �� ���
int read_eeprom (int adr, char *str, int len)
{
	int count = 0;
	int addr_lo = adr;//������� ���� ������
	int addr_hi = 0xA0;

	while(i2c_read_string(addr_hi, addr_lo, str, len) && (count++ < 8));
	if(count == 8) 
		return 1;
	else 
		return 0;   
}
