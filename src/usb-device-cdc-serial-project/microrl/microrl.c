#include <stdio.h>
#include "microrl.h"
#include <string.h>
#include <ctype.h>

#define DBG(...) printf("\033[33m");printf(__VA_ARGS__);printf("\033[0m");

char * prompt_default = _PROMPT_DEFAUTL;


#define _HIST_UP 0
#define _HIST_DOWN 1

void print_hist (ring_history_t * this)
{
	printf ("\n");
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (i == this->begin)
			printf ("b");
		else 
			printf (" ");
	}
	printf ("\n");
	int header = this->begin;
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (i == header) {
			printf ("%d", this->ring_buf [header]);
			header = this->ring_buf [header] + header + 1;
			if (header >= _RING_HISTORY_LEN)
				header -= _RING_HISTORY_LEN;
		} else
			printf ("%c", this->ring_buf[i]);
	}
	printf ("\n");
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (i == this->end)
			printf ("e");
		else 
			printf (" ");
	}
	printf ("\n");
}

//*****************************************************************************
static void hist_erase_older (ring_history_t * this)
{
	DBG ("hist erase older\n");
	int new_pos = this->begin + this->ring_buf [this->begin] + 1;
	if (new_pos >= _RING_HISTORY_LEN)
		new_pos = new_pos - _RING_HISTORY_LEN;
	
	this->begin = new_pos;
}

//*****************************************************************************
static int hist_is_space_for_new (ring_history_t * this, int len)
{
	if (this->ring_buf [this->begin] == 0)
		return true;
	if (this->end >= this->begin) {
		DBG ("hist + there is %d byte", _RING_HISTORY_LEN - this->end + this->begin - 2);
		if (_RING_HISTORY_LEN - this->end + this->begin - 2 > len)
			return true;
	}	else {
		DBG ("hist - there is %d - %d - 1 = %d byte", this->begin, this->end,  this->begin - this->end - 1);
		if (this->begin - this->end - 1> len)
			return true;
	}
	return false;
}

//*****************************************************************************
static void hist_save_line (ring_history_t * this, char * line, int len)
{
	DBG ("\nsave to history %d byte\n", len);
	while (!hist_is_space_for_new (this, len)) {
		DBG ("hist no space for new\n");
		hist_erase_older (this);
	}
	// if it's first line
	if (this->ring_buf [this->begin] == 0)
		this->ring_buf [this->begin] = len;
	
	// store line
	memcpy (this->ring_buf + this->end + 1, line, len);

	this->ring_buf [this->end] = len;
	this->end = this->end + len + 1;
	if (this->end >= _RING_HISTORY_LEN)
		this->end -= _RING_HISTORY_LEN;
	this->ring_buf [this->end] = 0;
	this->cur = 0;
	print_hist (this);
}

//*****************************************************************************
static int hist_restore_line (ring_history_t * this, char * line, int dir)
{
	int cnt = 0;
	int header = this->begin;
	while (this->ring_buf [header] != 0) {
		header += this->ring_buf [header] + 1;
		cnt++;
	}	

	if (dir == _HIST_UP) {
//		DBG ("found %d in history\n", cnt);
		if (cnt >= this->cur) {
			int header = this->begin;
			int j = 0;

			while ((this->ring_buf [header] != 0) && (cnt - j -1 != this->cur)) {
				header += this->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
//				DBG ("header %d\n", header);
				j++;
			}
//			DBG ("restore %d len is %d\n", j, this->ring_buf[header]);
			if (this->ring_buf[header])
				this->cur++;
			memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
			return this->ring_buf[header];
		}
	} else {
		if (this->cur > 0) {
				this->cur--;
			int header = this->begin;
			int j = 0;

			while ((this->ring_buf [header] != 0) && (cnt - j != this->cur)) {
				header += this->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
//				DBG ("header %d\n", header);
				j++;
			}
//			DBG ("restore %d len is %d\n", j, this->ring_buf[header]);
			memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
			return this->ring_buf[header];
		}

	}
	return 0;
}



//*****************************************************************************
static char * find_zerro (char * str)
{
	while (*(str++) != '\0');
	return str++;
}

//*****************************************************************************
static int split (char * cmd_line, const char ** tkn_arr)
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
	this->cmdline[1] = '\n';
	memset(this->ring_hist.ring_buf, 0, _RING_HISTORY_LEN);
	this->ring_hist.begin = 0;
	this->ring_hist.end = 0;
	this->ring_hist.cur = 0;
	this->cmdlen =0;
	this->cursor = 0;
	this->execute = NULL;
	this->get_completion = NULL;
	this->prompt_str = prompt_default;
	this->print = print;
	print_prompt (this);
}

