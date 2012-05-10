int adv=0;			/* white's advantage */
int f,r;

rooks[0] = -1;
rooks[1] = -1;

if (!mate_material(WHITE))
{
    if (!mate_material(BLACK))
    {
	return 0;
    }
    adv += weights[NOMATERIAL];
    verbose_print(8,8,weights[NOMATERIAL],"no mating material (white)");
}
else if (!mate_material(BLACK))
{
    adv += -weights[NOMATERIAL];
    verbose_print(8,8,-weights[NOMATERIAL],"no mating material (black)");
}

for (f=0;f<8;f++) {
    for (r=0;r<8;r++) {
	switch(getpiece(f,r)) {
	    case WQUEEN:
		adv += score_queen(f,r,WHITE);
		break;
	    case BQUEEN:
		adv -= score_queen(f,r,BLACK);
		break;
	    case WROOK:
		adv += score_rook(f,r,WHITE);
		break;
	    case BROOK:
		adv -= score_rook(f,r,BLACK);
		break;
	    case WPAWN:
		adv += score_pawn(f,r,WHITE);
		break;
	    case BPAWN:
		adv -= score_pawn(f,r,BLACK);
		break;
	    case WKNIGHT:
		adv += score_knight(f,r,WHITE);
		break;
	    case BKNIGHT:
		adv -= score_knight(f,r,BLACK);
		break;
/*  	    case WBISHOP: */
/*  		adv += score_bishop(f,r,WHITE); */
/*  		break; */
/*  	    case BBISHOP: */
/*  		adv -= score_bishop(f,r,BLACK); */
/*  		break; */
	}
    }
}

if (board->pieces[WBISHOP]>=2) {
    adv += weights[TWOBISHOPS];
    verbose_print(8,8,weights[TWOBISHOPS], "bishop pair");
}

if (board->pieces[BBISHOP]>=2) {
    adv -= weights[TWOBISHOPS];
    verbose_print(8,8,-weights[TWOBISHOPS], "bishop pair");
}

if (wkcastledp()) {
    verbose_print(8,8,weights[KCASTLEBONUS], "castled king side");
    adv+=weights[KCASTLEBONUS];
}
else if (wqcastledp()) {
    verbose_print(8,8,weights[QCASTLEBONUS], "castled queen side");
    adv+=weights[QCASTLEBONUS];
}
else {
    if (!wkcastlep()) {
	verbose_print(8,8,-weights[NOCASTLEKING],
		      "lost castling rights (kingside)");
	adv-=weights[NOCASTLEKING];
    }
    if (!wqcastlep()) {
	verbose_print(8,8,-weights[NOCASTLEQUEEN],
		      "lost castling rights (queenside)");
	adv-=weights[NOCASTLEQUEEN];
    }
}

if (bkcastledp())
{
    verbose_print(8,8,-weights[KCASTLEBONUS], "castled king side");
    adv-=weights[KCASTLEBONUS];
}
else if (bqcastledp())
{
    verbose_print(8,8,-weights[QCASTLEBONUS], "castled queen side");
    adv-=weights[QCASTLEBONUS];
}
else
{
    if (!bkcastlep()) {
	verbose_print(8,8,weights[NOCASTLEKING],
		      "lost castling rights (kingside)");
	adv+=weights[NOCASTLEKING];
    }
    if (!bqcastlep()) {
	verbose_print(8,8,weights[NOCASTLEQUEEN],
		      "lost castling rights (queenside)");
	adv+=weights[NOCASTLEQUEEN];
    }
}

verbose_printf(("adv: %6d\n", adv));
verbose_printf(("material: WHITE %6d\tBLACK %6d\n",
		board->material[WHITE], board->material[BLACK]));
verbose_printf(("position: WHITE %6d\tBLACK %6d\n",
		board->position[WHITE], board->position[BLACK]));
verbose_printf(("material advantage: %6d\n", material_advantage()));
verbose_printf(("mobility: %6d\n", mobility_advantage()));

adv += board->position[WHITE]-board->position[BLACK];
adv += material_advantage();
adv += mobility_advantage();

#ifdef VERBOSE
for(r=7;r>=0;r--)
{
    for (f=0;f<8;f++)
    {
	chesspiece p = getpiece(f,r);
	if (p)
	    printf(" %c=%3d ", rep(p), squarevalue(f,r,p));
	else
	    printf("       ");
    }
    printf("\n");
}

#endif /* VERBOSE */

return (tomove() == WHITE) ? adv : -adv;
