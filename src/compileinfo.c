#include <stdlib.h>
#include <stdio.h>

#include "bce.h"

void credits()
{
    char *str[] = 
    {
	"+-----",
	"| BACE - Bowron Abernethy Chess Engine",
	"| Written by Christopher Bowron - bowronch@cse.msu.edu",
	"| $Revision: 4.6 $ $Date: 2001/07/05 22:05:21 $",
	"| Compiled " __DATE__ ", " __TIME__,
	"| http://www.cse.msu.edu/~bowronch/BACE",
	"|",
#ifdef LEARNING	
	"| Learning: On",
#else
	"| Learning: Off",
#endif	
	"+-----",
	0
    };

    int i;
    for (i=0;str[i]!=NULL;i++)
    {
	printf("%s\n", str[i]);
    }
}

