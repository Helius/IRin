/*
Autor: Samoylov Eugene aka Helius (ghelius@gmail.com)
BUGS and TODO:
*) if HOME press on not empty cmdline, and input char, cursor jump to end of cmdline
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "microrl.h"

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
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (isalpha(this->ring_buf[i]))
			printf ("%c", this->ring_buf[i]);
		else 
			printf ("%d", this->ring_buf[i]);
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
		if (_RING_HISTORY_LEN - this->end + this->begin - 1 > len)
			return true;
	}	else {
		if (this->begin - this->end - 1> len)
			return true;
	}
	return false;
}

//*****************************************************************************
static void hist_save_line (ring_history_t * this, char * line, int len)
{
//	DBG ("\nsave to history %d byte\n", len);
	while (!hist_is_space_for_new (this, len)) {
//		DBG ("hist no space for new\n");
		hist_erase_older (this);
	}
	// if it's first line
	if (this->ring_buf [this->begin] == 0) 
		this->ring_buf [this->begin] = len;
	
	// store line
	if (len < _RING_HISTORY_LEN-this->end-1)
		memcpy (this->ring_buf + this->end + 1, line, len);
	else {
		int part_len = _RING_HISTORY_LEN-this->end-1;
		memcpy (this->ring_buf + this->end + 1, line, part_len);
		memcpy (this->ring_buf, line + part_len, len - part_len);
	}
	this->ring_buf [this->end] = len;
	this->end = this->end + len + 1;
	if (this->end >= _RING_HISTORY_LEN)
		this->end -= _RING_HISTORY_LEN;
	this->ring_buf [this->end] = 0;
	this->cur = 0;
//	DBG ("\nbegin %d, end %d\n", this->begin,  this->end);
	print_hist (this);
}

//*****************************************************************************
// copy saved line to 'line' and return size of line
static int hist_restore_line (ring_history_t * this, char * line, int dir)
{
	int cnt = 0;
	// count history record	
	int header = this->begin;
	while (this->ring_buf [header] != 0) {
		header += this->ring_buf [header] + 1;
		if (header >= _RING_HISTORY_LEN)
			header -= _RING_HISTORY_LEN; 
		cnt++;
	}

	if (dir == _HIST_UP) {
//		DBG ("found %d in history\n", cnt);
		if (cnt >= this->cur) {
			int header = this->begin;
			int j = 0;
			// found record for 'this->cur' index
			while ((this->ring_buf [header] != 0) && (cnt - j -1 != this->cur)) {
				header += this->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
				j++;
			}
//			DBG ("use header %d, %d value %d\n", j, header, this->ring_buf[header]);
//			DBG ("restore %d len is %d\n", j, this->ring_buf[header]);
			if (this->ring_buf[header]) {
					this->cur++;
				// obtain saved line
				if (this->ring_buf [header] + header < _RING_HISTORY_LEN) {
					memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
				} else {
					int part0 = _RING_HISTORY_LEN - header - 1;
	//				DBG ("first part is %d\n", part0);
					memcpy (line, this->ring_buf + header + 1, part0);
					memcpy (line + part0, this->ring_buf, this->ring_buf[header] - part0);
				}
				return this->ring_buf[header];
			}
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
			if (this->ring_buf [header] + header < _RING_HISTORY_LEN) {
				memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
			} else {
				int part0 = _RING_HISTORY_LEN - header - 1;
//				DBG ("first part is %d\n", part0);
				memcpy (line, this->ring_buf + header + 1, part0);
				memcpy (line + part0, this->ring_buf, this->ring_buf[header] - part0);
			}
//			memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
			return this->ring_buf[header];
		}

	}
	return 0;
}

//*****************************************************************************
static int split (microrl_t * this)
{
	int i = 0;
	int ind = 0;
	while (1) {
		while ((this->cmdline [ind] == '\0') && (ind < this->cmdlen)) {
			ind++;
		}
		if (!(ind < this->cmdlen)) return i;
		this->tkn_arr[i++] = this->cmdline + ind;
		if (i >= _COMMAND_TOKEN_NMB) {
			return -1;
		}
		while ((this->cmdline [ind] != '\0') && (ind < this->cmdlen)) {
			ind++;
		}
		if (!(ind < this->cmdlen)) return i;
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

static void terminal_set_cursor (microrl_t * this, int offset)
{
	char str[16];
	if (offset > 0) {
		snprintf (str, 12, "\033[%dC", offset);
		this->print (str);
	}	else if (offset < 0) {
		snprintf (str, 12, "\033[%dD", abs(offset));
		this->print (str);
	}
}

//*****************************************************************************
// print cmdline to screen, replace '\0' to wihitespace 
void terminal_print_line (microrl_t * this, int offset)
{
//	this->print ("\033[s");
	this->print ("\033[100D"); //TODO: 100 is magic
	this->print ("\033[7C");   //TODO: set cursor position after prompt
	this->print ("\033[K");
	char nch [] = {0,0};
	int len = this->cmdlen;
	for (int i = 0; i < len; i++) {
		nch [0] = this->cmdline [i];
		if (nch[0] == '\0')
			nch[0] = ' ';
		this->print (nch);
	}
	this->print ("\033[100D"); //TODO: 100 is magic
	this->print ("\033[7C");   //TODO: set cursor position after prompt
//	this->print ("\033[u");
//	if (offset > 0)
//		this->print ("\033[C");
	terminal_set_cursor (this, offset);
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
			int len = hist_restore_line (&this->ring_hist, this->cmdline, _HIST_UP);
			if (len) {
				this->cursor = this->cmdlen = len;
				terminal_print_line (this, this->cursor);
			}
			return 1;
		} else if (ch == 'B') {
			int len = hist_restore_line (&this->ring_hist, this->cmdline, _HIST_DOWN);
			if (len) {
				this->cursor = this->cmdlen = len;
				terminal_print_line (this, this->cursor);
			}
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
// insert len char of text at cursor position
static int microrl_insert_text (microrl_t * this, char * text, int len)
{
	if (this->cmdlen + len < _COMMAND_LINE_LEN - 1) {
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
// remove one char at cursor
static void microrl_backspace (microrl_t * this)
{
	if (this->cursor > 0) {
		terminal_backspace (this);
		memmove (this->cmdline + this->cursor-1,
						 this->cmdline + this->cursor,
						 this->cmdlen-this->cursor+1);
		this->cursor--;
		this->cmdline [this->cmdlen] = '\0';
		this->cmdlen--;
		terminal_print_line (this, this->cursor);
	}
}

//*****************************************************************************
void microrl_insert_char (microrl_t * this, int ch)
{
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
				status = split (this);
				if (status == -1)
					this->print ("ERROR: Max command amount exseed\n");
				if ((status > 0) && (this->execute != NULL)) {
					if (this->execute (status, this->tkn_arr)) {
						if (this->cmdlen > 0)
							hist_save_line (&this->ring_hist, this->cmdline, this->cmdlen);
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
				int status = split (this);
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
					terminal_print_line (this, 0);
					for (int i = 0; i < this->cursor; i++)
						this->print("\033[C");
				}
			}
				break;
			//-----------------------------------------------------
			case KEY_ESC:
				escape = 1;
				break;
			//-----------------------------------------------------
			case KEY_NAK: // Ctrl+U
					while (this->cursor > 0) {
					microrl_backspace (this);
				}
				break;
			//-----------------------------------------------------
			case KEY_DEL: // Backspace
				microrl_backspace (this);
			break;
			//-----------------------------------------------------
			default:
			if ((ch == ' ') && (this->cmdlen == 0)) 
				break;
			if (microrl_insert_text (this, (char*)&ch, 1))
				terminal_print_line (this, this->cursor);
			break;
		}
	}
}
