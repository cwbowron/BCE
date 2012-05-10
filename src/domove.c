/*
  This file is part of Boo's Chess Engine
  Copyright 2000 by Christopher Bowron
*/  

#include "bce.h"
#include <stdio.h>

int ply;
move gamestack[MAXMOVES];
undo undostack[MAXMOVES];

hashtype positions[MAXMOVES];

int fake_count = 0;

/* clear flags that last only one ply */
void clear_flags()
{
    board->flags = (board->flags & ~(enp|dbl|cas|promo_flag));
}


void fake_domove()
{
    undostack[ply].flags = board->flags;
    gamestack[ply] = dummymove;

    positions[ply] = board->hash;

    board -> hash ^= 1;
    
/*      board->flags = (board->flags & ~(enp|dbl|cas|promo_flag)); */
    clear_flags();
    
    ply++;
    fake_count++;
    
    switch_sides();
}

void fake_undomove()
{
    ply--;
    fake_count--;
    
    board->flags = undostack[ply].flags;
    board->hash = positions[ply];
}

void domove(move m)
{
    square start, end;
    chesspiece p, captured;
    int v;

    start = FR(m);
    end = TO(m);

    p = getpiece__(start);
    v = VALUE(p);
    captured = getpiece__(end);

    undostack[ply].m = m;
    undostack[ply].captured = captured;
    undostack[ply].moved = p;
    undostack[ply].flags = board->flags;
    
    gamestack[ply] = m;

    positions[ply] = board->hash;

    board -> hash ^= 1;
    
    /* clear flags that last only one ply */
/*      board->flags = (board->flags & ~(enp|dbl|cas|promo_flag)); */
    clear_flags();

    setpiece__(end, p);
    setpiece__(start, empty);
    
    updatematerial__(end, captured, 0);
    updatemoved(start,end,p);

    switch (start)
    {	case A1:board->flags &= ~(wqc);break;
	case A8:board->flags &= ~(bqc);break;
	case H8:board->flags &= ~(bkc);break;
	case H1:board->flags &= ~(wkc);break;
    }
    
    switch (end)
    {	case A1:board->flags &= ~(wqc);break;
	case A8:board->flags &= ~(bqc);break;
	case H1:board->flags &= ~(wkc);break;
	case H8:board->flags &= ~(bkc);break;
    }

    switch (v)
    {
	case pawn:
	{
	    int ef = F(end);
	    int er = R(end);
	    int sf = F(start);
	    int sr = R(start);
	    
	    switch (er)
	    {

		case RANK4:
		    sr = R(start);		    
		    if ((er-sr)==2)
			board->flags |= dbl;
		    break;
		    
		case RANK5:	
		    sr = R(start);	    
		    if ((sr-er)==2)
			board->flags |= dbl;
		    break;

		case RANK6:
		case RANK3:
		    sf = F(start);

		    if ((sf!=ef)&&!captured)
		    {
			sr = R(start);
			board->flags |= enp;
			updatematerial(ef, sr, p^1, 0);
			setpiece(ef, sr, empty);
		    }
		    break;

		case RANK1:
		case RANK8:
		{
		    chesspiece promo = getpromo();
		    setpiece__(end, promo);
		    updatematerial__(end,p,0);
		    updatematerial__(end,promo,1);
		    board->flags |= promo_flag;
		}
		break;
	    }
	}
	break;
	    
	case king:
	    if (m==BQC)
	    {
		board->flags |= (cas | bqcastled);
		setpiece__(D8, BROOK);
		setpiece__(A8, empty);
		updatemoved(A8, D8, BROOK);
	    }
	    else if (m==BKC)
	    {
		board->flags |= (cas | bkcastled);
		setpiece__(F8, BROOK);
		setpiece__(H8, empty);
		updatemoved(H8, F8, BROOK);
	    }
	    else if (m==WQC)
	    {
		board->flags |= (cas | wqcastled);
		setpiece__(D1, WROOK);
		setpiece__(A1, empty);
		updatemoved(A1, D1, WROOK);
	    }
	    else if (m==WKC)
	    {
		board->flags |= (cas | wkcastled);
		setpiece__(F1, WROOK);
		setpiece__(H1, empty);
		updatemoved(H1, F1, WROOK);
	    }
	    board->kings[tomove()] = end;
	    board->flags &= (tomove()==WHITE) ? ~(wkc|wqc) : ~(bkc|bqc);
	    break;
    }

    ply++;

    switch_sides();
}

