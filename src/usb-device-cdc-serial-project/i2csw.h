#ifndef _I2CSW_H_
#define _I2CSW_H_

#include "lib_AT91SAM7S64.h"

//******************************** set up **************************************
// define data and clock line
#define SCL_LINE    AT91C_PIO_PA8
#define SDA_LINE    AT91C_PIO_PA7

// clock for delay
#define F_MHZ 60

#define I2C_READ_BIT  1

//#define __delay_cycles(x)   for ( int __jj=0; __jj<x; __jj++ ) {
//	volatile int __tmp=0;                                         
//}                                                      

#define SCL_SET     AT91F_PIO_SetOutput   (AT91C_BASE_PIOA, SCL_LINE) 
#define SCL_CLR     AT91F_PIO_ClearOutput (AT91C_BASE_PIOA, SCL_LINE)
#define SCL_OUT     AT91F_PIO_CfgOutput   (AT91C_BASE_PIOA, SCL_LINE) 
#define SCL_IN      AT91F_PIO_CfgInput    (AT91C_BASE_PIOA, SCL_LINE)
#define SCL_IS      AT91F_PIO_IsInputSet  (AT91C_BASE_PIOA, SCL_LINE)

#define SDA_SET     AT91F_PIO_SetOutput   (AT91C_BASE_PIOA, SDA_LINE) 
#define SDA_CLR     AT91F_PIO_ClearOutput (AT91C_BASE_PIOA, SDA_LINE)
#define SDA_OUT     AT91F_PIO_CfgOutput   (AT91C_BASE_PIOA, SDA_LINE) 
#define SDA_IN      AT91F_PIO_CfgInput    (AT91C_BASE_PIOA, SDA_LINE)
#define SDA_IS      AT91F_PIO_IsInputSet  (AT91C_BASE_PIOA, SDA_LINE)

//#define i2c_delay() __delay_cycles(1*F_MHZ)
//#define i2c_delay2() __delay_cycles(2*F_MHZ)
//#define i2c_delay5() __delay_cycles(3*F_MHZ)   

//*************************** public function *********************************

// init line
void i2c_init(void);

// set delay
void i2c_set_delay (int delay);
int i2c_get_delay (void);
int at24_write_16_byte (int adr, char * data);
int at24_read_16_byte (int adr, char * data);

// read one byte from bus
int i2c_read_byte (int, int);

// write one byte to bus
int i2c_write_byte (int, int, int);

// read array from bus
int i2c_read_string(int, int, char *, int);

// write array to bus
int i2c_write_string(int, int, char *, int);




#endif
