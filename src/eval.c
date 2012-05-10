#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bce.h"

int *weights;
int *base_weights;
int weight_mode=1;

int default_weights[]= {
    /* piece values */
    0, 1000, 3250, 3500, 5000, 9000, 0,
    /* misc values */
    0, 0, 330, 670, 50, 30, 150, 100, 100, 200, 20, 80, 150, 100, LOSE/4, 200
};

void setup_default_weights()
{
    int i, j;
    
    base_weights = malloc(sizeof(*base_weights)*LAST_WEIGHT);
    weights = base_weights;
    memset(base_weights, 0, sizeof(*base_weights)*LAST_WEIGHT);

    memcpy(base_weights, default_weights, sizeof(default_weights));

    for(i=0;i<8;i++)
	for(j=0;j<8;j++)
	{
	    int x = SQ(i,j);
	    int y = SQ(j,i);
	    
	    base_weights[WHITE_PAWN_POS+x] = whitepawnsquares[y];
	    base_weights[BLACK_PAWN_POS+x] = blackpawnsquares[y];
	    base_weights[KNIGHT_POS+x] = knightsquares[y];
	    base_weights[BISHOP_POS+x] = bishopsquares[y];
	    base_weights[ROOK_POS+x] = rooksquares[y];
	    base_weights[QUEEN_POS+x] = queensquares[y];
	    base_weights[KING_POS+x] = kingsquares[y];
	}

    memcpy(base_weights+END_GAME_WEIGHTS, base_weights,
	   sizeof(*base_weights)*(LOCAL_LAST_WEIGHT));

    for (i=0;i<LOCAL_LAST_WEIGHT;i++)
    {
	if (base_weights[i] != base_weights[i+END_GAME_WEIGHTS])
	    printf("[%3i] %5d != %5d\n",i, base_weights[i],
		   base_weights[i+END_GAME_WEIGHTS]);
    }
}

/*
  return 1 if weights sucessfully read from file
  return 0 on any failure
*/
int load_weights(char *fn)
{
    FILE *in = fopen(fn, "r");

    if (in)
    {
	int i;

	weights = base_weights = malloc(sizeof(*weights)*LAST_WEIGHT);
	if (base_weights==NULL) return 0;
	
	for (i=0;i<LAST_WEIGHT;i++)
	{
	    int value;

	    if (feof(in))
	    {
		fprintf(stderr,
			"load_weights: premature end of file %s\n", fn);
		return 0;
	    }
	    
	    if (fscanf(in, "%d", &value)!=1) {
		fprintf(stderr,"load_weights: error reading from %s: %s\n",
		       fn, strerror(errno));
		free(base_weights);
		fclose(in);
		return 0;
	    }
	    else base_weights[i] = value;
	}
	fclose(in);
	return 1;
    }
    else
    {
	fprintf(stderr,"load_weights: error opening %s: %s\n", fn,
		strerror(errno));
	return 0;
    }
}

void save_weights(char *fn)
{
    FILE *out = fopen(fn, "w+");
    if (out) {
	int i;
	for (i=0;i<LAST_WEIGHT;i++) {
	    fprintf(out, "%d\n", base_weights[i]);
	}
	fclose(out);
    }
    else
	fprintf(stderr,"save_weights: error creating %s: %s\n",
		fn, strerror(errno));
}

int squarevalue__(int i, chesspiece p)
{
    switch(p)
    {
	case WPAWN:
	    return weights[WHITE_PAWN_POS+i];
	case BPAWN:
	    return weights[BLACK_PAWN_POS+i];
	case WKNIGHT:
	case BKNIGHT:
	    return weights[KNIGHT_POS+i];
	case WBISHOP:
	case BBISHOP:
	    return weights[BISHOP_POS+i];
	case WROOK:
	case BROOK:
	    return weights[ROOK_POS+i];
	case WQUEEN:
	case BQUEEN:
	    return weights[QUEEN_POS+i];
	case WKING:
	case BKING:
	    return weights[KING_POS+i];
	default:
	    return 0;
    }
}
    
int squarevalue(int f, int r, chesspiece p)
{
    return squarevalue__(SQ(f,r),p);
}




int material_advantage()
{
#ifdef FANCY    
    int d,t,np;
    
    d = board->material[WHITE] - board->material[BLACK];
    t = board->material[WHITE] + board->material[BLACK];
    np = board->pieces[(d>0)?WPAWN:BPAWN];
    
    return d + ((d * np) * (8000 - t)) / (64000 * (np + 1));
#else
    return board->material[WHITE] - board->material[BLACK];
#endif    
}

int mobility_advantage()
{
#ifdef NOT
    int mobility[2];
    move_and_score *restore_sp;

    restore_sp = move_sp;
    genmoves(WHITE);
    mobility[WHITE] = move_sp-restore_sp;
    move_sp = restore_sp;
    genmoves(BLACK);
    mobility[BLACK] = move_sp-restore_sp;
    move_sp = restore_sp;
    
    return (mobility[WHITE] - mobility[BLACK]) * MOBILITYFACTOR;
#else
    return 0;
#endif    
}


int mate_material(int c)
{
    if (board->material[c]<weights[ROOK_VALUE])
    {
	if (board->pieces[PIECE(c,pawn)] == 0)
	    return 0;
    }
    return 1;
}

