#include <stdio.h>
#include "microrl.h"
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h> 

//*****************************************************************************
void print (char * str)
{
	fprintf (stderr, "%s", str);
}

//*****************************************************************************
int execute (int i, const char * const * tkn_arr)
{
	for (int i = 0; tkn_arr[i] != NULL; i++) {
		fprintf (stderr, "[%s] ", tkn_arr[i]);
	}
	printf ("\n");
}

//*****************************************************************************
// get char user pressed, no waiting Enter input
char get_char (void)
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

//*****************************************************************************
void main (int argc, char ** argv)
{
	// microrl object and pointer on it
	microrl_t rl;
	microrl_t * prl = &rl;
	int i;
	microrl_init (prl, print);
	microrl_set_execute_callback (prl, execute);
	
	while (1) {
		microrl_insert_char (prl, get_char());
	}
}
