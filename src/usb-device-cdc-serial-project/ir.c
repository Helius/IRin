/*
module for hadling ir signal and recognise it
*/
#include <string.h>
#include <utility/trace.h>
#include <pio/pio.h>
#include "ir.h"

static const Pin ledGrn = PIN_LED_GRN;
//static const Pin dbg_pin   = PIN_DBG;

//*****************************************************************************
// calls once at startup, setup internal data
void ir_init (ir_t * this)
{
	this->state = _IR_IDLE;
	this->protocol = _UNDEF_PR;
	this->code = 0;
	this->ready_flag = false;
	this->led_cnt = 0;
}

//*****************************************************************************
// need to call 100us period
void ir_time_handler (ir_t * this)
{
	if (this->state != _IR_IDLE)
		this->timer++;

	if (this->state == _IR_REPEAT) {
		this->ready_flag = true;
		this->led_cnt = 100;
		PIO_Set (&ledGrn);
		this->state = _IR_IDLE;
		this->timer = 0;
	}

	if (this->timer > 150) {
		this->state = _IR_IDLE;
		this->timer = 0;
	}

	if (this->led_cnt > 0)
		this->led_cnt--;
	else
		PIO_Clear (&ledGrn);
}

//*****************************************************************************
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

//*****************************************************************************
// handler is called on each toggle ir data line level
void ir_line_handler (ir_t * this, int level)
{
	if ((this->state == _IR_IDLE) && (level == 1)) { // synchro first front
		this->timer=0;
		this->raw = 0;
		this->count = 0;
		this->state = _IR_SYNCHRO_UP;
//		PIO_Set (&dbg_pin);
	}	else if ((this->state == _IR_SYNCHRO_UP) && (!level) &&
						(this->timer < _TIME_SYNCH_HI_PULSE + 10) && 
						(this->timer > _TIME_SYNCH_HI_PULSE - 10)) {
//		PIO_Clear (&dbg_pin);
		this->timer = 0;
		this->state = _IR_SYNCHRO_DOWN;
	}	else if ((this->state == _IR_SYNCHRO_DOWN) && (level) &&
						(this->timer < _TIME_SYNCH_LO_PULSE + 10) && 
						(this->timer > _TIME_SYNCH_LO_PULSE - 10)) {
		this->timer = 0;
		this->state = _IR_RX_DATA;
//		PIO_Set (&dbg_pin);
	}	else if ((this->state == _IR_SYNCHRO_DOWN) && (level) &&
						(this->timer < _TIME_DATA_1_PULSE + 5) && 
						(this->timer > _TIME_DATA_1_PULSE - 5)) {
		this->state = _IR_REPEAT;
	}	else if (this->state == _IR_RX_DATA) {
		if (level) {
			this->raw >>= 1;
			if ((this->timer < _TIME_DATA_1_PULSE + 5) &&
					(this->timer > _TIME_DATA_1_PULSE - 5)) {
				this->raw	|= 0x80000000;
//				PIO_Set (&dbg_pin);
			} else if ((this->timer < _TIME_DATA_0_PULSE + 5) &&
								(this->timer > _TIME_DATA_0_PULSE - 5)) {
//				PIO_Clear (&dbg_pin);
			}
			this->count++;
			if (this->count > 31) {
				if(!accept_code (this)) {
					TRACE_DEBUG ("IR Reciev ERROR %x", this->raw);
				}
				this->state = _IR_IDLE;
//				PIO_Clear (&dbg_pin);
			}
			this->timer = 0;
		}
	}
}

//*****************************************************************************
int ir_code (ir_t * this)
{
	if (this->ready_flag) {
		this->ready_flag = false;
		return this->code;
	}
	return 0;
}

