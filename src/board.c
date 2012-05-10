/*
  This File is part of Boo's Chess Engine
  Copyright 2000 by Christopher Bowron
*/  
#include <stdio.h>
#include <string.h>

#include "bce.h"

void clearboard()
{
    int f, r;
    for (f=0;f<8;f++)
	for (r=0; r<8; r++)
	    setpiece(f,r,empty);
}

char rep(chesspiece piece)
{
    int v = chesspiecevalue(piece);
    int c = chesspiececolor(piece);
    
    if (c == WHITE)
	return " PNBRQK"[v];
    else if (c==BLACK)
	return " pnbrqk"[v];
    else
	return ' ';
}


void printboardbig()
{
    int f,r;
    move m;
    square end;
    int ef = 8, er = 8;
    
    if (ply)
    {
	m = lastmove();
	end = TO(m);
	ef = F(end);
	er = R(end);
    }
    
    printf("    a   b   c   d   e   f   g   h  \n");
    printf("  +---+---+---+---+---+---+---+---+\n");
    for (r=7;r>=0;r--)
    {
	printf("%d |", r+1);
	for (f=0;f<=7;f++)
	{
	    if (highlight&&(ef==f)&&(er==r))
		printf("*%c*|", rep( getpiece(f, r) ));
	    else
		printf(" %c |", rep( getpiece(f, r) ));		
	}
	printf(" %d \n", r+1);
	printf("  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    a   b   c   d   e   f   g   h  \n");
}

void printboardblind()
{
    /* do nothing */
}


void printboardsmall()
{
    int f,r;
    move m;
    square end;
    int ef = 8, er = 8;
    
    if (ply)
    {
	m = lastmove();
	end = TO(m);
	ef = F(end);
	er = R(end);
    }
    
    printf("   a b c d e f g h\n");
    printf("  +-+-+-+-+-+-+-+-+\n");
    for (r=7;r>=0;r--)
    {
	printf("%d |", r+1);
	for (f=0;f<=7;f++)
	{
	    printf("%c|", rep( getpiece(f, r) ));
	}
	printf(" %d\n", r+1);
    }
    printf("  +-+-+-+-+-+-+-+-+\n");    
    printf("   a b c d e f g h\n");
}


			
void setupboard()
{
    int f;
    int inits[] = { rook, knight, bishop, queen, king, bishop, knight, rook };
     
    clearboard();
    
    for (f=0; f<8; f++)
    {
      setpiece(f, RANK2, WPAWN);
      setpiece(f, RANK7, BPAWN);

      setpiece(f, RANK1, makepiece(WHITE,inits[f]));
      setpiece(f, RANK8, makepiece(BLACK,inits[f]));
    }

    board->kings[WHITE] = E1;
    board->kings[BLACK] = E8;
    
    countmaterial();
    board->flags=makevariables();
}

void countmaterial()
{
    int i;
    
    memset(board->position, 0, sizeof(board->position));
    memset(board->material, 0, sizeof(board->material));
    memset(board->pawns, 0, sizeof(board->pawns));
    memset(board->pieces,0, sizeof(board->pieces));
    memset(board->pawnbits,0,sizeof(board->pawnbits));

    
    
    board->piececount[0] = 0;
    board->piececount[1] = 0;
    
    for (i=0;i<64;i++)
    {
	chesspiece p = getpiece__(i);

	updatematerial__(i,p,1);

	if (p==BKING)
	    board->kings[BLACK] = i;
	if (p==WKING)
	    board->kings[WHITE] = i;
    }

    compute_hash();
}
