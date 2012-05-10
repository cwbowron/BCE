/*
  This File is part of Boo's Chess Engine
  Copyright 2000 by Christopher Bowron
*/  
#include <stdlib.h>
#include <stdio.h>
#include "bce.h"

int pawnattackedp(int f, int r, int c);
int knightattackedp(int f, int r, int c);
int slideattackedp(int f, int r, int df, int dr, int c, chesspiece p);

int kingattackedp(int f, int r, int c, int o)
{
    if ((abs(R(board->kings[o])-r)<2)&&
	(abs(F(board->kings[o])-f)<2))
    {
	return king;
    }
    return 0;
}

int fullincheckp(int c)
{
    int f,r,o;

    f = F(board->kings[c]);
    r = R(board->kings[c]);
    o = opp(c);
    
    if (pawnattackedp(f,r,c))
    {
	return pawn;
    }
    if (knightattackedp(f,r,c))
    {
	return knight;
    }
    if (slideattackedp(f, r, 1,  1, c, makepiece(o, bishop)))
    {
	return bishop;
    }
    if (slideattackedp(f, r, 1, -1, c, makepiece(o, bishop)))
    {
	return bishop;
    }
    if (slideattackedp(f, r,-1,  1, c, makepiece(o, bishop)))
    {
	return bishop;
    }
    if (slideattackedp(f, r,-1, -1, c, makepiece(o, bishop)))
    {
	return bishop;
    }
    if (slideattackedp(f, r, 1, 0, c, makepiece(o, rook)))
    {
	return rook;
    }
    if (slideattackedp(f, r, 0, 1, c, makepiece(o, rook)))
    {
	return rook;
    }
    if (slideattackedp(f, r, -1, 0, c, makepiece(o, rook)))
    {
	return rook;
    }
    if (slideattackedp(f, r, 0, -1, c, makepiece(o, rook)))
    {
	return rook;
    }
    if (kingattackedp(f,r,c,o))
    {
	return king;
    }

    return 0;
}

/* did the last move put c in check? */
int incheckp(int c)
{
    move m = lastmove();
    square end = TO(m);
    chesspiece p;
    
    int kf = F(board->kings[c]);
    int kr = R(board->kings[c]);

    p = getpiece__(end);
    
    switch (chesspiecevalue(p))
    {
	case pawn:
	    if (enpassantedp())
	    {
		return fullincheckp(c);
	    }
	    else
	    {
		int adf = abs(kf-F(end));
		int dr = kr-R(end);

		/* should this be -dir or dir? */
		if ((adf==1)&&(dr==-dir(c)))
		    return pawn;
	    }
	    break;
	    
	case knight:
	{
	    int adf = abs(F(end)-kf);
	    int adr = abs(R(end)-kr);

	    if ((adf==1)&&(adr==2))
		return knight;
	    if ((adr==1)&&(adf==2))
		return knight;
	}
	break;

	case king:
	    return fullincheckp(c);
	    
	case bishop:
	{
	    int df = F(end)-kf;
	    int dr = R(end)-kr;
	    int adf = abs(df);
	    int adr = abs(dr);

	    if (adf==adr)
		if (slideattackedp(kf,kr,signum(df),signum(dr),c,p))
		    return bishop;
	}
	break;
	
	case queen:
	{
	    int df = F(end)-kf;
	    int dr = R(end)-kr;

	    if (df==0)
	    {
		if (slideattackedp(kf,kr,0,signum(dr),c,p))
		    return queen;
	    }
	    else if (dr==0)
	    {
		if (slideattackedp(kf,kr,signum(df),0,c,p))
		    return queen;
	    }
	    else if (abs(df)==abs(dr))
	    {
		if (slideattackedp(kf,kr,signum(df),signum(dr),c,p))
		    return queen;
	    }
	}
	break;
	
	case rook:
	{
	    int df = F(end)-kf;
	    int dr = R(end)-kr;

	    if (df==0)
	    {
		if (slideattackedp(kf,kr,0,signum(dr),c,p))
		    return rook;
	    }
	    else if (dr==0)
	    {
		if (slideattackedp(kf,kr,signum(df),0,c,p))
		    return rook;
	    }
	}
	break;
    }
    /* look for discovery checks */
    {
	square start = FR(m);
	
	int df = F(start) - kf;
	int dr = R(start) - kr;

	if (df==0)
	    return slideattackedp(kf,kr,0,signum(dr),c,
				  makepiece(opp(c),rook));
	if (dr==0)
	    return slideattackedp(kf,kr,signum(df),0,c,
				  makepiece(opp(c),rook));
	if (abs(df)==abs(dr))
	    return slideattackedp(kf,kr,signum(df),signum(dr),c,
				  makepiece(opp(c),bishop));
	return 0;
    }
}

