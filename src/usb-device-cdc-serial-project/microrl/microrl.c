#include <stdio.h>
#include "microrl.h"
//#include <utility/trace.h>
#include <string.h>
#include <ctype.h>

static const char * prompt_default = _PROMPT_DEFAUTL;

//*****************************************************************************
char * find_zerro (char * str)
{
	while (*(str++) != '\0') {
	}
	return str++;
}

//*****************************************************************************
int split (char * cmd_line, const char ** tkn_arr)
{
	char * pos = cmd_line;
	int i = 0;
	while (*pos != '\0') {
		tkn_arr[i++] = pos;
		if (i > _COMMAND_TOKEN_NMB) {
			return -1;
		}
		pos = find_zerro (pos);
	}
	return i;
}

//*****************************************************************************
static void print_prompt (microrl_t * this)
{
	this->print (this->prompt_str);
}

//*****************************************************************************
static void terminal_backspace (microrl_t * this)
{
		this->print ("\033[D \033[D");
}

//*****************************************************************************
static void terminal_newline (microrl_t * this)
{
	this->print ("\n\r");
}

//*****************************************************************************
void microrl_init (microrl_t * this, void (*print) (char *)) 
{
	memset(this->cmdline, 0, _COMMAND_LINE_LEN);
	this->cmdpos =0;
	this->cursor = 0;
	this->execute = NULL;
	this->get_complition = NULL;
	this->prompt_str = prompt_default;
	this->print = print;
}

//*****************************************************************************
void microrl_set_complite_callback (microrl_t * this, char * (*get_complition)(int, const char* const*))
{
	this->get_complition = get_complition;
}

//*****************************************************************************
void microrl_set_execute_callback (microrl_t * this, int (*execute)(int, const char* const*))
{
	this->execute = execute;
}

#define _CURSOR_MOVE 1

//*****************************************************************************
int escape_process (microrl_t * this, char ch)
{
	static int seq = 0;

	if (ch == '[') {
		seq = _CURSOR_MOVE;	
	} else if (seq == _CURSOR_MOVE) {
		if (ch == 'A') {
//			printf ("Up");
			return 1;
		} else if (ch == 'B') {
//			printf ("Down");
			return 1;
		} else if (ch == 'C') {
			if (this->cursor < this->cmdpos) {
				this->print ("\033[C");
				this->cursor++;
			}
			return 1;
		} else if (ch == 'D') {
			if (this->cursor > 0) {
				this->cursor--;
				this->print ("\033[D");
			}
			return 1;
		} else {
			return 1;
		}
	}
	return 0;
}

//*****************************************************************************
void microrl_insert_char (microrl_t * this, int ch)
{
	char nch [3];
	int status;
	static int escape = false;

//	printf (" (%c:%d) ", ch, ch);
//	return;

	if (escape) {
		if (escape_process(this, ch)) {
			escape = 0;
		}
	} else {
		switch (ch) {
			case KEY_CR:
			case KEY_LF:
				terminal_newline (this);
				status = split (this->cmdline, this->tkn_arr);
	//			printf ("status: %d\n", status);
				if (status == -1)
					printf ("ERROR: Max command amount is %d\n", _COMMAND_TOKEN_NMB);
				if ((status > 0) && (this->execute != NULL)) {
	//				printf ("\t\tcall execute\n");
					this->execute (status, this->tkn_arr);
				}
				print_prompt (this);
				this->cmdpos = 0;
				this->cursor = 0;
				memset(this->cmdline, 0, _COMMAND_LINE_LEN);
			
				break;
			case KEY_HT:
	//			char ** compl_token; 
	//			//TODO: call callback, if not NULL
	//			if (get_complition != NULL) {
	//				compl_token = get_complition ();
	//			}
				break;
			case KEY_ESC:
				escape = 1;
				break;
			case KEY_NAK:
				printf ("<<<");
				break;
			case KEY_DEL:
				if (this->cursor > 0) {
					terminal_backspace (this);
					this->print ("\033[K");
					this->cursor--;
					for (int i = 0; i < this->cmdpos - this->cursor; i++) {
						this->cmdline [i + this->cursor] = this->cmdline [i + 1 + this->cursor]; 
					}
				}
				break;
			default:
				if (this->cmdpos < _COMMAND_LINE_LEN - 1) {

					if (ch == ' ') {
						// no spaces at begin of line, and no twice space
						if ((this->cmdpos == 0) || (this->cmdline [this->cmdpos-1] == '\0'))
							break;
						this->cmdline[this->cmdpos++] = '\0';
						this->cursor++;
					} else {
						this->cmdline[this->cmdpos++] = ch;
						this->cursor++;
					}
					this->cmdline[this->cmdpos] = '\0';
				
					nch[0] = ch;
					nch[1] = '\0';
					this->print (nch);
				}
				break;
		}
	}
}
