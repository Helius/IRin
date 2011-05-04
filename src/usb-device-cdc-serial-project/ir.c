/*
module for hadling ir signal and recognise it
*/
#include <string.h>
#include <utility/trace.h>
#include <pio/pio.h>
#include "ir.h"

static const Pin ledGrn = PIN_LED_GRN;
//*****************************************************************************
// calls once at startup, setup internal data
void ir_init (ir_t * this)
{
	this->state = _IR_IDLE;
	this->protocol = _UNDEF_PR;
	this->code = 0;
	this->ready_flag = false;
}

//*****************************************************************************
// need to call 100us period
void ir_time_handler (ir_t * this)
{
	if (this->state != _IR_IDLE)
		this->timer++;
	if (this->timer > 200) 
		this->state = _IR_IDLE;
	if (this->led_cnt > 0) {
		this->led_cnt--;
	} else {
		PIO_Clear (&ledGrn);
	}
}

static int accept_code (ir_t * this)
{
	unsigned char cmd[2];
//	TRACE_DEBUG ("raw([!cmd][cmd][adr_h][adr_l]): %x", this->raw);
	cmd[0] = 0xFF & (this->raw >> 16);
	cmd[1] = 0xFF & (this->raw >> 24);
	//check command
	if (cmd[0] == (cmd[1]^0xFF)) {
		this->code = this->raw & (0xFFFFFF);
		this->ready_flag = true;
//		TRACE_DEBUG ("\t[cmd][adr_h][adr_l] %x", this->code);
		PIO_Set (&ledGrn);
		this->led_cnt = 1000;
		return true;
	}
	return false;
}

static const Pin pinToggle = PIN_LED_DS4;
//*****************************************************************************
// handler is called on each toggle ir data line level
void ir_line_handler (ir_t * this, int level)
{
	if ((this->state == _IR_IDLE) && (level)) { // synchro first front
		PIO_Set (&pinToggle);
		this->state = _IR_SYNCHRO_UP;
		this->raw = 0;
		this->count = 0;
		this->timer = 0;
	}	else if ((this->state == _IR_SYNCHRO_UP) && (!level) && 
						(this->timer < _TIME_SYNCH_HI_PULSE + 10) && 
						(this->timer > _TIME_SYNCH_HI_PULSE - 10)) {
		this->state = _IR_SYNCHRO_DOWN;
		this->timer = 0;
	}	else if ((this->state == _IR_SYNCHRO_DOWN) && (level) &&
						(this->timer < _TIME_SYNCH_LO_PULSE + 10) && 
						(this->timer > _TIME_SYNCH_LO_PULSE - 10)) {
		this->state = _IR_RX_DATA;
		this->timer = 0;
	}	else if ((this->state == _IR_RX_DATA)) {

		if (level) {
		this->raw >>= 1;
			if ((this->timer < _TIME_DATA_1_PULSE + 4) &&
					(this->timer > _TIME_DATA_1_PULSE - 4)) {
//				PIO_Set (&pinToggle);
				this->raw	|= 0x80000000;
			} else if ((this->timer < _TIME_DATA_0_PULSE + 4) &&
								(this->timer > _TIME_DATA_0_PULSE - 4)) {
//				PIO_Clear (&pinToggle);
			}
			this->count++;
			if (this->count > 31) {
				if(!accept_code (this)) {
					TRACE_DEBUG ("IR Reciev ERROR %x", this->raw);
				}
				this->state = _IR_IDLE;
			}
			this->timer = 0;
		}
	} else {
	}
}

//*****************************************************************************
int ir_code (ir_t * this)
{
	if (this->ready_flag) {
		this->ready_flag = false;
		PIO_Set (&pinToggle);
		return this->code;
	}
	return 0;
}

