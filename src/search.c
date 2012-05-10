/*
  This File is part of Boo's Chess Engine
  Copyright 2001 by Christopher Bowron
*/  
#include <stdlib.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>

#include "bce.h"

/*  #define NULL_MOVE */

#define POLL_FREQ	((1<<13)-1)
#define NO_TIMEOUT	INF

#define ASPIRATION_WINDOW          2000

#define poll_check() if (((++search_info.nodes)&(POLL_FREQ))==0) poll()

#define stored_inc(d) (d*4)
#define cutoff_inc(d) (d)

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
    
int gamemode;
int pondering = 0;

int history[64*64];		/* history heuristic for move ordering */

struct
{
    move m;
    int depth;
} counter[64*64];

search_info_type search_info;
branch_info_type branch_info;

int (*search)(int,int,int,int) = negascout_tables;

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

long get_ms()
{
    struct timeb timebuffer;
    ftime(&timebuffer);
    return (timebuffer.time * 1000) + timebuffer.millitm;
}

int actualmode()
{
    int i, pieces = 0;

    for (i=0;i<=BKING;i++) pieces += board->pieces[i];
    
    if (pieces<=10)
	return ENDGAME;
    else
	return MIDGAME;
    
}

void change_gamemode(int mode, int clear)
{
    switch(mode)
    {
	case OPENING:
	    gamemode = OPENING;
	    weights = base_weights;
	    break;
	case MIDGAME:
	    gamemode = MIDGAME;
	    weights = base_weights;
	    break;
	case ENDGAME:
	    gamemode = ENDGAME;
	    weights = base_weights + END_GAME_WEIGHTS;
	    break;
    }
    if (clear)
    {
	countmaterial();
	clear_hash();
    }
}

/*
  return a quick evaluation of a move for use in move ordering
*/
int quickeval(move m, int p, move hashed)
{
    int result;
    int piece;

    piece = getpiece__(TO(m));
    if (piece)
    {
	int n = weights[chesspiecevalue(piece)];
	int d = weights[chesspiecevalue(getpiece__(FR(m)))];

	if (d==0) d = 100000;	/* king  */
	
	result = 10000 + 3000 * n / d;
    }
    else
	result = 0;

    if (m == counter[lastmove()].m)
	result += 20000;
    else if (m == hashed)
	result += 15000;
    else if (m == killers[p][0].m)
	result += 10000;
    else if (m == killers[p][1].m)
	result += 5000;
    else
	result += history[m];

    return result;
}

/*
  compare two moves for initial move ordering
*/
static int movecmp(const void *foo, const void *bar)
{
    return ((move_and_score*)foo)->score-((move_and_score*)bar)->score;
}

/*
** The h-constants for this version of Shell-sort come from:
** Handbook of Algorithms and Data Structures in Pascal and C
** By Gaston Gonnet, Ricardo Baeza-Yates
** Addison Wesley; ISBN: 0201416077
** These h-increments work better for me than Sedgewick's under
** full optimization.  Your mileage may vary, so I suggest you
** try Sedgewick's suggestions as well.  The h-increment arena
** is a rich environment for exploration.  I suggest attempting
** all possible permutations below 20, since that is where a
** good shell-sort is crucuial.  If you find something wonderul
** you may get your name up in lights.
** -- D. Corbit
*/
int ShellSort(move_and_score start[], move_and_score end[])
{
    int i, j, h;
    size_t n = (end - start);
    move_and_score  key;

    for (h = n; h > 0;)
    {
        for (i = h; i < n; i++)
	{
            j = i;
            key = start[i];
            while ((j >= h) & (key.score < start[j - h].score))
	    {
                start[j] = start[j - h];
                j -= h;
            }
            start[j] = key;
        }
        h = (size_t) ((h > 1) & (h < 5)) ? 1 : 5 * h / 11;
    }

    return 0;
}

void sortmoves(move_and_score *start, move_and_score *end, move hashed)
{
    move_and_score *t;
    
    int p = ply - search_info.startply;

    for (t=start;t<end;t++)
	t->score = quickeval(t->m, p, hashed);
    
/*      qsort(start,end-start,sizeof(*end),movecmp); */
    ShellSort(start, end);
}


void clearthoughts()
{
    memset(history, 0, sizeof(history));
    memset(search_info.pv.moves, 0, sizeof(search_info.pv.moves));
    memset(search_info.pv.length, 0, sizeof(search_info.pv.length));
    memset(killers,0,sizeof(killers));

    memset(counter,0,sizeof(counter));
}

