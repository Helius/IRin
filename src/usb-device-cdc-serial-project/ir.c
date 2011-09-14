/*
module for hadling ir signal and recognise it
*/
#include <string.h>
#include <utility/trace.h>
#include <pio/pio.h>
#include "ir.h"

static const Pin ledGrn = PIN_LED_GRN;
static const Pin dbg_pin   = PIN_DBG;

//*****************************************************************************
// calls once at startup, setup internal data
void ir_init (ir_t * this)
{
	this->state = _IR_IDLE;
	this->protocol = _UNDEF_PR;
	this->code = 0;
	this->receive_flag = false;
	this->repeat_flag = false;
	this->led_cnt = 0;
	this->repeat_delay = _DEF_REPEAT_DELAY;
	this->repeat_time = 0;
}

//*****************************************************************************
// need to call 100us period for ir internal timing clock
void ir_time_handler (ir_t * this)
{
	this->repeat_time++;
	if (this->state != _IR_IDLE)
		this->timer++;

	if (this->state == _IR_REPEAT) {
		this->repeat_flag = true;
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
	cmd[0] = 0xFF & (this->raw >> 16);
	cmd[1] = 0xFF & (this->raw >> 24);
	//check command
	if (cmd[0] == (cmd[1]^0xFF)) {
		this->code = this->raw & (0xFFFFFF);
		this->receive_flag = true;
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
		this->timer = 0;
		this->raw = 0;
		this->count = 0;
		this->state = _IR_SYNCHRO_UP;
//		PIO_Set (&dbg_pin);
	}	else if ((this->state == _IR_SYNCHRO_UP) && (!level) &&
						(this->timer < _TIME_SYNCH_LO_PULSE + 6) && 
						(this->timer > _TIME_SYNCH_LO_PULSE - 6)) {
		this->timer = 0;
		this->state = _IR_SYNCHRO_DOWN;
//		PIO_Clear (&dbg_pin);
	}	else if ((this->state == _IR_SYNCHRO_DOWN) && (level) &&
						(this->timer < _TIME_SYNCH_LO_PULSE + 6) && 
						(this->timer > _TIME_SYNCH_LO_PULSE - 6)) {
		this->timer = 0;
		this->state = _IR_RX_DATA;
	}	else if ((this->state == _IR_SYNCHRO_DOWN) && (level) &&
						(this->timer < _TIME_DATA_1_PULSE + _DATA_LAPS) && 
						(this->timer > _TIME_DATA_1_PULSE - _DATA_LAPS) &&
						(this->repeat_time > this->repeat_delay*10)) {
		this->repeat_time = 0;
		this->state = _IR_REPEAT;
		this->timer = 0;
	}	else if (this->state == _IR_RX_DATA) {
		if (level) {
			this->raw >>= 1;
			if ((this->timer < _TIME_DATA_1_PULSE + _DATA_LAPS) &&
					(this->timer > _TIME_DATA_1_PULSE - _DATA_LAPS)) {
				this->raw	|= 0x80000000;
			} else if ((this->timer < _TIME_DATA_0_PULSE + _DATA_LAPS) &&
								(this->timer > _TIME_DATA_0_PULSE - _DATA_LAPS)) {
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
	}
}

//*****************************************************************************
// return key code or repeated key code, 0 if there is no new key
int ir_code (ir_t * this)
{
	if (this->receive_flag) {
		this->receive_flag = false;
		this->repeat_time = 0;
		return this->code;
	} 
	
	if (this->repeat_flag) {
		this->repeat_flag = false;
		this->repeat_time = 0;
		return this->code;
	}
	return 0;
}

//*****************************************************************************
int ir_get_repeat_delay (ir_t * this)
{
	return this->repeat_delay;
}

//*****************************************************************************
void ir_set_repeat_delay (ir_t * this, int delay)
{
	if ((delay < 5000) && (delay > 50))
		this->repeat_delay = delay;
	else
		this->repeat_delay = _DEF_REPEAT_DELAY;
}

