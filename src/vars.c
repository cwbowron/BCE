/*
  This File is part of Boo's Chess Engine
  Copyright 2000 by Christopher Bowron
*/  

#include <stdlib.h>

#include "bce.h"

chessboard *board = NULL;
book_and_table mem;

int promopiece	 = queen;
int midgamedepth = 10;
int endgamedepth = 30;
int searchdepth;
long int searchtime   = 12000;	/* number of seconds*1000 to limit search */

int highlight    = 1;		/* hightlight last move in printboard? */
int book_mode    = 1;		/* use book? */
int explain_mode = 0;		/* tell our owner our thoughts on server? */
int xboard_mode  = 0;		/* are we under xboard? */
int robo_mode    = 0;		/* are we under robofics? */
int ponder_mode  = 0;		/* think on oppenents time? */
int thinking_mode= 1;		/* display our thoughts? */

char *bookfile   = "book.txt";
char *weightfile = "bce.weights";

int computer[2],clockinc;
long chessclock[2];

hashtype randoms[BKING+1][64];

void (*printboard)() = printboardbig;

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

const int knightdx[] = { 2, -2,  2, -2, 1,  1, -1, -1 };
const int knightdy[] = { 1,  1, -1, -1, 2, -2,  2, -2 };

int whitepawnsquares[64] =
{
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0, -50, -50,   0,   0,   0,
      0,  10,  20,  30,  30,  20,  10,   0,
     20,  30,  50,  80,  80,  50,  30,  20,
     30,  50,  50, 100, 100,  50,  50,  30,
    200, 250, 300, 300, 300, 300, 250, 200,
    300, 500, 500, 600, 600, 500, 500, 300,
      0,   0,   0,   0,   0,   0,   0,   0
};

int blackpawnsquares[64]=
{
      0,   0,   0,   0,   0,   0,   0,   0,
    300, 500, 500, 600, 600, 500, 500, 300,
    200, 250, 300, 300, 300, 300, 250, 200,
     30,  50,  50, 100, 100,  50,  50,  30,
     20,  30,  50,  80,  80,  50,  30,  20,
      0,  10,  20,  30,  30,  20,  10,  0,
      0,   0,   0, -50, -50,   0,   0,  0,
      0,   0,   0,   0,   0,   0,   0,  0,
};

int knightsquares[64]=
{
      00,  30,  70, 100, 100,  70,  30,  00 ,
      30,  70, 100, 130, 130, 100,  70,  30 ,
      70, 100, 130, 160, 160, 130, 100,  70 ,
     100, 130, 160, 200, 200, 160, 130, 100 ,
     100, 130, 100, 200, 200, 160, 130, 100 ,    
      70, 100, 130, 160, 160, 130, 100,  70 ,
      30,  70, 100, 130, 130, 100,  70,  30 ,
      00,  30,  70, 100, 100,  70,  30,  00 
};

int bishopsquares[64]=
{
     140, 140, 140, 140, 140, 140, 140, 140,
     140, 220, 180, 180, 180, 180, 220, 140,
     140, 180, 220, 220, 220, 220, 180, 140,
     140, 180, 220, 220, 220, 220, 180, 140,
     140, 180, 220, 220, 220, 220, 180, 140,
     140, 180, 220, 220, 220, 220, 180, 140,
     140, 220, 180, 180, 180, 180, 220, 140,
     140, 140, 140, 140, 140, 140, 140, 140
};

int rooksquares[64]=
{
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0
};

int queensquares[64]=
{
      00,  50, 100, 200, 200, 100,  50,  00,
      50, 100, 150, 200, 200, 150, 100,  50,
     100, 150, 200, 250, 250, 200, 150, 100,
     150, 200, 250, 300, 300, 250, 200, 150,
     150, 200, 250, 300, 300, 250, 200, 150,
     100, 150, 200, 250, 250, 200, 150, 100,
      50, 100, 150, 200, 200, 150, 100,  50,
      00,  50, 100, 200, 200, 100,  50,  00
};

int kingsquares[64]=
{
    -10, -10, -50, -50, -50, -50, -10, -10,
     00,  00, -50, -50, -50, -50,  00,  00,
    -50, -50, -50, -50, -50, -50, -50, -50,
    -50, -50, -50, -50, -50, -50, -50, -50,
    -50, -50, -50, -50, -50, -50, -50, -50,
    -50, -50, -50, -50, -50, -50, -50, -50,
     00,  00, -50, -50, -50, -50,  00,  00,
    -10, -10, -50, -50, -50, -50, -10, -10
};