#define verbose_print(file,rank,bonus,expl) /* do nothing*/ 
#define verbose_printf(foo) /* do nothing*/

int rooks[2];

int score_queen(int f, int r, int c)
{
    int o = opp(c);
    int df = abs(F(board->kings[o])-f);
    int dr = abs(R(board->kings[o])-r);
    int bonus=(7-min(df,dr))*weights[QUEEN_TROPISM];

    verbose_print(f,r,bonus,"queen/king tropism");
    return bonus;
}

int score_rook(int f, int r, int c)
{
    int o = opp(c);
    int df = abs(F(board->kings[o])-f);
    int dr = abs(R(board->kings[o])-r);
    int bonus=(7-min(df,dr))*weights[ROOK_TROPISM];
    
    verbose_print(f,r,bonus,"rook/king tropism");

    if (c == WHITE)
    {
	if (r == 6)
	{
	    bonus += weights[SEVENTH_RANK_ROOK];
	    verbose_print(f,r,weights[SEVENTH_RANK_ROOK],"seventh rank rook");
	}
    }
    else if (r == 1)
    {
	bonus += weights[SEVENTH_RANK_ROOK];
	verbose_print(f,r,weights[SEVENTH_RANK_ROOK],"seventh rank rook");
    }
    
    if (rooks[c] == -1)
    {
	rooks[c] = SQ(f,r);
    }
    else
    {
	if ((f==F(rooks[c]))||(r==R(rooks[c])))
	{
	    bonus += weights[DOUBLEDROOKS];
	    verbose_print(f,r,weights[DOUBLEDROOKS], "doubled rooks");
	}
    }
    
    if (!board->pawns[c][f+1])
    {
	bonus += weights[SEMIOPEN];
	if (!board->pawns[o][f+1])
	{
	    bonus += weights[OPENFILE];
	    verbose_print(f,r,weights[OPENFILE]+weights[SEMIOPEN],
			  "rook on open file");
	}
	else verbose_print(f,r,weights[SEMIOPEN], "rook on semi-open file");
    }
    
    return bonus;
}

int score_knight(int f, int r,int c)
{
    int o = opp(c);
    int df = abs(F(board->kings[o])-f);
    int dr = abs(R(board->kings[o])-r);
    int bonus=(14-(dr+df))*weights[KNIGHT_TROPISM];

    verbose_print(f,r,bonus,"knight/king tropism");
    return bonus;
}

/*  int score_bishop(int c) */
/*  { */
/*      return 0; */
/*  } */

int score_pawn(int f, int r, int c)
{
    int bonus = 0;

#define PASSED_PAWN_BONUS
#ifdef PASSED_PAWN_BONUS
    if (c==WHITE)
    {
	int x;
	
	for(x=r+1;x<8;x++)
	{
	    if (getpiece(f,x)==BPAWN)
		goto ppdone;
	    if (getpiece(f-1,x)==BPAWN)
		goto ppdone;
	    if (getpiece(f+1,x)==BPAWN)
		goto ppdone;
	}
	bonus += weights[PASSED_PAWN+r];
    }
    else
    {
	int x;
	
	for(x=r-1;x>0;x--)
	{
	    if (getpiece(f,x)==WPAWN)
		goto ppdone;
	    if (getpiece(f-1,x)==WPAWN)
		goto ppdone;
	    if (getpiece(f+1,x)==WPAWN)
		goto ppdone;
	}
	bonus += weights[PASSED_PAWN+7-r];
    }
    ppdone:
    
#endif    
    
    if (board->pawns[c][f+1]>1)
    {
	bonus -= weights[DOUBLED];
	verbose_print(f,r,-weights[DOUBLED],"doubled pawns");
    }
    if (!board->pawns[c][f]&&!board->pawns[c][f+2])
    {
	bonus -= weights[ISOLATED];
	verbose_print(f,r,-weights[ISOLATED],"isolated pawn");
    }
    else
    {
	int i;
	if (c == WHITE)
	{
	    for (i=r;i>0;i--)
	    {
		if ((f>0)&&(getpiece(f-1,i)==WPAWN))
		    goto Escape;
		if ((f<7)&&(getpiece(f+1,i)==WPAWN))
		    goto Escape;
	    }
	    bonus -= weights[BACKEDUP];
	    verbose_print(f,r,-weights[BACKEDUP],"backedup pawn");
	}
	else
	{
	    for (i=r;i<8;i++)
	    {
		if ((f>0)&&(getpiece(f-1,i)==BPAWN))
		    goto Escape;
		if ((f<7)&&(getpiece(f+1,i)==BPAWN))
		    goto Escape;
	    }
	    bonus -= weights[BACKEDUP];
	    verbose_print(f,r,-weights[BACKEDUP],"backedup pawn");
	}
    }
    
    Escape:
    return bonus;
}


int evaluate() {
#include "evaluator.inc.c"    
}

#undef verbose_print
#undef verbose_printf
#define verbose_print(file,rank,bonus,expl) printf("%c%c %+7d [%s]\n", FL(file), RK(rank),bonus,expl)
#define verbose_printf(foo) printf foo
#define VERBOSE

int evaluate_verbose(){
#include "evaluator.inc.c"
}