void signal_handler(int sig)
{
    search_info.stop = sig;
    signal(SIGINT, signal_handler);
}


char thoughts[512];

char *formatthoughts(int x, int best, long duration, move *moves)
{
    char *b;
    int j;

    b = thoughts +
	sprintf(thoughts,"%2d %8d %4ld %7d", x, best/10, duration,
		search_info.nodes);

    for (j=0;moves[j];j++)
    {
	b += sprintf(b," %s", movestring(moves[j]));
    }
    b+=sprintf(b,"\n");
    return thoughts;
}

/*
  c is the player who is pondering what they are going to do when
  there opponent makes a move.

  this will set up some values in history that are hopefully cutoffs
  for the real search and put stuff in the trasposition tables
*/
void ponder(int c)
{
    int saveddepth;
    int savedtime;
    int j;
    void (*savedhandler)(int);
    
    clearthoughts();
    
    if (gamemode == OPENING) return;
    if (ply<=0) return;
    
    savedhandler = signal(SIGINT, signal_handler);
    search_info.stop = 0;
    
    savedtime = searchtime;
    saveddepth = searchdepth;

    searchdepth = 100;
    searchtime = NO_TIMEOUT;
    pondering = 1;
    
    think();

    pondering = 0;
    signal(SIGINT, savedhandler);
    searchtime = savedtime;
    searchdepth = saveddepth;
    
    for (j=1;j<PV_LENGTH;j++)
    {
	search_info.pv.moves[j-1] = search_info.pv.moves[j];
	killers[j-1][0].m = killers[j][0].m;
	killers[j-1][1].m = killers[j][1].m;
	killers[j-1][0].score = killers[j][0].score;
	killers[j-1][1].score = killers[j][1].score;
    }
}

move bookmove()
{
    if (gamemode == OPENING)
    {
	move m = bookopening();
	if (validmove(m)==1)
	{
	    if (thinking_mode||robo_mode)
		output("0 0 0 0 Book Opening\n");
	    return m;
	}
	else 
	{
	    change_gamemode(MIDGAME, 1);
	}
    }
    return dummymove;		/* not in book mode or invalid book move */
}


move think()
{
    long starttime;
    int duration;
    int sig;
    int c = tomove();
    move best_line[PV_LENGTH];
    
    static int best;
    static int best_depth;
    static int x;
    static int depth_reached;
    static move best_move;
    static int branches;
    double bf;
    move_and_score *restore_sp;

    alloc_board(ply);		/* allocate storage for saved leaf */
    
#ifdef ASPIRATION_WINDOW    
    best = evaluate();
#endif
    
    if (actualmode()!=gamemode)
	change_gamemode(actualmode(), 1);

    if (!ponder_mode)
	clearthoughts();
    
    starttime = get_ms();
    search_info.startply = ply;
    restore_sp = move_sp;

    if((sig = setjmp(search_info.restore_stack)))
    {
	while (ply>search_info.startply)
	    undomove();
	
	move_sp = restore_sp;
	
	switch (sig)
	{
	    case SIGALRM:
		printf("time expired\n");
		break;
	    case SIGINT:
		printf("pondering interrupted\n");
		break;
	    default:
		printf("unknown signal\n");
		break;
	}
    }
    else
    {
	long timeout = 0;
	
	/*
	  search for time/30
	*/
	if (!pondering)
	{
	    int clock_time = chessclock[c];

	    if (clock_time<0)
		clock_time = 500;

	    timeout = max(1,clock_time / 30);
	    
	    search_info.stoptime = starttime + timeout;
	}

	search_info.nodes = 0;
	search_info.branches = 0;
	tablehits[0] = tablehits[1] = tablehits[2] = 0;
	search_info.pv.hits = 0;
	memset(search_info.move_depth,0,sizeof(search_info.move_depth));
	
	search_info.polling = 0;

	for(x=1;;x++)
	{
#ifdef ASPIRATION_WINDOW	    
	    int alpha = best - ASPIRATION_WINDOW;
	    int beta = best + ASPIRATION_WINDOW;
	    best = search(alpha, beta, x, 1);
	    
	    if (best<=alpha)
	    {
		best = search (-INF, beta, x, 1);
	    }
	    else if (best>=beta)
	    {
		best = search (alpha, INF, x, 1);
	    }
	    
#else
	    best = search (-INF, INF, x, 1);
#endif
	    
	    best_move = search_info.pv.moves[0];
	    memcpy(best_line,search_info.pv.moves, sizeof(best_line));
	    
	    best_line[search_info.pv.length[0]] = 0;
	    best_depth = search_info.pv.length[0];
	    
	    depth_reached = x;
	    branches = search_info.branches;
	    
	    if (best>WIN-50)
		break;
	    if (x>=searchdepth)
		break;
	    if (thinking_mode) 
	    {
		long dur = (get_ms()-starttime)/1000;
		char * str = formatthoughts(x, best, dur, best_line);
		
		printf("%s", str);
	    }

	    /*
	      after our first level search, we should have at least one
	      valid move, so we can poll for time on subsequent searches
	    */
	    if (timeout>0)
	    {
		search_info.polling = 1;
	    }
	} 
    }

    /* if the search we were working on when we timed out is valid,
       use it.
    */
    if ((search_info.pv.length[0] > 0) &&
	(1==validmove(search_info.pv.moves[0])))
    {
	best_move = search_info.pv.moves[0];
	memcpy(best_line,search_info.pv.moves, sizeof(best_line));
    
	best_line[search_info.pv.length[0]] = 0;
	best_depth = search_info.pv.length[0];
    }
    
    depth_reached = x;

    bf = branch_factor(branches, depth_reached);

    if (bf>0)
    {
	branch_info.average = (branch_info.average*branch_info.count+bf)/
	    (branch_info.count+1);
	branch_info.count ++;
    }

    duration = (get_ms()-starttime);
    if (!duration) duration = 1;
    
    if ((best>WIN-50)||(best<LOSE+50))
	output("mate in %d\n", WIN-abs(best));

    output("%s", formatthoughts(depth_reached,best,duration/1000,best_line));
    output("n/s: %d, table: %d-%d-%d\n",
	   (int)(((double)search_info.nodes/(double)duration)*1000),
	   tablehits[0],tablehits[1],tablehits[2]);

    output("branch factor: %3.3f [%3.3f]\n", bf, branch_info.average);
    output("depths searched: %2d, %2d, %2d, %2d, %2d\n",
	   search_info.move_depth[0],search_info.move_depth[1],
	   search_info.move_depth[2],search_info.move_depth[3],
	   search_info.move_depth[4]);

    save_leaf(best_line, best_depth, best, 0);

    return best_move;
}

