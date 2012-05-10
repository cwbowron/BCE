#include "bce.h"

int alphabeta_search(int a, int b, int d)
{
    int searched = 0;
    int incheck;
    int p;
    int c = TOMOVE;
    move_and_score *restore_sp;

    p = ply-search_info.startply;
    search_info.pv.length[p]=p;

    if ((p!=0)&&(position_seen_before()))
	return 0;
    
    if (d<=0)
	return qsearch(a,b,0);

    poll_check();
    search_info.branches ++;
    
    incheck = incheckp(c);
/*      if (incheck) d++; */
    
    restore_sp = move_sp;

    genmoves(c);
/*      sortmoves(restore_sp,move_sp); */
    
    while ((move_sp>restore_sp))
    {
	move m;
	int intocheck;

	m = popmove();
	domove(m);
      
	intocheck = (incheck) ? fullincheckp(c) : intocheckp(m,c);
      
	if (!intocheck)
	{
	    int value;
	    
	    searched++;
	    value = -alphabeta_search(-b, -a, d-1);

	    if (value>=b)
	    {
/*  		history[m]+=cutoff_inc(d); */
/*  		store_killer(m,p,value,b); */
		move_sp=restore_sp;
		undomove();
		return b;
	    }

	    if (value>a)
	    {
/*  		history[m] += stored_inc(d); */
		a = value;
		storepv(m,p);
		search_info.move_depth[p] = searched;
	    }
	}
	undomove();
    }
  
    move_sp = restore_sp;
  
    if (searched)
	return a;		/* at least one valid move */
    else if (incheck)
	return LOSE+p;		/* no valid moves and in check */
    else
	return STALEMATE;	/* no valid moves and not incheck */
}

int alphabeta_tables(int a, int b, int d)
{
    int searched = 0;
    int incheck;
    int p;
    int c = TOMOVE;
    move_and_score *restore_sp;
    int retrieve_type;
    int oldalpha = a;
    move bestmove;
    int hashscore;
    
    p = ply-search_info.startply;
    search_info.pv.length[p]=p;

    if (p)
    {
	if (position_seen_before())
	    return 0;

	switch (retrieve_type = retrieve_hash(d, &hashscore, &bestmove))
	{
	    case EXACT:
		return hashscore;
	    case L_BOUND:
		if (hashscore>=b)
		    return hashscore;
		break;
	    case U_BOUND:
		if (hashscore<=a)
		    return hashscore;
		break;
	    case LOWER_DEPTH:
		break;
	    case NO_HIT:
		break;
	    default:
		fprintf(stderr,"unknown transposition table enumeration: %d\n",
			retrieve_type);
		exit(EXIT_FAILURE);
	}
	if (d<=0)
	    return qsearch(a,b,0);
    }

    poll_check();

    incheck = incheckp(c);
/*      if (incheck) d++; */

    restore_sp = move_sp;

    genmoves(c);
    sortmoves(restore_sp,move_sp, 0);
    
    while ((move_sp>restore_sp))
    {
	move m;
	int intocheck;

	m = popmove();
	domove(m);
      
	intocheck = (incheck) ? fullincheckp(c) : intocheckp(m,c);
      
	if (!intocheck)
	{
	    int value;
	    
	    searched++;
	    value = -alphabeta_tables(-b, -a, d-1);

	    if (value>a)
	    {
		history[m] += d;
		a = value;
		storepv(m,p);
		bestmove = m;
	    }

	    if (a>=b)
	    {
		move_sp=restore_sp;
		undomove();
		store_hash(d, L_BOUND, a, m);
		return a;
	    }
	}
	undomove();
    }
  
    move_sp = restore_sp;
  
    if (searched)
    {
	if (a>oldalpha)
	    store_hash(d, EXACT, a, bestmove);
	else
	    store_hash(d, U_BOUND, a, bestmove);
	return a;		/* we found at least one valid move */
    }
    else if (incheck)
	return LOSE+p;		/* we found no valid moves and we are in check */
    else
	return STALEMATE;	/* we found no valid moves and are not incheck */
}


int negamax_search(int a, int b, int d)
{
    int searched = 0;
    int incheck;
    int p;
    int c = TOMOVE;
    
    move_and_score *restore_sp;

    p = ply-search_info.startply;
    search_info.pv.length[p]=p;
    
    if (p&&position_seen_before())
	return 0;
    
    if (d<=0)
	return qsearch(a,b,0);

    poll_check();
    
    
    incheck = incheckp(c);
/*      if (incheck) d++; */
    
    restore_sp = move_sp;

    genmoves(c);
    
    while ((move_sp>restore_sp))
    {
	move m;
	int intocheck;

	m = popmove();
	domove(m);
      
	intocheck = (incheck) ? fullincheckp(c) : intocheckp(m,c);
      
	if (!intocheck)
	{
	    int value;
	    
	    searched++;
	    value = -negamax_search(-b, -a, d-1);

	    if (value>a)
	    {
		a = value;
		storepv(m,p);		
	    }
	}
	undomove();
    }
  
    move_sp = restore_sp;
  
    if (searched)
	return a;		/* we found at least one valid move */
    else if (incheck)
	return LOSE+p;		/* we found no valid moves and we are in check */
    else
	return STALEMATE;	/* we found no valid moves and are not incheck */
}
