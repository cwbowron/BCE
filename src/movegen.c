/*
  This File is part of Boo's Chess Engine
  Copyright 2001 by Christopher Bowron
*/  

#include "bce.h"

void getmovese(int f, int r, int c);
void getmovesp(int f, int r, int c);
void getmovesn(int f, int r, int c);
void getmovesb(int f, int r, int c);
void getmovesr(int f, int r, int c);
void getmovesq(int f, int r, int c);
void getmovesk(int f, int r, int c);

void genattackse(int f, int r, int c);
void genattacksp(int f, int r, int c);
void genattacksn(int f, int r, int c);
void genattacksb(int f, int r, int c);
void genattacksr(int f, int r, int c);
void genattacksq(int f, int r, int c);
void genattacksk(int f, int r, int c);

void gensliderattacks(int f, int r, int df, int dr, int color);
void gensliderattacksDEC(square s, int dd, int color);
void gensliderattacksINC(square s, int dd, int color);
    
typedef void (*generator)(int, int, int);
typedef void (*attackgen)(int, int, int);

move_and_score move_stack[8096],*move_sp;

void pushmove(move m)
{
    move_sp->m = m;
    move_sp ++;
}

move popmove()
{
    move_sp--;
    return move_sp->m;
}

attackgen attackgens[]=
{
    genattackse,
    genattacksp,
    genattacksn,
    genattacksb,
    genattacksr,
    genattacksq,
    genattacksk
};

generator generators[]=
{
    getmovese,
    getmovesp,
    getmovesn,
    getmovesb,
    getmovesr,
    getmovesq,
    getmovesk
};

void genmoves()
{
    int f,r;

    int col = tomove();
    for (f=0;f<8;f++)
	for (r=0;r<8;r++)
	{
	    chesspiece p = getpiece(f,r);
	    if (p && (chesspiececolor(p) == col))
	    {
		int v = chesspiecevalue(p);
		generators[v](f,r,col);
	    }
	}
}

void genattacks()
{
    int f,r;

    int col = tomove();
    for (f=0;f<8;f++)
	for (r=0;r<8;r++)
	{
	    chesspiece p = getpiece(f,r);
	    
	    if (p && (chesspiececolor(p)==col))
	    {
		int v;
		v = chesspiecevalue(p);
		attackgens[v](f,r,col);
	    }
	}
}

void getmovese(int f, int r, int c)
{
    /* do nothing */
}

void getsliders(int f, int r, int df, int dr, int color)
{
    square s1 = SQ(f,r);
    chesspiece p;
    
    for (;;)
    {
	f += df;
	r += dr;

	if (offboardp(f,r)) return;
	
	p = getpiece(f,r);
	
	if (p == empty)
	    pushmove(MV(s1, SQ(f,r)));
	else if (chesspiececolor(p)==color)
	    return;
	else
	{
	    pushmove(MV(s1, SQ(f,r)));
	    return;
	}
    }
}

void getmovesp(int f, int r, int c)
{
    int forward = dir(c);
    square loc = SQ(f,r);
    
    r += forward;
    
    if (f<7)
	if (oppenentp (f+1, r, c))
	    pushmove(MV(loc, SQ(1+f, r)));
    
    if (f>0)
	if (oppenentp (f-1, r, c))
	    pushmove(MV(loc, SQ(f-1, r)));

    if (emptyp(f,r))
    {
	int startrank = ((c == WHITE) ? 2 : 5);
	pushmove(MV(loc, SQ(f,r)));

	/* can go two on first move */
	if ((r == startrank)&&(emptyp(f,r+forward)))
	    pushmove(MV(loc, SQ(f,r+forward)));
    }

    if (doublepushp())
    {
	square start;
	square end;
	move m;

	m = lastmove();
	
	start = FR(m);
	end = TO(m);
	
	r -= forward;		/* restore to original r */
	if (R(end) == r)
	{
	    if (((f+1) == F(start))||
		((f-1) == F(start)))
		pushmove(MV(loc, SQ(F(start),forward+r)));
	}
    }
}