move bce()
{
    move temp;
    
    if ((temp=bookmove())!=dummymove)
	return temp;
    else
	return think();
}

/* check for time up or pondering interrupt... */
void poll()
{
    if (!xboard_mode)
    {
	if (bioskey())
	{
	    longjmp(search_info.restore_stack, SIGINT);
	}
    }
    else if (search_info.stop)
    {
	int r = search_info.stop;
	search_info.stop=0;
	longjmp(search_info.restore_stack,r);
    }
    else if (search_info.polling&&(get_ms()>search_info.stoptime))
    {
	longjmp(search_info.restore_stack, SIGALRM);
    }
}

void storepv(move m, int p)
{
    search_info.pv.moves[p] = m;
    search_info.pv.length[p] = search_info.pv.length[p+1];
}

void store_retrieved(move m, int p)
{
    search_info.pv.moves[p] = m;
    search_info.pv.moves[p+1] = dummymove;
    search_info.pv.length[p] = p+2;
    search_info.pv.length[p+1] = p+2;
}

int extend_search_p()
{
    if (board->flags&promo_flag)
	return 1;
    return 0;
}

void store_counter(move m, int depth)
{
    if (-depth<=counter[lastmove()].depth)
    {
	counter[lastmove()].m = m;
	counter[lastmove()].depth = depth;
    }
}

#ifdef NULL_MOVE

#define AVOID_NULL_PIECES (7)
#define AVOID_NULL_MAT (10*STATIC_PAWN_VALUE)

int NullOk(int depth, int side_to_move)
{
    int cwp, cbp,
	base_reduction = (depth > 3) ? 0 : 1;
    
    /* If there is a risk of Zugzwang then return base_reduction */
    if (side_to_move == WHITE && board->material[WHITE] < AVOID_NULL_MAT)
        return base_reduction;

    if (side_to_move == BLACK &&  board->material[BLACK] < AVOID_NULL_MAT)
        return base_reduction;

    cwp = board->piececount[WHITE];
    if (side_to_move == WHITE && cwp < AVOID_NULL_PIECES)
        return base_reduction;

    cbp = board->piececount[BLACK];
    if (side_to_move == BLACK && cbp < AVOID_NULL_PIECES)
        return base_reduction;

    return 2 + ((depth) > ((6) + (((cwp < 4 && cbp < 4) ? 2 : 0))) ? 1 : 0);
}
#endif

