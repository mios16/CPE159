/* sample.C - Sample Main program for SPEDE 	Aug */

#include <spede/stdio.h>		// For printf()
#include <spede/flames.h>		// For cons_printf()

int main (void)
{
	long i;

	i = 111;
	while (i < 116)
	{
		DisplayMsg(i);
		i = i + 1;
	}
	return 0;
}     // end main()

int DisplayMsg(long i)
{
	printf( "%d Hello world %d \nECS\n", i, 2 * i);
	cons_printf( "--> Hello world <--\nCPE/CSC\n" );

}