const int df[] = { 1,  1, -1, -1, 2, -2,  2, -2 };
const int dr[] = { 2, -2,  2, -2, 1,  1, -1, -1 };

void getmovesn(int f, int r, int c)
{
    int i, nf, nr;
    int o;
    chesspiece p;
    square s1;

    s1 = SQ(f,r);
    o = opp(c);

    for (i=0;i<8;i++)
    {
	nf = f+df[i];
	nr = r+dr[i];

	if (offboardp(nf,nr))
	    continue;

	p = getpiece(nf, nr);
	if (!p || (c != chesspiececolor(p)))
	{
	    pushmove(MV(s1, SQ(nf, nr)));
	}
    }
}

void getmovesb(int f, int r, int c)
{
    getsliders(f,r,1,1,c);
    getsliders(f,r,1,-1,c);
    getsliders(f,r,-1,1,c);
    getsliders(f,r,-1,-1,c);
}

void getmovesr(int f, int r, int c)
{
    getsliders(f,r, 1, 0,c);
    getsliders(f,r, 0, 1,c);
    getsliders(f,r,-1, 0,c);
    getsliders(f,r, 0,-1,c);
}

void getmovesq(int f, int r, int c)
{
    getmovesr(f,r,c);
    getmovesb(f,r,c);
}

#define THRUWK MV(E1, F1)
#define THRUWQ MV(E1, D1)
#define THRUBK MV(E8, F8)
#define THRUBQ MV(E8, D8)

void getmovesk(int f, int r, int c)
{
    int df, dr, nf, nr;
    int o;
    chesspiece p;
    
    square s1 = SQ(f,r);
    o = opp(c);
    
    for (df=-1;df<2;df++)
    for (dr=-1;dr<2;dr++)
    {
	if (!df && !dr) continue;
	
	nf = f+df;
	nr = r+dr;
	p = getpiece(nf, nr);
	if (!offboardp(nf, nr) && (!p || (c != chesspiececolor(p))))
	    pushmove(MV(s1, SQ(nf, nr)));
    }
    
    if (c==WHITE)
    {
	if (wqcastlep()&&
	    !getpiece__(D1)&&
	    !getpiece__(C1)&&
	    !getpiece__(B1)&&
	    !incheckp(c)&&
	    !wouldbeincheckp(THRUWQ))
	{
	    pushmove(WQC);
	}
	if (wkcastlep()&&
	    !getpiece__(F1)&&
	    !getpiece__(G1)&&
	    !incheckp(c)&&
	    !wouldbeincheckp(THRUWK))
	{
	    pushmove(WKC);
	}
    }
    else
    {
	if (bqcastlep()&&
	    !getpiece__(D8)&&
	    !getpiece__(C8)&&
	    !getpiece__(B8)&&
	    !incheckp(c)&&
	    !wouldbeincheckp(THRUBQ))
	{
	    pushmove(BQC);
	}
	if (bkcastlep()&&
	    !getpiece__(F8)&&
	    !getpiece__(G8)&&
	    !incheckp(c)&&
	    !wouldbeincheckp(THRUBK))
	{
	    pushmove(BKC);
	}
    }
}

/* generate pawn captures and promotions */
void genattacksp(int f, int r, int c)
{
    int forward = dir(c);
    square loc = SQ(f,r);
    
    r += forward;
    
    if (f<7)
    {
	chesspiece p = getpiece(f+1, r);
	if (p&&chesspiececolor(p)!=c)
	    pushmove(MV(loc, SQ(f+1, r)));
    }
    
    if (f>0)
    {
	chesspiece p = getpiece(f-1,r);
	if (p&&chesspiececolor(p)!=c)
	    pushmove(MV(loc, SQ(f-1, r)));
    }

    if (c==WHITE)
    {
	if ((r==6)&&emptyp(f,r))
	    pushmove(MV(loc, SQ(f,r)));
    }
    else
    {
	if ((r==1)&&emptyp(f,r))
	    pushmove(MV(loc, SQ(f,r)));
    }
    
    /* get en passante */
    if (doublepushp())
    {
	square start;
	square end;
	move m;

	m = lastmove();
	
	start = FR(m);
	end = TO(m);
	
	r -= forward;		/* restore to original r */
	if (R(end) == r)
	{
	    if (((f+1) == F(start))||
		((f-1) == F(start)))
		pushmove(MV(loc, SQ(F(start),forward+r)));
	}
    }
}

