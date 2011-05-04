#include <stdio.h>
#include "microrl.h"
#include <utility/trace.h>
#include <string.h>

static const char * prompt_default = _PROMPT_DEFAUTL;

//*****************************************************************************
static void print_prompt (microrl_t * this)
{
	this->print (this->prompt_str);
}
//*****************************************************************************
static void terminal_newline (microrl_t * this)
{
	this->print ("\n\r");
}

//*****************************************************************************
void microrl_init (microrl_t * this, void (*print) (char *)) 
{
	this->cmdline[0] = '\0';
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

//*****************************************************************************
void microrl_insert_char (microrl_t * this, int ch)
{
	char nch [2];
	switch (ch) {
		case KEY_CR:
		case KEY_LF:
			//TODO: if cmdline no empty - execute callback
			terminal_newline (this);
			print_prompt (this);
			break;
		case KEY_HT:
//			char ** compl_token; 
//			//TODO: call callback, if not NULL
//			if (get_complition != NULL) {
//				compl_token = get_complition ();
//			}
			break;
		default:
			if (this->cmdpos < _COMMAND_LINE_LEN) {
				nch[0] = ch;
				nch[1] = '\0';
				this->print (nch);
				strcat (this->cmdline, nch);
				this->cmdpos++;
			}
			break;
	}
}