void undomove()
{
    move m;
    chesspiece p;
    chesspiece captured;
    square start, end;

    /* if the last move was actually a null move just fake it */
    if (gamestack[ply-1] == dummymove)
    {
/*  	printf("whoa nelly\n"); */
	fake_undomove();
	return;
    }
    
    ply--;

    m = undostack[ply].m;
    p = undostack[ply].moved;
    captured = undostack[ply].captured;
    
    start = FR(m);
    end = TO(m);

    if (board->flags & (cas|enp|promo_flag))
    {
	if (board->flags & promo_flag)
	{
	    updatematerial__(end, getpiece__(end),0);	/* remove promo */
	    updatematerial__(end, p, 1);		/* restore pawn */
	}
	else if (board->flags & enp)
	{
	    int ef = F(end);
	    int sr = R(start);

	    setpiece(ef, sr, p^1);
	    updatematerial(ef, sr, p^1, 1);
	}
	else if (board->flags & cas)
	{
	    board->kings[COLOR(p)] = start;
	    switch (m)
	    {
		case WQC:
		    setpiece__(A1, WROOK);
		    setpiece__(D1, empty);
		    updatematerial__(A1, WROOK, 1);
		    updatematerial__(D1, WROOK, 0);
		    break;
		
		case WKC:
		    setpiece__(H1, WROOK);
		    setpiece__(F1, empty);
		    updatematerial__(G1, WROOK, 1);
		    updatematerial__(F1, WROOK, 0);
		    break;
		
		case BQC:
		    setpiece__(A8, BROOK);
		    setpiece__(D8, empty);
		    updatematerial__(A8, BROOK, 1);
		    updatematerial__(D8, BROOK, 0);
		    break;
		
		case BKC:
		    setpiece__(H8, BROOK);
		    setpiece__(F8, empty);
		    updatematerial__(H8, BROOK, 1);
		    updatematerial__(F8, BROOK, 0);
		    break;
	    }
	}
    }
    else if (p == WKING)
	board->kings[WHITE] = start;
    else if (p == BKING)
	board->kings[BLACK] = start;
    
    setpiece__(start, p);
    setpiece__(end, captured);
    
    updatematerial__(end, captured, 1);
    updatemoved(end, start, p);

    board->flags = undostack[ply].flags;
    board->hash = positions[ply];
}


void updatematerial__(int i, chesspiece p, int add)
{
    if (p)
    {
	int v, c, value, sq;

	v = VALUE(p);
	c = COLOR(p);
	value = weights[EMPTY_VALUE+v];
	sq = squarevalue__(i,p);

	board->hash ^= randoms[p][i];
	
	if (add)
	{
	    board->pieces[p]++;
	    board->material[c] += value;
	    board->position[c] += sq;
	    
	    if (v==pawn)
	    {
		board->pawns[c][F(i)+1] += 1;
/*  		board->pawnbits[c] |= (BIT__(i)); */
	    }
	    else
	    {
		board->piececount[c]++;
	    }
	}
	else
	{
	    board->pieces[p]--;
	    board->material[c] -= value;
	    board->position[c] -= sq;
	    
	    if (v==pawn)
	    {
		board->pawns[c][F(i)+1] -= 1;
/*  		board->pawnbits[c] &= ~(BIT__(i)); */
	    }
	    else
	    {
		board->piececount[c]--;
	    }
	}
    }
}

void updatematerial(int f, int r, chesspiece p, int add)
{
    updatematerial__(SQ(f,r),p,add);
}

void updatemoved(square start, square end, chesspiece p)
{
    updatematerial__(start, p, 0);
    updatematerial__(end, p, 1);
}

chesspiece getpromo()
{
    if (computer[tomove()])
	return PIECE(tomove(), queen);

    return PIECE(tomove(), promopiece);
}