/*
  did this move m, which has been done on the board,
  put c into check?

  check to make sure the removal of this piece from its square did not
  open a rank or a file that is threatened by oppenent.
*/
int intocheckp(move m, int c)
{
    square end = TO(m);
    
    if (chesspiecevalue(getpiece__(end))==king)
    {
	return fullincheckp(c);
    }
    else if (enpassantedp())
    {
	return fullincheckp(c);
    }
    else
    {
	int kf, kr;
	int df, dr;
	
	square start = FR(m);

	kf = F(board->kings[c]);
	kr = R(board->kings[c]);

	df = F(start) - kf;
	dr = R(start) - kr;

	if (df==0)
	    return slideattackedp(kf,kr,0,signum(dr),c,
				  makepiece(opp(c),rook));
	if (dr==0)
	    return slideattackedp(kf,kr,signum(df),0,c,
				  makepiece(opp(c),rook));
	if (abs(df)==abs(dr))
	    return slideattackedp(kf,kr,signum(df),signum(dr),c,
				  makepiece(opp(c),bishop));
	return 0;
    }
}

int pawnattackedp(int f, int r, int c)
{
    chesspiece op = PIECE(opp(c), pawn);
    
    r-=dir(opp(c));

    if (f<7)
    {
	if (op == getpiece(f+1, r))
	    return 1;
    }
    
    if (f>0)
    {
	if (op == getpiece(f-1, r))
	    return 1;
    }
    
    return 0;
}

int knightattackedp(int f, int r, int c)
{
    int i;
    chesspiece n = makepiece(opp(c), knight);
    
    for (i=0;i<4;i++)
    {
	if (!offboardp(f+knightdx[i], r+knightdy[i]))
	    if (getpiece(f+knightdx[i], r+knightdy[i])==n)
	    {
		return 1;
	    }
	
	if (!offboardp(f+knightdy[i], r+knightdx[i]))
	    if (getpiece(f+knightdy[i], r+knightdx[i])==n)
	    {
		return 1;
	    }
    }
    return 0;
}

int slideattackedp(int f, int r, int df, int dr, int c, chesspiece p)
{
    chesspiece tp, q=makepiece(opp(c), queen);
    
    for (;;)
    {
	f+=df;
	r+=dr;
	
	if (offboardp(f,r))
	    return 0;

	tp = getpiece(f,r);
	if (tp)
	{
	    if (tp==p)
	    {
		return 1;
	    }
	    else if (tp==q)
	    {
		return queen;
	    }
	    return 0;
	}
    }
}

/* assumes that you are not currently in check */
int wouldbeincheckp(move m)
{
    int r;
    int c = tomove();
    domove(m);
    r = intocheckp(m,c);
    undomove();
    return r;
}

int incheck_lazy()
{
    int r = 0;
    
    move_and_score *restore = move_sp;

    genattacks();
    while (move_sp>restore)
    {
	move tm = popmove();
	
	if (TO(tm) == board->kings[opp(tomove())])
	{
	    r = getpiece__(FR(tm));
	    move_sp = restore;
	}
    }
    return r;
}


int wouldbe_lazy(move m)
{
    int r = 0,
	c = tomove();

    move_and_score *restore;
    
    domove(m);

    restore = move_sp;
    genattacks();
    while (move_sp>restore)
    {
	move tm = popmove();
	
	if (TO(tm) == board->kings[c])
	    r = getpiece__(FR(tm));
    }
    undomove();
    return r;
}

int wouldbeincheckfullp(move m)
{
/*      int r; */
/*      int c = tomove(); */
/*      domove(m); */
/*      r = fullincheckp(c); */
/*      undomove(); */
/*      return r; */
    return wouldbe_lazy(m);
}

int canmovep(int c)
{
    move_and_score *restore_sp = move_sp;

    genmoves(c);
    
    while (move_sp>restore_sp)
    {
	if (!wouldbeincheckfullp(popmove()))
	{
	    move_sp = restore_sp;
	    return 1;
	}
    }
    return 0;
}

int sufficientmaterial()
{
    if (board->material[WHITE]>=weights[ROOK_VALUE])
	return 1;
    if (board->material[BLACK]>=weights[ROOK_VALUE])
	return 1;
    if (board->pieces[WPAWN]>0)
	return 1;
    if (board->pieces[BPAWN]>0)
	return 1;
    if ((board->pieces[BBISHOP]==1)&&
	(board->pieces[WBISHOP]==1))
	return 1;
    return 0;
}

int draw_by_rep()
{
    int x, count = 0;
    
    for (x=0;x<ply;x++)
    {
	if (positions[x]==board->hash)
	{
	    count++;
	    if (count>=2) return 1;
	}
    }
    return 0;
}

int position_seen_before()
{
    int x;
    for (x=(tomove()==WHITE)?0:1;x<ply;x+=2)
    {
	if (positions[x]==board->hash)
	    return 1;
    }
    return 0;
}

int gameoverp(int c)
{
    if (draw_by_rep())
    {
	return REP_DRAW;
    }
    else if (!sufficientmaterial())
    {
	return NON_MATERIAL;		/* draw */
    }
    else if (!canmovep(c))
    {
	if (fullincheckp(c))
	{
	    return END_CHECKMATE;		/* checkmate */
	}
	else
	{
	    return END_STALEMATE;		/* stalemate */
	}
    }
    return IN_PROGRESS;			/* play on! */
}
