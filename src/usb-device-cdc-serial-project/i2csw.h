#ifndef _I2CSW_H_
#define _I2CSW_H_

#include "lib_AT91SAM7S64.h"

//******************************** set up **************************************
// define data and clock line
#define SCL_LINE    AT91C_PIO_PA8
#define SDA_LINE    AT91C_PIO_PA7

// clock for delay
#define F_MHZ				60

#define I2C_RW_BIT  1

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

//*************************** public function *********************************

// init line
void i2c_init(void);
// set delay
void i2c_set_delay (int delay);
// get delay
int  i2c_get_delay (void);
// start condition
void i2c_start (void);
// stop condition
void i2c_stop (void);
// put byte on bus
int i2c_putbyte (int data);
// get bute from bus
int i2c_getbyte (int ack);


//// read one byte from bus
//int i2c_read_byte (int, int);
//// write one byte to bus
//int i2c_write_byte (int, int, int);
//// read array from bus
//int i2c_read_string(int, int, char *, int);
//// write array to bus
//int i2c_write_string(int, int, char *, int);




#endif
