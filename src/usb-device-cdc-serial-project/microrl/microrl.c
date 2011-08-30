#include <stdio.h>
#include "microrl.h"
#include <string.h>
#include <ctype.h>

#define DBG(...) printf("\033[33m");printf(__VA_ARGS__);printf("\033[0m");

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
	print_prompt (this);
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
int escape_process (microrl_t * this, char ch)
{
	static int seq = 0;

	if (ch == '[') {
		seq = _ESC_BRACKET;	
	} else if (seq == _ESC_BRACKET) {
		if (ch == 'A') {
//			DBG ("Up");
			return 1;
		} else if (ch == 'B') {
//			DBG ("Down");
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
		} else if (ch == '7') {
			seq = _ESC_HOME;
			return 0;
		} else if (ch == '8') {
			seq = _ESC_END;
			return 0;
		} 
	} else if (ch == '~') {
			if (seq == _ESC_HOME) {
				while (this->cursor>0) {
					this->print ("\033[D");
					this->cursor--;
				}
				return 1;
			} else if (seq == _ESC_END) {
					while (this->cursor < this->cmdpos) {
						this->print ("\033[C");
						this->cursor++;
					}
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

//	DBG (" (%c:%d) ", ch, ch);

	if (escape) {
		if (escape_process(this, ch))
			escape = 0;
	} else {
		switch (ch) {
			//-----------------------------------------------------
			case KEY_CR:
			case KEY_LF:
				terminal_newline (this);
				status = split (this->cmdline, this->tkn_arr);
				if (status == -1)
					this->print ("ERROR: Max command amount exseed\n");
				if ((status > 0) && (this->execute != NULL)) {
					this->execute (status, this->tkn_arr);
				}
				print_prompt (this);
				this->cmdpos = 0;
				this->cursor = 0;
				memset(this->cmdline, 0, _COMMAND_LINE_LEN);
			
				break;
			//-----------------------------------------------------
			case KEY_HT:
			{
				char ** compl_token; 
				int status = split (this->cmdline, this->tkn_arr);
				if (this->get_complition != NULL) {
					compl_token = this->get_complition (status, this->tkn_arr);
					int i = 0;
					while (compl_token [i] != NULL) {
						this->print (compl_token[i]);
						this->print (" ");
						i++;
					}
					terminal_newline (this);
					print_prompt (this);
					i = 0;
					char chn [2] = {0,0};
					while (i < this->cursor) {
						if (this->cmdline[i] != '\0')
							chn[0] = this->cmdline[i];
						else
							chn[0] = ' ';
						this->print (chn);
						i++;
					}
				}
			}
				break;
			//-----------------------------------------------------
			case KEY_ESC:
				escape = 1;
				break;
			//-----------------------------------------------------
			case KEY_NAK: // Ctrl+U
				while (this->cursor-- > 0) {
					terminal_backspace (this);
				}
				memset(this->cmdline, 0, _COMMAND_LINE_LEN);
				this->cmdpos = 0;
				break;
			//-----------------------------------------------------
			case KEY_DEL://TODO: rewrite it!
				if (this->cursor > 0) {
					terminal_backspace (this);
					this->print ("\033[K");

					memcpy (this->cmdline + this->cursor-1, this->cmdline + this->cursor, this->cmdpos - this->cursor+1);
					this->cmdline [this->cmdpos] ='\0';
					this->cursor--;
					this->cmdpos--;
					for (int i = this->cursor; i < this->cmdpos; i++) {
						char chn [2] = {0,0};
						chn [0] = this->cmdline [i];
						if (chn[0] == '\0') {
							chn[0] = ' ';
						}
						this->print (chn);
					}
					for (int i = this->cursor; i < this->cmdpos; i++) 
						this->print ("\033[D");
				}
				break;
			//-----------------------------------------------------
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
