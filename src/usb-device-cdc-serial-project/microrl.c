#include <stdio.h>
#include "microrl.h"
//#include <utility/trace.h>
#include <string.h>
#include <ctype.h>

static const char * prompt_default = _PROMPT_DEFAUTL;

//*****************************************************************************
int noempty (char * str)
{
	int len = strlen (str);
	if (len == 0)
		return false;
	for (int i = 0; i < len; i++) {
		if (str[i] != ' ') {
			return true;
		}
	}
	return false;
}

//*****************************************************************************
// return ptr to not_whitespace char or NULL if str isn't contain one
// replace whitescapce to '\0'
char * find_not_witespace (char * str)
{
	if ((str == NULL) || (*str == '\0'))
		return NULL;
	int i = 0;
	while (isspace(*(str + i))) {
		*(str + i) = '\0';
		i++;
	}
	if (*(str + i) == '\0') {
		return NULL;
	}
	return (str + i);
}

//*****************************************************************************
// return ptr to whitescapce char or NULL if str isn't contain one
char * find_whitespace (char * str)
{
	if ((str == NULL) || (*str == '\0')) {
		return NULL;
	}
	int i = 0;
	while ((*(str + i) != '\0') && (!isspace(*(str + i)))) {
		i++;
	}
	if (*(str + i) == '\0') 
		return NULL;
	if (i == 0)
		return NULL;
	return (str + i);
}

//*****************************************************************************
// split cmd line on token, removed white space
int split (const char * cmd_line, const char ** tkn_arr)
{
	char * tkn = cmd_line;
	int i = 0;
	do {
		tkn = find_not_witespace (tkn);
		tkn_arr [i++] = tkn;
		if (i == _COMMAND_TOKEN_NMB)
			return -1;
		if (tkn != NULL) {
			tkn = find_whitespace (tkn);
		}
	} while (tkn != NULL);
	printf ("{%d}",i);
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
	switch (ch) {
		case KEY_CR:
		case KEY_LF:
			terminal_newline (this);
			this->cmdpos = 0;
			if (split (this->cmdline, this->tkn_arr) && (this->execute != NULL)) {
				if (this->execute (0, this->tkn_arr)) {
					this->print ("Command not found, use 'help'");
				}
			}
			terminal_newline (this);
			print_prompt (this);
			this->cmdline[0] = 0;
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
