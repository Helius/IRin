/*
Microrl library config files
Autor: Eugene Samoylov aka Helius (ghelius@gmail.com)
*/
#ifndef _MICRORL_CONFIG_H_
#define _MICRORL_CONFIG_H_

/*********** CONFIG SECTION **************/

/*
Command line length, define max possible command buffer size here.
If user input chars more then it parametrs, chars not added to command line.*/
#define _COMMAND_LINE_LEN 24

/*
Command token number, define max token it command line, if number of token 
typed in command line exceed this value, then prints message about it and
command line not to be parced and 'execute' callback will not calls.
Token is word separate by white space, for example 3 token line:
"IRin> set mode test" */
#define _COMMAND_TOKEN_NMB 8

/*Define it, if you wanna use complite functional, and set complitetion callback,
now if user press TAB calls 'copmlitetion' callback. If you no need it, you can just set 
NULL to callback ptr and do not use it, but for memory saving tune, 
if you are not going to use it - disable this define.
*/
#define _USE_COMPLETE

/*Define it, if you wanna use history. It s work's like bash history, and
set stored value to cmdline, if UP and DOWN key pressed. Using history add
memory consuming, depends from _RING_HISTORY_LEN parametr */
#define _USE_HISTORY

/*
History ring buffer length, define static buffer size.
For saving memory, each entered cmdline store to history in ring buffer,
so we can not say, how many line we can store, it depends from cmdline len,
but memory using more effective. We not prefer dinamic memory allocation for
small and embedded devices.*/
#ifdef _USE_HISTORY
#define _RING_HISTORY_LEN 32
#endif

/*
Define you prompt string here. You can use colors escape code, for highlight you prompt,
for example this prompt will green color (if you terminal supports color)*/
#define _PROMPT_DEFAUTL "\033[32mIRin >\033[0m "
/*
Define prompt text (without ESC sequence, only text) prompt length, it needs because if you use
ESC sequence, it's not possible detect only text length*/
#define _PROMPT_LEN       7

/********** END CONFIG SECTION ************/

#endif
