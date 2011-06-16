#include <stdio.h>
#include "microrl.h"
//#include <utility/trace.h>
#include <string.h>
#include <ctype.h>

static const char * prompt_default = _PROMPT_DEFAUTL;

//*****************************************************************************
// return ptr to not_whitespace char or NULL if str isn't contain one
// replace whitescapce to '\0'
char * find_not_witespace (char * str)
{
	int i = 0;
	
	if ((str == NULL) || (*str == '\0'))
		return NULL;
	while (isspace(*(str + i))) {
		*(str + i) = '\0';
		i++;
	}
	if (*(str + i) == '\0') 
		return NULL;
	return (str + i);
}

//*****************************************************************************
// return ptr to whitescapce char or NULL if str isn't contain one
char * find_whitespace (char * str)
{
	int i = 0;

	if ((str == NULL) || (*str == '\0')) 
		return NULL;
	while ((*(str + i) != '\0') && (!isspace(*(str + i)))) {
		i++;
	}
	if ((*(str + i) == '\0') || (i == 0))
		return NULL;
	return (str + i);
}

//*****************************************************************************
// split cmd line on token, removed white space
//TODO: rewrite it, static array fo ptr to substring is not cool...
int split (char * cmd_line, const char ** tkn_arr)
{
	int i = 0;
	do {
		cmd_line = find_not_witespace (cmd_line);
		tkn_arr [i++] = cmd_line;
		tkn_arr [i] = NULL;
		if (i == _COMMAND_LINE_LEN) 
			return i;
		cmd_line = find_whitespace (cmd_line);
	} while (cmd_line != NULL);

	return i;
}

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
	this->cmdpos =0;
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
	char nch [3];
	int status;
	switch (ch) {
		case KEY_CR:
		case KEY_LF:
			terminal_newline (this);
			status = split (this->cmdline, this->tkn_arr);
			printf ("status: %d\n", status);
			if (status == -1)
				printf ("ERROR: Max command amount is %d\n", _COMMAND_TOKEN_NMB);
			if ((status > 0) && (this->execute != NULL)) {
				printf ("\t\tcall execute\n");
				this->execute (0, this->tkn_arr);
			}
			print_prompt (this);
			this->cmdpos = 0;
			this->cmdline[0] = '\0';
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
