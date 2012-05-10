#include <stdlib.h>
#include <string.h>

#include "bce.h"

int tablehits[3];

move_and_score killers[PV_LENGTH][2];

void compute_randoms()
{
    int i,j;

    for (i=0;i<=BKING;i++)
	for (j=0;j<64;j++)
	{
#ifdef HASH_64
	    randoms[i][j] = ((hashtype) getrandomnumber() << 32) | getrandomnumber();
#else 	    
	    randoms[i][j] = getrandomnumber();
#endif
	}
    
}

void compute_hash()
{
  int x;
  board->hash = 0;
  for (x=0;x<64;x++)
  {
      chesspiece p;
      if ((p=getpiece__(x)))
      {
	  board->hash ^= randoms[p][x];
      }
  }  
  board->hash ^= tomove();
}

void clear_hash()
{
    memset(tables, 0, sizeof(tables));
}


void store_hash(int depth, int flag, int score, move m)
{
    ttable *tt = tables + hash_index(board->hash);

    if (depth < tt->depth)
	return;

    tt->hash = board->hash;
    tt->depth = depth;
    tt->score = score;
    tt->flag = flag;
    tt->m    = m;
}

int retrieve_hash(int depth, int *score, move *m)
{
    ttable *tt = tables + hash_index(board->hash);

    if (tt->hash == board->hash)
    {
	if (tt->score<LOSE+100)
	{
	    *score = tt->score + (ply - search_info.startply) + tt->depth;
	}
	else if (tt->score>WIN-100)
	{
	    *score = tt->score - (ply - search_info.startply) - tt->depth;
	}
	else
	{
	    *score = tt->score;
	}
	
	*m = tt->m;
	if (tt->depth > depth)
	{
	    tablehits[0] ++;
	    return tt->flag;
	}
	else if (tt->depth == depth)
	{
	    tablehits[1] ++;
	    return tt->flag;
	}
	else
	{
	    tablehits[2] ++;
	    return LOWER_DEPTH;
	}
    }
    return NO_HIT;
}

void store_killer(move m, int p, int a, int b)
{
    if (a>killers[p][0].score)
    {
	killers[p][1].m = killers[p][0].score;
	killers[p][1].score = killers[p][0].score;
	killers[p][0].m = m;
	killers[p][0].score = a;
    }
    else if (a>killers[p][1].score)
    {
	killers[p][1].m = m;
	killers[p][1].score = a;
    }
}
