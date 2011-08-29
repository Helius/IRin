#ifndef _MICRORL_H_
#define _MICRORL_H_

#define true  1
#define false 0

 /* define the Key codes */
#define KEY_NUL 0 /**< ^@ Null character */
#define KEY_SOH 1 /**< ^A Start of heading, = console interrupt */
#define KEY_STX 2 /**< ^B Start of text, maintenance mode on HP console */
#define KEY_ETX 3 /**< ^C End of text */
#define KEY_EOT 4 /**< ^D End of transmission, not the same as ETB */
#define KEY_ENQ 5 /**< ^E Enquiry, goes with ACK; old HP flow control */
#define KEY_ACK 6 /**< ^F Acknowledge, clears ENQ logon hand */
#define KEY_BEL 7 /**< ^G Bell, rings the bell... */
#define KEY_BS  8 /**< ^H Backspace, works on HP terminals/computers */
#define KEY_HT  9 /**< ^I Horizontal tab, move to next tab stop */
#define KEY_LF  10  /**< ^J Line Feed */
#define KEY_VT  11  /**< ^K Vertical tab */
#define KEY_FF  12  /**< ^L Form Feed, page eject */
#define KEY_CR  13  /**< ^M Carriage Return*/
#define KEY_SO  14  /**< ^N Shift Out, alternate character set */
#define KEY_SI  15  /**< ^O Shift In, resume defaultn character set */
#define KEY_DLE 16  /**< ^P Data link escape */
#define KEY_DC1 17  /**< ^Q XON, with XOFF to pause listings; "okay to send". */
#define KEY_DC2 18  /**< ^R Device control 2, block-mode flow control */
#define KEY_DC3 19  /**< ^S XOFF, with XON is TERM=18 flow control */
#define KEY_DC4 20  /**< ^T Device control 4 */
#define KEY_NAK 21  /**< ^U Negative acknowledge */
#define KEY_SYN 22  /**< ^V Synchronous idle */
#define KEY_ETB 23  /**< ^W End transmission block, not the same as EOT */
#define KEY_CAN 24  /**< ^X Cancel line, MPE echoes !!! */
#define KEY_EM  25  /**< ^Y End of medium, Control-Y interrupt */
#define KEY_SUB 26  /**< ^Z Substitute */
#define KEY_ESC 27  /**< ^[ Escape, next character is not echoed */
#define KEY_FS  28  /**< ^\ File separator */
#define KEY_GS  29  /**< ^] Group separator */
#define KEY_RS  30  /**< ^^ Record separator, block-mode terminator */
#define KEY_US  31  /**< ^_ Unit separator */

#define KEY_DEL 127 /**< Delete (not a real control character...) */


#define _HISTORY_LEVELS		10
#define _COMMAND_LINE_LEN 128
#define _COMMAND_TOKEN_NMB 8
#define _PROMPT_DEFAUTL "IRin > "

typedef struct {
	char * prompt_str;
	char cmdline [_COMMAND_LINE_LEN];
	int cmdpos;                           // last position in command line
	int cursor;
	char const * tkn_arr [_COMMAND_TOKEN_NMB];
	int (*execute) (int argc, const char * const * argv );
	char * (*get_complition) (int argc, const char * const * argv );	
	void (*print) (char *);
} microrl_t;

// init internal data, calls once at start up
void microrl_init (microrl_t * this, void (*print)(char*));

// set pointer to callback complition func, that called when user press 'Tab'
// param: argc - argument count, argv - pointer array to token string
// must return NULL-terminated string, contain complite variant splitted by 'Whitespace'
// If complite token found, it's must contain only one token to be complitted
// Empty string if complite not found, and multiple string if there are some token
void microrl_set_complite_callback (microrl_t * this, char * (*get_complition)(int, const char* const*));

// pointer to callback func, that called when user press 'Enter'
// param: argc - argument count, argv - pointer array to token string
void microrl_set_execute_callback (microrl_t * this, int (*execute)(int, const char* const*));

// insert char to cmdline (for example call in usart RX interrupt)
void microrl_insert_char (microrl_t * this, int ch);

#endif