//*****************************************************************************
void microrl_set_complite_callback (microrl_t * this, char * (*get_completion)(int, const char* const*))
{
	this->get_completion = get_completion;
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
				
			this->cmdlen = hist_restore_line (&this->ring_hist, this->cmdline, _HIST_UP);
			if (this->cmdlen) {
			
				while (this->cursor>0) {
					terminal_backspace (this);
					this->cursor--;
				}

				int i = 0;
				char chn [2] = {0,0};
				while (i < this->cmdlen) {
					if (this->cmdline[i] != '\0')
						chn[0] = this->cmdline[i];
					else
						chn[0] = ' ';
					this->print (chn);
					i++;
					this->cursor++;
				}
			}

			return 1;
		} else if (ch == 'B') {
//			DBG ("Down");
			this->cmdlen = hist_restore_line (&this->ring_hist, this->cmdline, _HIST_DOWN);
			if (this->cmdlen) {
			
				while (this->cursor>0) {
					terminal_backspace (this);
					this->cursor--;
				}

				int i = 0;
				char chn [2] = {0,0};
				while (i < this->cmdlen) {
					if (this->cmdline[i] != '\0')
						chn[0] = this->cmdline[i];
					else
						chn[0] = ' ';
					this->print (chn);
					i++;
					this->cursor++;
				}
			}

			return 1;
			return 1;
		} else if (ch == 'C') {
			if (this->cursor < this->cmdlen) {
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
					while (this->cursor < this->cmdlen) {
						this->print ("\033[C");
						this->cursor++;
					}
				return 1;
			}
		
	}
	return 0;
}

//*****************************************************************************
void terminal_clear_line (microrl_t * this)
{
	this->print ("\033[K");
}

//*****************************************************************************
// print cmdline to screen, replace '\0' to wihitespace 
void terminal_print_line (microrl_t * this, int pos)
{
	terminal_clear_line (this);
	char nch [] = {0,0};
	int len = this->cmdlen;
	for (int i = pos; i < len; i++) {
		nch [0] = this->cmdline [i];
		if (nch[0] == '\0')
			nch[0] = ' ';
		this->print (nch);
	}
	for (int i = pos; i < len - 1; i++) {
		this->print ("\033[D");
	}
}

//*****************************************************************************
// insert len char of text at cursor position
static int microrl_insert_text (microrl_t * this, char * text, int len)
{
	if (this->cmdlen + len < _COMMAND_LINE_LEN - 2) {
		memmove (this->cmdline + this->cursor + len,
						 this->cmdline + this->cursor,
						 this->cmdlen - this->cursor + 2);
		for (int i = 0; i < len; i++) {
			this->cmdline [this->cursor + i] = text [i];
			if (this->cmdline [this->cursor + i] == ' ') {
				this->cmdline [this->cursor + i] = 0;
			}
		}
		this->cursor += len;
		this->cmdlen += len;
		return true;
	}
	return false;
}

//*****************************************************************************
void microrl_insert_char (microrl_t * this, int ch)
{
	static char prevch = 'a';
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
					if (this->execute (status, this->tkn_arr)) {
//						if (this->cmdlen > 0)
//							hist_save_line (&this->ring_hist, this->cmdline, this->cmdlen);
					}
				}
				print_prompt (this);
				this->cmdlen = 0;
				this->cursor = 0;
				memset(this->cmdline, 0, _COMMAND_LINE_LEN);
				this->ring_hist.cur = 0;
			
				break;
			//-----------------------------------------------------
			case KEY_HT:
			{
				char ** compl_token; 
				int status = split (this->cmdline, this->tkn_arr);
				if (this->get_completion != NULL) {
					compl_token = this->get_completion (status, this->tkn_arr);
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
				this->cmdlen = 0;
				break;
			//-----------------------------------------------------
			case KEY_DEL://TODO: rewrite it!
				if (this->cursor > 0) {
					terminal_backspace (this);
					this->print ("\033[K");

					memcpy (this->cmdline + this->cursor-1, this->cmdline + this->cursor, this->cmdlen - this->cursor+1);
					this->cmdline [this->cmdlen] ='\0';
					this->cursor--;
					this->cmdlen--;
					for (int i = this->cursor; i < this->cmdlen; i++) {
						char chn [2] = {0,0};
						chn [0] = this->cmdline [i];
						if (chn[0] == '\0') {
							chn[0] = ' ';
						}
						this->print (chn);
					}
					for (int i = this->cursor; i < this->cmdlen; i++) 
						this->print ("\033[D");
				}
				break;
			//-----------------------------------------------------
			default:
			if ((ch == ' ') && (this->cmdlen == 0)) {
				break;
			} else if ((ch == ' ') && (prevch == ' ')) {
				break;
			}
			prevch = ch;
			if (microrl_insert_text (this, (char*)&ch, 1))
				terminal_print_line (this, this->cursor-1);
			break;
		}
	}
}