int qsearch(int a, int b, int d)
{
    move_and_score *restore_sp;
    int incheck = 0;
    int searched = 0;
    int b2;
    int p;
    int c = tomove();
    int current_eval;
    move bestmove = 0;
    int a_original;
    int init_a = a;
    int store_position = 0;
    
    p = ply-search_info.startply;
    search_info.pv.length[p]=p;

    if (position_seen_before())
	return 0;
    
    current_eval = evaluate();
     
    if ((current_eval>a)&&(current_eval<b))
	store_position = 1;
    
    if (p>=PV_LENGTH)
    {
	if (store_position)
	    store_board(search_info.startply);

	return current_eval;
    }
    else if  (current_eval>=b)
	return current_eval;
    else if (current_eval>a)
	a = current_eval;

    poll_check();
    
    a_original = a;
    b2 = b;

    incheck = incheckp(c);

    restore_sp = move_sp;
    
    genattacks();
    sortmoves(restore_sp, move_sp, 0);
    
    while (move_sp>restore_sp)
    {
	move m;
	int intocheck;
	  
	m = popmove();

	domove(m);

	intocheck = (incheck) ? fullincheckp(c) : intocheckp(m,c);
    
	if (!intocheck)
	{
	    int t = -qsearch(-b2, -a, d+1);
	    
	    searched++;
	    
	    /* re-search */
	    if ((t>a)&&(t<b))
	    {
		if ((searched>1))
		    a = -qsearch(-b,-t, d+1);

		storepv(m, p);
	    }
	    
	    a = max(a,t);
	    if (a >= b)
	    {
		move_sp = restore_sp;
		undomove();

		store_counter(m, p);
		return a;
	    }
	    b2 = a + 1;
	}
	undomove();
    }
  
    move_sp=restore_sp;
  
    if (searched)
    {
	store_counter(bestmove, p);

	if ((a_original == a) && (a != init_a) && store_position)
	    store_board(search_info.startply);		

	return a;		/* had at least one valid move/attack */
    }
    else
    {
	if (store_position)
	    store_board(search_info.startply);

	return current_eval;	/* we found no valid attacks */
    }
}


