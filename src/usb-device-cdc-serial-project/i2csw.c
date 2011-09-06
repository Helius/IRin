// software i2c library for read and write to bus

#include "i2csw.h"

//*****************************************************************************
void i2c_init (void)
{
    SDA_IN;
    SCL_IN;
    SDA_CLR;
    SCL_CLR;
}
//*****************************************************************************
static void i2c_start (void)
{				
    SDA_OUT;
    i2c_delay2();
    SCL_OUT;
    i2c_delay2();
}
//*****************************************************************************
static void i2c_rep_start (void)
{
    SCL_OUT;
    SDA_IN;
    i2c_delay2();           
    SCL_IN;
    i2c_delay();           
}
//*****************************************************************************
static void i2c_stop (void)
{
    SCL_OUT;
    SDA_OUT;
    i2c_delay2();    
    SCL_IN;
    i2c_delay2();  
    SDA_IN;
    i2c_delay2();    
}
//*****************************************************************************
static int i2c_putbyte (int data)
{
	int count = 8, ret = 0;
	while (count--) {
		SCL_OUT;
		i2c_delay();
		if ( data&0x80 )
			SDA_IN;
		else                
			SDA_OUT;
		data<<=1;
		i2c_delay();
		SCL_IN;	
		i2c_delay();
	}
	SCL_OUT;
	i2c_delay();
	SDA_IN;
	i2c_delay();
	SCL_IN;

	while (!(SCL_IS));
	if (SDA_IS) 
		ret=1;
	i2c_delay2();
	return ret;
}

//*****************************************************************************
static int i2c_getbyte (int ack)
{
	int count=8, data;

	while (count--) {
		data <<= 1;
		SCL_OUT;
		i2c_delay2();
		SCL_IN;
		i2c_delay();
		if (SDA_IS) data |= 1;	
	}

	SCL_OUT;
	i2c_delay();
	if ( ack )  
		SDA_OUT;
	else        
		SDA_IN;
	
	i2c_delay();
	SCL_IN;
	while (!(SCL_IS));
	i2c_delay();
	SCL_OUT;
	i2c_delay2();
	SDA_IN;
	i2c_delay2();
	return data;
}

//*****************************************************************************
// read 1 byte with adress
int i2c_read_byte (int addr_hi, int addr_lo)
{
	i2c_start();	
	i2c_putbyte(addr_hi);
	i2c_putbyte(addr_lo);
	i2c_stop(); 
	i2c_delay5();
	i2c_start();	
	i2c_putbyte(addr_hi+I2C_READ_BIT);
	int data=i2c_getbyte(0);
	i2c_stop();

	return data;
}

//*****************************************************************************
// write 1 byte to adr, return 0 if bus response and 1 if not respons
int i2c_write_byte(int addr_hi, int addr_lo, int data)
{
	int res = 0;
	i2c_start();	
	res |= i2c_putbyte(addr_hi);
	res |= i2c_putbyte(addr_lo);
	res |= i2c_putbyte(data);
	i2c_stop();
	return res;
}

//*****************************************************************************
// read array of byte to *str, return 0 if bus response and 1 if not respons
int i2c_read_string (int addr_hi, int addr_lo, char *str, int len)
{
	i2c_start();
	if(i2c_putbyte(addr_hi)) {
          return 1;
        }
	if(i2c_putbyte(addr_lo)) {
          return 1;
        }
	i2c_stop();
	i2c_delay5();
	i2c_start();	
	if(i2c_putbyte(addr_hi+I2C_READ_BIT)) {
          return 1;
        }
	while (--len)
		*(str++)=i2c_getbyte(1);

	*(str++)=i2c_getbyte(0);
	i2c_stop();
    return 0;
}

//*****************************************************************************
// write array *str, return 0 if bus respons and 1 if not respons
int i2c_write_string(int addr_hi, int addr_lo, char *str, int len)
{
	i2c_start();	
	if(i2c_putbyte(addr_hi)) {
          return 1; 
        }
	if(i2c_putbyte(addr_lo)) {
          return 1;
        }
	while (len--)
          if(i2c_putbyte(*(str++))) {
             return 1;
          }
	i2c_stop();
  return 0;
}

