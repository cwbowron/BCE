#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "bce.h"

#define MAX_PLIES 512

#define TD_ALPHA (0.7)
#define TD_LAMBDA (0.8)

#define EPSILON (100)

#define EVAL_INTERVAL (50)

#define EVAL_SCALE (atanh(0.35)/(double)STATIC_PAWN_VALUE)

/*  #define LEARNING */
#define USE_POW

struct 
{
    chessboard *boards[MAX_PLIES];
    int search_results[MAX_PLIES];
    int actual[MAX_PLIES];
    int usable[MAX_PLIES];
    int index;
} stored;


double gradient(int, chessboard *, int);
double **gradientvector(int, int);
void release_gradients(double **g, int count, int n);

int eval_for_white()
{
    return (tomove() == WHITE) ? evaluate() : -evaluate();
}

void init_learning()
{
    stored.index = 0;
    memset(stored.usable, 0, sizeof(stored.usable));
    memset(stored.boards, 0, sizeof(stored.boards));    
}

void alloc_board(int p)
{
    stored.index ++;
    stored.boards[stored.index] = malloc(sizeof(chessboard));
    memset(stored.boards[stored.index], 0, sizeof(chessboard));
}

void free_learning()
{
    int x;

    for (x=0;x<MAX_PLIES;x++)
	if (stored.boards[x])
	    free(stored.boards[x]);
}

void store_board(int p)
{
#ifdef LEARNING    
    memcpy(stored.boards[stored.index], board, sizeof(chessboard));
#endif    
}

void save_leaf(move *pv, int n, int value, int hashed)
{
#ifndef LEARNING
    return;
#else
    int s_ply = ply;
    int searched_value = -1;
    int stored_board_value = -1;

    
    if (tomove() != WHITE)
	value = -value;

    stored.search_results[stored.index] = value;

    /* first check our stored position */
    if (stored.boards[stored.index]->piececount[0]>0)
    {
	chessboard *temp_board = board;

	board = stored.boards[stored.index];
	countmaterial();
	
	stored_board_value = eval_for_white();

	board = temp_board;
	countmaterial();

	if (((stored_board_value > (value - EVAL_INTERVAL)) &&
	    (stored_board_value < (value + EVAL_INTERVAL))) ||
	    (value > WIN - 50) ||
	    (value < LOSE + 50))
	{
	    stored.usable[stored.index] = 1;
	    stored.actual[stored.index] = stored_board_value;
	    printf("STORING: qsearch board = %d\n", stored_board_value);
	}
    }

    /* then check to see if our value is the same as our last search */
    if (!stored.usable[stored.index])
    {
	if ((stored.search_results[stored.index] ==
	     stored.search_results[stored.index-1]))
	{
	    memcpy(stored.boards[stored.index], stored.boards[stored.index-1],
		   sizeof(chessboard));
	    stored.usable[stored.index] = 1;
	    stored.actual[stored.index] = stored.actual[stored.index]-1;
	    printf("STORING: previous board = %d\n",
		   stored.search_results[stored.index-1]);
	}
    }

    /* lastly check to see if our pv leads to a correct board eval */
    if (!stored.usable[stored.index])
    {
	int i;
	
	printf("pv: ");
	for (i=0;i<n;i++)
	{
	    if (pv[i] == dummymove)
		break;

	    domove(pv[i]);
	    printf("%s ", movestring(pv[i]));
	}
	printf("\n");
    
	searched_value = eval_for_white();
	
	if ((searched_value > value - EVAL_INTERVAL)&&
	    (searched_value < value + EVAL_INTERVAL))
	{
	    memcpy(stored.boards[stored.index], board, sizeof(chessboard));
	    stored.usable[stored.index] = 1;
	    stored.actual[stored.index] = searched_value;
	    printf("STORING: pv board = %d\n", searched_value);
	}
	else
	{
	    printf("!usable: stored_position %d, "
		   "searched %d, previous %d, stored %d\n",
		   stored_board_value, searched_value,
		   stored.search_results[stored.index-1], value);
	}
	
	while (ply>s_ply)
	    undomove();
    }
    
#endif
    
}

void store_weight_history()
{
    FILE *tmp = fopen("weighttrace", "a+");
    fprintf(tmp, "%d\n", base_weights[2]);
    fclose(tmp);
}


/*
  these should be inside update_weights, but due to compiler bug
  they are not
*/
double tanhvector[MAX_PLIES];
double d[MAX_PLIES];	/* temporal difference array */
double delta[MAX_PLIES];	/* change in weights */