int negascout_tables(int a, int beta, int d, int DoNull)
{
    int searched = 0;
    int incheck;
    move_and_score *restore_sp;
    int b;
    int p;
    int c = tomove();

#ifdef NULL_MOVE
    int nd;
#endif    
    
    int hashscore;
    move bestmove;
    int oldalpha = a;
    int retrieve_type;

    p = ply-search_info.startply;

    /* draw? */
    if (p&&position_seen_before())
	return 0;
    
    switch (retrieve_type = retrieve_hash(d, &hashscore, &bestmove))
    {
	case EXACT:
	    if ((hashscore>a)&&(hashscore<beta))
		if (validmove(bestmove) == 1)
		{
		    store_retrieved(bestmove, p);
		    return hashscore;
		}
	    break;
	case L_BOUND:
	    if (hashscore>=beta)
		return hashscore;
/*  	    if (hashscore>a) */
/*  		a = hashscore; */
	    break;
	case U_BOUND:
	    if (hashscore<=a)
		return hashscore;
/*  	    if (hashscore<beta) */
/*  		beta = hashscore; */
	    break;
	case LOWER_DEPTH:
	    break;
	case NO_HIT:
	    break;
	    default:
		fprintf(stderr,
			"unknown transposition table enumeration: %d\n",
			retrieve_type);
		exit(EXIT_FAILURE);
    }
    
    if (d<=0)
	return qsearch(a,beta,0);

    search_info.branches ++;
    poll_check();
    
    search_info.pv.length[p] = p;

    b = beta;
    
    incheck = incheckp(c);

#ifdef NULL_MOVE

    /* From Dann Corbit DCorbit@connx.com*/
    
    /* Perform a NULL move if; (0) We're not on the first 2 plies (1) We're
     * not in the process of performing a NULL move right now (Note - DoNull
     * stores whether or not we are OK to NULL move, not whether we are doing
     * one, hence testing for TRUE not FALSE) (2) We're not in check (3)
     * There are enough pieces left to avoid high risk of zugzwang 
     */
    if ((ply > 2) && DoNull &&
	!incheck && (nd = NullOk(d, c)) &&
	((d - nd) >= 1))
    {
	int t;

        /* Increase the NULL reduction by one ply to account for the fact
         * that we've passed on this move. */
        nd++;

/*  	printf("<"); */
	
	/* Reverse side to move... */
	fake_domove();

        /* Do the null move reduced-depth search */
        t = -negascout_tables(-b, -a, d - nd, 0);

        /* Put things back */
	fake_undomove();

/*  	printf(">"); */
	
        /* A fail high means that the positional score here is so high that
         * even if the opponent is given a 'free move' then he still can't
         * improve his position enough to increase the value of alpha. If
         * this is the case then clearly the current position is very strong.
         * In fact it is so strong that the opponent would never let it occur
         * in the first place, so we should cause a cut off.  */
        if (t >= b) {

            /* Maybe hash tables need to be updated with lower bound? */
            return t;
        }
    }
    
#endif

    
    restore_sp = move_sp;
  
    genmoves();
    sortmoves(restore_sp, move_sp, bestmove);
    
    while (move_sp>restore_sp)
    {
	move m;
	int intocheck;
	
	m = popmove();
	domove(m);
      
	intocheck = (incheck) ? fullincheckp(c) : intocheckp(m, c);
	
	if (!intocheck)
	{
	    int t;
	    searched++;
	    
	    t = -negascout_tables(-b, -a, d-1, 1);
	    
	    if ((t>a)&&(t<beta))
	    {
		/* re-search */
		if (searched>1)
		    a = -negascout_tables(-beta,-t, d-1, 1);
		history[m] += stored_inc(d);
		search_info.move_depth[p] = searched;
		storepv(m, p);
		bestmove = m;
	    }
	    
	    if (t>a) a=t;

	    if (a>=beta)
	    {
		undomove();
		move_sp = restore_sp;
		history[m] += cutoff_inc(d);
		store_killer(m,p,a,beta);
		store_hash(d, L_BOUND, a, m);

		store_counter(m, p);
		return a;
	    }
	    b = a + 1;
	}
	undomove();
    }
  
    move_sp = restore_sp;

    if (searched)
    {
	if (a>oldalpha)
	{
	    store_hash(d, EXACT, a, bestmove);
	    store_counter(bestmove, p);
	}
	else
	{
	    store_hash(d, U_BOUND, a, bestmove);
	}
	
	return a;		/* at least one valid move */
    }
    else if (incheck)
	return LOSE+p;		/* farther we are from losing the better */
    else
	return STALEMATE;	/* no valid moves and not incheck */
}

int negascout_search(int a, int beta, int d)
{
    int searched = 0;
    int incheck;
    move_and_score *restore_sp;
    int b;
    int p;

    int c = tomove();

    p = ply-search_info.startply;
    search_info.pv.length[p] = p;

    if (p&&position_seen_before())
	return 0;

    if (d<=0)
	return qsearch(a,beta,0);

    poll_check();
    search_info.branches++;
    
    b = beta;
    
    incheck = incheckp(c);
/*      if (incheck) d++; */

    restore_sp = move_sp;
  
    genmoves();
    sortmoves(restore_sp, move_sp, 0);
    
    while (move_sp>restore_sp)
    {
	move m;
	int intocheck;
	
	m = popmove();
	domove(m);
      
	intocheck = (incheck) ? fullincheckp(c) : intocheckp(m, c);
      
	if (!intocheck)
	{
	    int t;
	    
	    searched++;
	    
	    t = -negascout_search(-b, -a, d-1);

	    if ((t>a)&&(t<beta))
	    {
		if (searched>1)
		    a = -negascout_search(-beta,-t, d-1);
		history[m] += stored_inc(d);
		storepv(m, p);
		search_info.move_depth[p] = searched;
	    }

	    if (t>a) a = t;

	    if (a>=beta)
	    {
		history[m]+=cutoff_inc(d);
		undomove();
		move_sp = restore_sp;
		return a;
	    }
	    b = a + 1;
	}
	undomove();
    }
  
    move_sp = restore_sp;

    if (searched)
	return a;		/* at least one valid move */
    else if (incheck)
	return LOSE+p;		/* the farther we are from losing the better */
    else
	return STALEMATE;	/* no valid moves and not incheck */
}

/* not currently used */
int MTDF(int estimate, int d)
{
    int g = estimate;
    int upperbound = WIN;
    int lowerbound = LOSE;
    
    while (upperbound>lowerbound)
    {
	int b = (g==lowerbound) ? g + 1 : g;

	g = search(b - 1, b, d, 1);
	
	if (g<b)
	    upperbound = g;
	else
	    lowerbound = g;
    }
    return g;
}