void genattacksn(int f, int r, int c)
{
    int i, nf, nr;
    int o;
    square s1 = SQ(f,r);
    chesspiece p;
    
    o = opp(c);

    for (i=0;i<8;i++)
    {
	nf = f+df[i];
	nr = r+dr[i];

	if (offboardp(nf,nr))
	    continue;
	
	p = getpiece(nf, nr);
	if (p && (c != chesspiececolor(p)))
	{
	    pushmove(MV(s1, SQ(nf, nr)));
	}
    }
}

void genattacksb(int f, int r, int c)
{
    gensliderattacks(f,r,1,1,c);
    gensliderattacks(f,r,1,-1,c);
    gensliderattacks(f,r,-1,1,c);
    gensliderattacks(f,r,-1,-1,c);
}

void genattacksr(int f, int r, int c)
{
    gensliderattacks(f,r, 1, 0,c);
    gensliderattacks(f,r, 0, 1,c);
    gensliderattacks(f,r,-1, 0,c);
    gensliderattacks(f,r, 0,-1,c);
}

void genattacksq(int f, int r, int c)
{
    genattacksb(f,r,c);
    genattacksr(f,r,c);
}

void genattacksk(int f, int r, int c)
{
    int df, dr, nf, nr;
    int o;
    chesspiece p;
   
    square s1 = SQ(f,r);
    o = opp(c);
    
    for (df=-1;df<2;df++)
    for (dr=-1;dr<2;dr++)
    {
	if (!df && !dr) continue;
	
	nf = f+df;
	nr = r+dr;
	
	if (!offboardp(nf, nr)&&
	    (p = getpiece(nf, nr))&&
	    (c != chesspiececolor(p)))
	    pushmove(MV(s1, SQ(nf, nr)));
    }
}

void genattackse(int f, int r, int c)
{
    /* do nothing */
}

void gensliderattacks(int f, int r, int df, int dr, int color)
{
    chesspiece p;
    square s1=SQ(f,r);
    
    for (;;)
    {
	f += df;
	r += dr;

	if (offboardp (f,r)) return;
	
	p = getpiece(f,r);
	
	if (p)
	{
	    if (chesspiececolor(p) != color)
	    {
		pushmove(MV(s1, SQ(f,r)));
		return;
	    }
	    return;
	}
    }
}
#ifdef NOT
void gensliderattacksINC(square s, int dd, int color)
{
    chesspiece p;
    square s2 = s;
    
    for (;;)
    {
	s2 += dd;

	if ((s2&007)==0)	/* if at RANK 1 we have looped */
	    break;
	if ((s2&~(077))!=0)
	    break;
	
	p = getpiece__(s2);
	
	if (p)
	{
	    if (chesspiececolor(p) != color)
	    {
		pushmove(MV(s, s2));
		return;
	    }
	    return;
	}
    }
}

void gensliderattacksDEC(square s, int dd, int color)
{
    chesspiece p;
    square s2 = s;
    
    for (;;)
    {
	s2 += dd;		/* dd should be negative */

	if ((s2&001)==0)	/* if at RANK 8 then we have looped  */
	    break;
	if ((s2&~(077))!=0)
	    break;
	
	p = getpiece__(s2);
	
	if (p)
	{
	    if (chesspiececolor(p) != color)
	    {
		pushmove(MV(s, s2));
		return;
	    }
	    return;
	}
    }
}
#endif