void update_weights(int winner)
{
#ifdef LEARNING    
    int unusable = 0;
    
#ifndef USE_POW
    double lambdas[MAX_PLIES];
    double temp_lambda;
#endif    

    double max, min;
    int weight_max = 0, weight_min = 0;
    
    double **gradients;

    int w, p, n = stored.index;

    if (n < 2) return;
    
    tellics("set open 0\n");

    memset(d,0,sizeof(d));

    /* pre calculate lambda powers */
#ifndef USE_POW    
    temp_lambda = 1.0;
    for (p=0;p<MAX_PLIES;p++)
    {
	lambdas[p] = temp_lambda;
	temp_lambda *= TD_LAMBDA;
    }
#endif
    
#define SMOOTH_EVAL
#ifdef SMOOTH_EVAL

    tanhvector[1] = tanh(EVAL_SCALE*stored.search_results[1]);

    for (p=2;p<n;p++)
    {
	double tmp;
	double normalized = stored.search_results[p];
	tanhvector[p] = tanh(EVAL_SCALE*normalized);

	if (stored.search_results[p] > WIN - 50)
	    tmp = 1.0;
	else if (stored.search_results[p] < LOSE + 50)
	    tmp = -1.0;
	else 
	    tmp = tanhvector[p];
	
	d[p-1] = (tmp-tanhvector[p-1]);
    }
    
#else

    for (p=2;p<n-1;p++)
    {
	if (stored.search_results[p] > WIN - 50)
	    d[p-1] = stored.search_results[p] - stored.search_results[p-1];
	
	else if (stored.search_results[p] < LOSE + 50)
	    d[p-1] = stored.search_results[p] - stored.search_results[p-1];
	
	else 
	    d[p-1] = stored.search_results[p] - stored.search_results[p-1];
    }
    
#endif    
    
    max = 0.0;
    min = 100000.0;
    
    gradients = gradientvector(LAST_WEIGHT, n);

    printf("Adjusting weights\n");
    printf("Number of weights: %d\n", LAST_WEIGHT);
    printf("Number of plies: %d\n", n);
    
    for (w = PAWN_VALUE; w < LAST_WEIGHT; w ++)
    {
	if (!xboard_mode && !robo_mode)
	    if (w % 20 == 0)
		printf(".");

	delta[w] = 0.0;
	
	for (p=1;p<n;p++)
	{
	    int j;
	    double S2 = 0.0;

	    unusable = 0;
	    
	    printf("p: %d\t", p);
	    
	    for (j=1;j<p-1;j++)
	    {
		if (stored.usable[j])
		{
		    double grad =
			/* = sech^2 * EVAL_SCALE * grad (chain rule) */
			(1.0 - tanhvector[j] * tanhvector[j]) *
			EVAL_SCALE *
			gradients[w][j];

#ifdef USE_POW		    
		    S2 += pow(TD_LAMBDA, p-j) * grad;
#else
		    S2 += lambdas[p-j] * grad;
#endif
		}
		else
		{
		    unusable ++;
		}
	    }
	    delta[w] += d[p] * S2 / EVAL_SCALE;
	}

	/* update weight accordingly */
	base_weights[w] += TD_ALPHA * delta[w];

	if (delta[w] > max)
	{
	    max = delta[w];
	    weight_max=base_weights[w];
	}
	else if (delta[w] < min)
	{
	    min = delta[w];
	    weight_min=base_weights[w];
	}
	
    }
    
    if (!xboard_mode && !robo_mode)
	printf("\n");

    printf("weights adjusted\n");
    
    countmaterial();
    
    if (!xboard_mode && !robo_mode)
    {
	printf("unusable states = %d\n", unusable);
	printf("min = %f (%d), max = %f (%d)\n", min, weight_min,
	       max, weight_max);
    }

    release_gradients(gradients, LAST_WEIGHT, n);

    store_weight_history();
#endif


    
    free_learning();

    tellics("set open 1\n");
}

void release_gradients(double **g, int count, int n)
{
    int i;

    for (i = 0; i<count;i++)
	free(g[i]);
    free(g);
}

/*
  return an array of the gradients
  g[WEIGHT][TIME]
*/ 
  
double **gradientvector(int count, int n)
{
    int t;
    int x;
    double **grad;

    grad = malloc(sizeof(*grad)*count);
    
    printf("Calculating gradients [%d x %d]\n",
	   count, n);
    
    for (x=1;x<count;x++)
    {
	if (!xboard_mode && !robo_mode)
	{
	    if (x % 20 == 0)
		printf(".");
	}
	
	grad[x] = malloc(sizeof(*grad[x]) * n);

	for (t = 0; t < n; t++)
	{
	    if (stored.usable[t])
	    {
		grad[x][t] = gradient(x, stored.boards[t], t);
	    }
	    else
	    {
		grad[x][t] = 0.0;
	    }
	}
    }
    
    if (!xboard_mode && !robo_mode)
	printf("\n");
    
    return grad;
}

/*
  returns the gradient with respect to weight w in board *b
  using the formulat (f(x+h)-f(x)) / h
*/
double gradient(int w, chessboard *b, int index)
{
    int v1, v2;
    chessboard *temp_board = board;

    board = b;

    if (gamemode != actualmode())
	change_gamemode(actualmode(), 0);
    
    countmaterial();
    v1 = eval_for_white();
    
    base_weights[w] += EPSILON;
    countmaterial();
    v2 = eval_for_white();

    base_weights[w] -= EPSILON;	/* restore weight */
    board = temp_board;		/* restore board */
    
    return (double)(v2-v1)/(double)EPSILON;
}
