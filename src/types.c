/*
  This File is part of Boo's Chess Engine
  Copyright 1999 by Christopher Bowron
*/  
#include <stdlib.h>
#include <stdio.h>

#include "bce.h"

char *squarestring(square s)
{
    char *res;
    res=malloc(sizeof(char)*3);
    res[0] = FL(F(s));
    res[1] = RK(R(s));
    res[2] = 0;
    return res;
}

char *movestring(move m)
{
    static char res[6];
    square s, e;
    s=FR(m);
    e=TO(m);

    if (m==dummymove)
	sprintf(res, "xxxx");
    else
	sprintf(res, "%c%c%c%c",
		FL(F(s)),RK(R(s)),
		FL(F(e)),RK(R(e)));
    return res;
}

void printmoves(int c)
{
    int count = 0;
    move_and_score *restore_sp = move_sp;
    
    genmoves(c);
    
    while (move_sp>restore_sp)
    {
	move m = popmove();
	if (validmove(m)==1) 
	{
	    printf("%s  ", movestring(m));
	    count++;
	    if (count % 10 == 0)
		printf("\n");
	}
    }
    if (count % 10 != 0)
	printf("\n");

    move_sp = restore_sp;
}

void printbitboard(bitboard b)
{
    int f,r;

    printf("- %llx -\n", b);
    for (r=7;r>=0;r--)
    {
	for (f=0;f<8;f++)
	{
	    if (b & BIT(f,r))
		printf("1");
	    else
		printf("0");
	}
	printf("\n");
    }
}

