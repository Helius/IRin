#ifndef _I2CSW_H_
#define _I2CSW_H_

#include "lib_AT91SAM7S64.h"

//#ifdef AT91SAM7S64
    #define __delay_cycles(x)   for ( int __jj=0; __jj<x; __jj++ ) {    \
            volatile int __tmp=0;                                       \
        }                                                      
//#endif

#define SCL_LINE    AT91C_PIO_PA8
#define SDA_LINE    AT91C_PIO_PA7

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

void i2c_init(void); ///< инициализация
int i2c_read_string(int, int, char *, int); ///< читать строку (массив)
int i2c_write_string(int, int, char *, int); ///< писать строку (массив)

#define I2C_PAGE_LEN 16
#define I2C_READ_BIT  1

#define F_MHZ 60

#define i2c_delay() __delay_cycles(1*F_MHZ)
#define i2c_delay2() __delay_cycles(2*F_MHZ)
#define i2c_delay5() __delay_cycles(3*F_MHZ)   

#endif
