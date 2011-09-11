#ifndef _IR_H_
#define _IR_H_

#define true	1
#define false 0

#define _DATA_LEN					4

#define _IR_IDLE					0
#define _IR_SYNCHRO_UP		1
#define _IR_SYNCHRO_DOWN	2
#define _IR_RX_DATA				3
#define _IR_REPEAT				4

#define _UNDEF_PR					0
#define _NEC_PR						1

#define _DEF_REPEAT_DELAY 500

//for NEC protocol (in 100us ticks)
#define _TIME_SYNCH_HI_PULSE	90
#define _TIME_SYNCH_LO_PULSE	45
#define _TIME_DATA_1_PULSE		22
#define _TIME_DATA_0_PULSE		11

// ir receiver object type
typedef struct {
	int protocol;
	int state;
	int count;
	int timer;
	unsigned int raw;
	unsigned int code; // at least 16 bit on your platform!!
	int receive_flag;
	int repeat_flag;
	int repeat_delay;
	int repeat_time;
	int led_cnt;
} ir_t;

// init of ir lib
void ir_init (ir_t *);
// call if ir receiver line level changed
void ir_line_handler (ir_t*, int);
// call periodically at 100 us for internal timings
void ir_time_handler (ir_t*);
// return key code or 0 if no key pressed
int  ir_code (ir_t*);
// set/get repeate key press delay 
void ir_set_repeat_delay (ir_t * this, int delay);
int  ir_get_repeat_delay (ir_t * this);


#endif
