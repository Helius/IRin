// software i2c library for read and write to bus

#include "i2csw.h"

int bus_delay = 5;

//*****************************************************************************
void i2c_set_delay (int delay) {
	bus_delay = delay;
}

//*****************************************************************************
int i2c_get_delay (void) {
	return bus_delay;
}

//*****************************************************************************
void i2c_delay (int cnt)
{
	for (int i = 0; i < cnt*bus_delay; i++) {
		volatile int tmp = i;
	}
}

//*****************************************************************************
int i2c_start (void)
{				
	SCL_IN;
	SDA_IN;
	i2c_delay (1);
	SDA_OUT;
	i2c_delay(1);
	SCL_OUT;
	i2c_delay(1);
	return 0;
}
//*****************************************************************************
void i2c_stop (void)
{
	SDA_OUT;
	i2c_delay(1);  
	SCL_IN;
	i2c_delay(1);  
	SDA_IN;
}
//*****************************************************************************
void i2c_init (void)
{
	SDA_IN;
	SCL_IN;
	SDA_CLR;
	SCL_CLR;
	i2c_stop();
	i2c_delay (1);
}

//*****************************************************************************
int i2c_putbyte (int data)
{
	int count = 8, ret = 0;
	while (count--) {
		SCL_OUT;
		if (data&0x80)
			SDA_IN;
		else                
			SDA_OUT;
		data<<=1;
		i2c_delay(1);
		SCL_IN;	
		i2c_delay(1);
	}
	SCL_OUT;
	i2c_delay(1);
	SDA_IN;
	i2c_delay(1);
	SCL_IN;
	if (SDA_IS) 
		ret=1;
//	i2c_delay(1);
	SCL_OUT;
	return ret;
}

//*****************************************************************************
int i2c_getbyte (int ack)
{
	int count=8, data=0xFF;

	while (count--) {
		data <<= 1;
		SCL_OUT;
		i2c_delay(1);
		SCL_IN;
		i2c_delay(1);
		if (SDA_IS) data |= 1;	
	}

	SCL_OUT;
	i2c_delay(1);
	if ( ack ) { 
		SDA_OUT;
	} else {
		SDA_IN;
	}
	i2c_delay(1);
	SCL_IN;
	i2c_delay(1);
	SCL_OUT;
	SDA_IN;
	return data;
}

