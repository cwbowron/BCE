/*
  This File is part of Boo's Chess Engine
  Copyright 2000 by Christopher Bowron
*/  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>

#include "bce.h"

const char * FEATURES = "feature ping=0 sigint=1 sigterm=0 myname=\"BCE\" colors=1 san=0 done=1";

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
struct cmd { char *name; void (*func)(char*); char *help; };
struct cmd commands[];

struct v_struct { char *name; void *value;};
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void tell_owner(char *format, ...)
{
  static char buf[16384];
  va_list ap;

  if (robo_mode&&explain_mode)
  {
      buf[0] = '\0';
      va_start(ap, format);
      vsprintf(buf, format, ap);
      va_end(ap);
      printf("tellics tell madhacker %s", buf);
  }
}

void whisper(char *format, ...)
{
    static char buf[16384];
    va_list ap;
    
    if (robo_mode)
    {
	buf[0] = '\0';
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	printf("tellics whisper %s", buf);
    }
}

void tellics(char *format, ...)
{
    static char buf[16384];
    va_list ap;
    
    if (robo_mode)
    {
	buf[0] = '\0';
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	printf("tellics %s", buf);
    }
}

void output(char *format, ...)
{
    static char buf[16384];
    va_list ap;
    
    buf[0] = '\0';
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    
    if (robo_mode)
    {
	if (explain_mode)
	    printf("tellics whisper %s", buf);
    }
    else
	printf("%s", buf);
}

void cmd_big(char *s){printboard = printboardbig;}
void cmd_blind(char *s){printboard=printboardblind;}
void cmd_small(char *s){printboard = printboardsmall;}

void cmd_pawns(char *s)
{
    printf("WHITE\n");
    printbitboard(board->pawnbits[0]);
    printf("BLACK\n");
    printbitboard(board->pawnbits[1]);
}

/*
+----
| functions that affect who is play each color
+---- 
*/
void cmd_black(char *s)
{
    computer[BLACK]=0;
    computer[WHITE]=searchdepth;
    board->flags |= turn_flag;
}

void cmd_white(char *s)
{
    computer[WHITE]=0;
    computer[BLACK]=searchdepth;
    board->flags &= ~turn_flag;
}

void cmd_force(char *s)
{
    computer[0]=0;
    computer[1]=0;
}

void cmd_go(char *s)
{
    computer[opp(tomove())]=0;
    computer[tomove()]=searchdepth;
}

void cmd_demo(char *s)
{
    computer[0] = computer[1] = searchdepth;
}

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
void cmd_book(char *s){loadbook(bookfile);}
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
void cmd_easy(char *s){ponder_mode = 0;}
void cmd_hard(char *s){ponder_mode = 1;}
void cmd_noexplain(char *s){explain_mode=0;}
void cmd_nopost(char *s){thinking_mode = 0;}
void cmd_post(char *s){thinking_mode = 1;}
void cmd_explain(char *s){explain_mode=1;}
void cmd_random(char *s){}

void cmd_eval(char *s){printf("Advantage : %4d\n", evaluate_verbose());}
void cmd_moves(char *s){printmoves(tomove());}
void cmd_redraw(char *s){printboard();}

void cmd_exit(char *s){exit(0);}

void cmd_ignore(char *s){/* do nothing */}
void cmd_suggest(char *s){printf("Hint: %s\n", movestring(bce()));}
void cmd_time(char *s){int v;sscanf(s, "%d", &v);chessclock[opp(tomove())] = v*10;}
void cmd_otim(char *s){int v;sscanf(s, "%d", &v);chessclock[tomove()] = v*10;}

void cmd_xboard(char *s)
{
    printboard = printboardblind;
    xboard_mode = 1;
}

void cmd_robofics(char *s)
{
    robo_mode=1;
    printboard = printboardblind;
    thinking_mode = 0;
}

void cmd_undo(char *s)
{
    computer[0] = 0;
    computer[1] = 0;
    
    if (ply) undomove();
}

void cmd_backup(char *s)
{
    int i;
    for (i=0;i<2;i++)
	cmd_undo(NULL);
}

void cmd_new(char *s)
{
    if (gameoverp(tomove()) == IN_PROGRESS)
    {
	if (weight_mode)
	    update_weights(-2);
    }
    
    initialize();
}

void cmd_draw(char *s)
{
    printf("Request Denied\n");
}

void cmd_board(char *s)
{
    int f,r;
    for (r=7;r>=0;r--)
    {
	for (f=0;f<8;f++)
	{
	    chesspiece p=getpiece(f,r);
	    printf("%c %2d %2d\t", rep(p),
		   chesspiececolor(p), chesspiecevalue(p));
	}
	printf("\n");
    }
    printf("piece counts [%2d %2d]\n", board->piececount[0], board->piececount[1]);
    
    for (f=WPAWN;f<=BKING;f++)
    {
	printf("%c = %2d\n", rep(f), board->pieces[f]);
    }
}

void cmd_hash(char *s)
{
    int i;
  
    printf("\tcur hash: %25llu\n", board->hash);
    for (i=ply-1;i>=0;i--)
    {
	printf("\tpos[%2d] = %25llu\n", i, positions[i]);
    }
}


void cmd_name(char *s)
{
#ifdef NOT
    char str[80];
    FILE *in = fopen("history.txt","r+");
    FILE *out = fopen("history.temp.txt", "w+");
    int found = 0;

    s++;

    if (in)
    {
	while (!feof(in))
	{
	    char *p;
	    
	    if (!fgets(str, 80, in))
		break;
	    
	    fprintf(out, "%s", str);
	    
	    if((p = strchr(str, '\n')) != NULL)
		*p = '\0';
	    if (!strcmp(s,str))
		found = 1;
	}
    }
    
    if (!found)
    {
	printf("oppenent not found in history %s\n", s);
	fprintf(out, "%s\n", s);
    }
    
    fclose(in);
    fclose(out);
    rename("history.temp.txt", "history.txt");
    remove("history.temp.txt");
#endif    
}
	
void cmd_game(char *s)
{
    int i;
    for(i=0;i<ply;i++)
    {
	char *s = movestring(gamestack[i]);
	if (i%2==0)
	    printf("\tW %2d %s\t", (i+1)/2+1, s);
	else
	    printf("\tB %2d %s\n", i/2+1, s);
    }
    if (ply%2==1) printf("\n");
}


void cmd_kings(char *s)
{
    char *str;
    
    str = squarestring(board->kings[WHITE]);
    printf("\tWhite King : %s\n", str);
    free(str);

    str = squarestring(board->kings[BLACK]);
    printf("\tBlack King : %s\n", str);
    free(str);
}


void cmd_set(char *s)
{
    struct v_struct var_structs[]=
    {
	{ "searchdepth", &searchdepth },
	{ "searchtime", &searchtime },
	{ "highlight", &highlight },
	{ "wclock", &chessclock[WHITE] },
	{ "bclock", &chessclock[BLACK] },
	{ "inc", &clockinc },
	{ "promopiece", &promopiece},
	{ "pawn",   &weights[PAWN_VALUE] },
	{ "knight", &weights[KNIGHT_VALUE] },
	{ "bishop", &weights[BISHOP_VALUE] },
	{ "rook",   &weights[ROOK_VALUE] },
	{ "queen",  &weights[QUEEN_VALUE] },
	{ "search", &search },
	{ "explain", &explain_mode },
	{ NULL, NULL }
    };
    
    if (s)
    {
	int found = 0;
	int i;
	char *v = malloc(strlen(s)+1);
	char *valuestr = malloc(strlen(s)+1);
	memset(v, 0, strlen(s));
	
	sscanf(s, "%s %s", v, valuestr);

	for (i=0; var_structs[i].name; i++)
	{
	    if (!strcasecmp(var_structs[i].name, v))
	    {
		if (strcmp(v,"search"))
		{
		    int value;
		    sscanf(valuestr, "%d", &value);
		    *((int *)var_structs[i].value) = value;
		    printf("\tSetting %s to %d\n", v, value);
		}
		else
		{
/*  		    if (!strcmp(valuestr,"ab")) */
/*  			search=alphabeta_search; */
/*  		    else if (!strcmp(valuestr,"abt")) */
/*  			search=alphabeta_tables; */
/*  		    else if (!strcmp(valuestr,"ns")) */
/*  			search=negascout_search; */
/*  		    else if (!strcmp(valuestr,"nst")) */
/*  			search=negascout_tables; */
/*  		    else if (!strcmp(valuestr,"nm")) */
/*  			search=negamax_search; */
/*  		    else */
/*  			printf("invalid value for search\n"); */
		}
		found = 1;
	    }
	}
	
	if (!found)
	{
	    if (strcmp(v,""))
		printf("\tVariable not recognized\n");
	    
	    printf("\t- Available variables -\n");
	    for (i=0; var_structs[i].name; i++)
	    {
		if (strcmp(var_structs[i].name,"search"))
		{
		    output("\t%-12s = %10d\n", var_structs[i].name,
			   *((int*)var_structs[i].value));
		}
		else
		{
/*  		    if (search==alphabeta_search) */
/*  			output("\t%-12s = %10s\n", var_structs[i].name, "ab"); */
/*  		    if (search==alphabeta_tables) */
/*  			output("\t%-12s = %10s\n", var_structs[i].name, "abt"); */
/*  		    else if (search==negascout_search) */
/*  			output("\t%-12s = %10s\n", var_structs[i].name, "ns"); */
/*  		    else if (search==negascout_tables) */
/*  			output("\t%-12s = %10s\n", var_structs[i].name, "nst"); */
/*  		    else if (search==negamax_search) */
/*  			output("\t%-12s = %10s\n", var_structs[i].name, "nm"); */
		}
	    }
	}
	free(v);
	free(valuestr);
    }
    countmaterial();
}


void cmd_attacks(char *s)
{
    move_and_score *restore_sp = move_sp;

    genattacks();
    
    if (move_sp>restore_sp)
    {
	while (move_sp>restore_sp)
	{
	    move m = popmove();
	    printf("%8.8lx - %8lu - %s\n", (unsigned long)m, (unsigned long)m,movestring(m));
	}	    
    }
    else
	printf("\tno available attacks\n");
}

void cmd_sd(char *s)
{
    int depth;
    
    if (sscanf(s, " %d", &depth))
    {
	searchdepth = depth;
	printf("\tassigning searchdepth = %d\n", searchdepth);
    }
    else
    {
	printf("\tError reading searchdepth\n");
    }
}


#define flag_test(test, pos) (test ? pos : '-')

char *formatflags()
{
    static char fmt[20];
    
    fmt[0] = flag_test(wqcastlep(), 'Q');
    fmt[1] = flag_test(wkcastlep(), 'K');
    fmt[2] = flag_test(bqcastlep(), 'q');
    fmt[3] = flag_test(bkcastlep(), 'k');
    fmt[4] = flag_test(castledp(), 'C');
    fmt[5] = flag_test(enpassantedp(), 'E');
    fmt[6] = flag_test(doublepushp(), 'P');
    fmt[7] = flag_test(wkcastledp(), 'K');
    fmt[8] = flag_test(wqcastledp(), 'Q');
    fmt[9] = flag_test(bkcastledp(), 'k');
    fmt[10] = flag_test(bqcastledp(), 'q');
    fmt[11] = tomove() == WHITE ? 'W' : 'B';
    fmt[12] = 0;

    return fmt;
}

void cmd_inspect(char *s)
{
    int i;
  
    output("\tPawns : ");
    output("W : "); 
    for (i=1;i<9;i++) output("%d",board->pawns[0][i]);
    output(" B : ");
    for (i=1;i<9;i++) output("%d",board->pawns[1][i]);
    output("\n");

    output("\tFlags : %s\n", formatflags());

    if (ply)
    {
	undo u = undostack[ply-1];
	int v = board->flags;
      
	board->flags = u.flags;
	output("\tprevious move : %c took %c = %s : %s\n",
	       rep(u.moved), rep(u.captured), movestring(u.m), formatflags());
	board->flags = v;
    }
  
    if (fullincheckp(tomove()))
    {
	output("\tYou are currently in check : %d\n", fullincheckp(tomove()));
    }
}

void cmd_default_weights(char *s)
{
    free(base_weights);
    setup_default_weights();
}


void cmd_save(char *s)
{
    FILE *out;
    char *filename;
    
    int i;
    
    filename = (s[1] == 0) ? "bce.saved" : ++s;

    out = fopen(filename, "w");
    if (!out)
    {
	printf("error opening file %s\n", filename);
	return;
    }
    
    for (i=0;i<ply-1;i++)
    {
	fprintf(out, "%d\n", gamestack[i]);
    }
    
    printf("- Game Saved as %s -\n", filename);
    fclose(out);
}

chesspiece lookup(char ch)
{
  char wbase[] = " PNBRQK";
  char bbase[] = " pnbrqk";
  
  char *p;
  
  if (ch == ' ')
      return empty;
  
  if ((p=strchr(wbase, ch)))
      return makepiece(WHITE, p-wbase);

  if ((p=strchr(bbase, ch)))
      return makepiece(BLACK, p-bbase);

  return empty;
}

void cmd_load(char *s)
{
    FILE *in;
    move m;
    char *filename;

    filename = (s[1] == 0) ? "bce.saved" : ++s;

    in = fopen(filename, "r");

    if (!in)
    {
	printf("error opening saved game: %s\n", filename);
	return;
    }

    initialize();

    computer[0] = 0;
    computer[1] = 0;
    
    while(!feof(in))
    {
	fscanf(in, "%u", &m);
	if (feof(in)) break;
	domove(m);
/*  	printf("Move: %s\n", movestring(m)); */
    }
    
    printf("- Board Loaded -\n");
    printboard();
    countmaterial();
    
    fclose(in);
}

void cmd_fen(char *s)
{
    char pos[80];
    char color;
    char castling[4];
    char enpassante[2];
    int halfmove, fullmove;
    int f, r, i;

    FILE *in = fopen(s+1, "r");

    if (!in)
    {
	printf("cmd_fen(): error opening %s\n", s+1);
	return;
    }

    change_gamemode(MIDGAME, 1);
    
    fscanf(in, "%s %c %s %s %d %d", pos, &color, castling, enpassante, &halfmove, &fullmove);

    printf("pos: %s\ncolor: %c\ncastling: %s\nenpassante: %s\n",
	   pos, color, castling, enpassante);

    for (i=0;i<64;i++) setpiece__(i,empty);

    f = 0;
    r = 7;
    for (i=0;i<strlen(pos);i++)
    {
	chesspiece p;

	if (pos[i]=='/')
	{
	    r--;
	    f=0;
	    printf("new rank: %d\n", r);
	}
	else if ((pos[i]>='1')&&(pos[i]<='9'))
	{
	    f += pos[i]-'0';
	}
	else if ((p=lookup(pos[i])))
	{
	    setpiece(f,r,p);
	    printf("%c @ %c%d\n", rep(p), f+'a',r);
	    f++;
	}
	else
	{
	    printf("unknown character in fen position string: %s[%d]\n", pos, i);
	    exit(EXIT_FAILURE);
	}
    }
    
    countmaterial();
    board->flags = 0;
    ply = 0;
    
    if (!strcmp(enpassante, "-"))
    {
	gamestack[ply]=dummymove;
    }
    else
    {
	int file = enpassante[0]-'a';
	int rank = enpassante[1]-'1';

	board->flags |= enp;
	
	if (rank == 4)
	    gamestack[ply] = MV(SQ(file,6), SQ(file,4));
	else if (rank==3)
	    gamestack[ply] = MV(SQ(file,1), SQ(file,3));
	else
	    printf("cmd_fen(): error parsing enpassante square\n");
    }
    ply++;
    
    if (strchr(castling, 'Q'))
	board->flags |= wqc;
    if (strchr(castling, 'K'))
	board->flags |= wkc;
    if (strchr(castling, 'q'))
	board->flags |= bqc;
    if (strchr(castling, 'k'))
	board->flags |= bkc;
    
    if (color=='w')
	board->flags &= ~turn_flag;

    else if (color=='b')
	board->flags |= turn_flag;
    else
	printf("cmd_fen(): error parsing color\n");
}

void cmd_help(char *s)
{
    int i=0;

    printf("\tAvailable Commands\n");
    while (commands[i].name)
    {
	printf("\t%-10s %s\n", commands[i].name, commands[i].help);
	i++;
    }
}

void cmd_info(char *s)
{
    credits();
}

void cmd_edit(char *s)
{
    char str[80];
    int color = WHITE;

    board->flags = 0;

    for(;;)
    {
	fgets(str, 80, stdin);
	if (!strcmp(str,"#\n"))
	    clearboard();
	else if (!strcmp(str, ".\n"))
	    break;
	else if (!strcmp(str, "c\n"))
	    color=opp(color);
	else if (!strcmp(str, "C\n"))
	    color=opp(color);
	else
	{
	    int f, r;
	    chesspiece p;
	    f = str[1]-'a';
	    r = str[2]-'1';
	    p = lookup(str[0]);
	    if (color==WHITE) p &= ~(1);
	    else p |= 1;
	    setpiece(f,r,p);
	}
    }

    countmaterial();

    /* set up castling rights */
    if ((board->kings[WHITE]==E1)&&(getpiece__(H1)==WROOK))
	board->flags |= wkc;
    if ((board->kings[WHITE]==E1)&&(getpiece__(A1)==WROOK))
	board->flags |= wqc;
    if ((board->kings[BLACK]==E8)&&(getpiece__(H8)==BROOK))
	board->flags |= bkc;
    if ((board->kings[BLACK]==E8)&&(getpiece__(A8)==BROOK))
	board->flags |= bqc;

    compute_hash();
}

void cmd_level(char *s)
{
    int movesPerSession, base, inc;
    
    sscanf(s, "%d %d %d", &movesPerSession, &base, &inc);
    chessclock[1] = chessclock[0] = base*60*1000;
    clockinc = inc*1000;

    searchdepth = 100;
    endgamedepth = 100;
}

void cmd_type(char *s)
{
/*      static char *gametypes[]={"blitz","standard","wild","suicide","crazyhouse","bughouse"}; */
}

void cmd_threats(char *s)
{
    int count=0;

    move_and_score *restore_sp = move_sp;

    board->flags ^= turn_flag;
    genmoves();
    board->flags ^= turn_flag;
    
    while (move_sp>restore_sp)
    {
	square end;
	int ef, er;

	move m = popmove();
	end = TO(m);
	ef = F(end);
	er = R(end);

	if (getpiece__(end))
	{
	    square start;
	    int sf, sr;

	    chesspiece p = getpiece(ef, er);
	    start = FR(m);
	    sf = F(start);
	    sr = R(start);
	    
	    printf("\t%c threatened on %c%c from %c%c\n",
		   rep(p), ef+'a',er+'1', sf+'a',sr+'1');
	    count++;
	}
    }
    if (count==0)
	printf("\t no pieces currently threatened\n");
}

void cmd_result(char *s)
{
    char results[10], comment[80];
    sscanf(s, " %s %s", results, comment);
    printf("read result '%s', comment '%s'\n", results, comment);
    if (book_mode)
    {
#ifdef NOT	
	if (!strcmp(results, "1/2-1/2"))
	{
	    hardupdatebook(-1, bookfile);
	}
	else if (!strcmp(results,"1-0"))
	{
	    hardupdatebook(WHITE, bookfile);
	}
	else if (!strcmp(results, "0-1"))
	{
	    hardupdatebook(BLACK, bookfile);
	}
#endif	
    }
    cmd_new(NULL);
}

void cmd_import(char *s)
{
}

void cmd_end(char *s)
{
    end(-2, "game ended");
    output("ending according to user\n");
    cmd_new(NULL);
}

void cmd_debug(char *s)
{
    freopen("debug.bce", "w+", stdout);
    printboard();
    cmd_inspect(NULL);
    cmd_set(NULL);
    exit(EXIT_FAILURE);
}

void cmd_save_binary_book(char *s)
{
    s++;
    if (savebook_binary(s))
	printf("book saved as %s (binary)\n", s);
    else
	printf("Error saving %s (binary)\n", s);
}

void cmd_load_binary_book(char *s)
{
    s++;
    if (loadbook_binary(s))
	printf("book loaded from %s (binary)\n", s);
    else
	printf("Error loading %s (binary)\n", s);
}

void cmd_protover(char *s)
{
   printf("%s\n", FEATURES);
}

struct cmd commands[] =
{
    { "attacks",cmd_attacks,"show available attacks" },
    { "backup", cmd_backup,"take back moves" },
    { "big", cmd_big, "use big board" },
    { "black", cmd_black, "xboard/RoboFICS" },
    { "blind", cmd_blind, "don't print board"},
    { "board", cmd_board, "show board setup" },
    { "book", cmd_book, "show book information" },
    { "computer", cmd_ignore, "xboard/RoboFICS"},
    { "debug", cmd_debug, "print debug info to file and die"},
    { "demo", cmd_demo, "play computer vs computer" },
    { "draw",  cmd_draw,    "offer a draw" },
    { "easy", cmd_easy,   "xboard/RoboFICS" },
    { "edit", cmd_edit, "edit the board (RoboFICS)" },
    { "end", cmd_end, "end the game (ROBOFICS)" },
    { "eval", cmd_eval, "show evaluation info" },
    { "exit",	cmd_exit,	"exit" },
    { "explain", cmd_explain, "fics stuff" },
    { "fen" , cmd_fen, "load fen file" },
    { "force", cmd_force, "force moves to setup board (RoboFICS)" },
    { "game",	cmd_game,	"show moves in current game" },    
    { "go",    cmd_go, "set BCE in motion (RoboFICS)" },
    { "hash", cmd_hash, "view positions and current hash" },
    { "hard",  cmd_hard,  "xboard/RoboFICS" },
    { "help", cmd_help, "show available commands" },
    { "hint", cmd_suggest,  "xboard/RoboFICS" },
    { "import", cmd_import, "load a game from FICS" },
    { "info", cmd_info, "program information" },
    { "inspect",cmd_inspect,	"show flags, etc"},
    { "killweights", cmd_default_weights, "load default weights" },
    { "kings", cmd_kings, "show location of kings" },
    { "lbbook", cmd_load_binary_book, "load book in binary form" },
    { "level", cmd_level,   "xboard/RoboFICS" }, 
    { "load", cmd_load,		"load board/flags"},
    { "moves",	cmd_moves,	"show available moves" },
    { "name",  cmd_name,    "xboard/RoboFICS" },
    { "new",	cmd_new,	"reset board, play a new game" },
    { "noexplain", cmd_noexplain, "fics stuff" },
    { "nopost", cmd_nopost, "don't show thinking" },
    { "otim", cmd_otim, "set oppenents clock (RoboFICS)" },
    { "pawns", cmd_pawns, "show pawn information" },
    { "post", cmd_post, "show thinking" },
    { "quit",	cmd_exit,	"quit" },
    { "random", cmd_random, "toggle random" },
    { "redraw", cmd_redraw, "redraw the board" },
    { "remove", cmd_backup, "xboard/RoboFICS" },
    { "reset",	cmd_new,	"reset board, play a new game" },
    { "result", cmd_result, "xboard/RoboFICS" },
    { "robofics", cmd_robofics, "RoboFICS"},
    { "icsdrone", cmd_robofics, "RoboFICS"},
    { "ics", cmd_robofics, "RoboFICS"},        
    { "save", cmd_save,		"save board/flags" },
    { "sbbook", cmd_save_binary_book, "save book as binary file" },
    { "sd",   cmd_sd, "set searchdepth" },
    { "set", cmd_set, "set variables" },
    { "small", cmd_small, "use small board" },
    { "suggest", cmd_suggest, "suggestion from computer" },
    { "threats", cmd_threats, "show threatened pieces/squares" },
    { "time", cmd_time, "set clock (RoboFICS)" },
    { "type",  cmd_type,    "xboard/RoboFICS" },
    { "undo", cmd_undo, "take back 1 move" },
    { "white", cmd_white,  "xboard/RoboFICS" },
    { "xboard", cmd_xboard, "xboard/RoboFICS" },
    { "protover", cmd_protover, "icsDrone" },
    {    NULL, NULL, NULL }
};

/* functions for reading moves follow */

/* remove +-x from string and read promotion information */
void strip(char *s)
{
    int i, j;
    size_t n = strlen(s) + 1;
    char *newstring = malloc(n);
    memset(newstring, 0, n);

    j=0;
    for (i=0;i<n;i++)
    {
	switch(s[i])
	{
	    case '=':
		promopiece = chesspiecevalue(lookup(s[++i]));
		break;
	    case '+':
	    case '-':
	    case 'x':
	    case ' ':
		break;
	    default:
		newstring[j] = s[i];
		j++;
		break;
	}
    }
    memcpy(s,newstring,n);
    free(newstring);
}

square findstart(chesspiece p, int f, int r, int gf, int gr)
{
    square s1 = 0;
    int count = 0;
    
    move_and_score *restore_sp = move_sp;
    genmoves();
    
    while(move_sp>restore_sp)
    {
	square start, end;
	int ef,er,sf,sr;

	move m = popmove();
	start = FR(m);
	end = TO(m);
	
	ef = F(end);
	er = R(end);
	sf = F(start);
	sr = R(start);

	if (p==getpiece(sf, sr))
	    if ((ef==f)&&((gf==sf)||(gf==8)))
		if ((er==r)&&((gr==sr)||(gr==8)))
		{
		    count++;
		    s1 = SQ(sf,sr);
		}
    }

    if (count>1)
    {
	printf("Illegal move (ambiguous)\n");
	return dummysq;
    }
    else if (count == 1)
    {
	return s1;
    }
    else
	return dummysq;
}

/*
ex. cd (cxd)
*/
move decode_pawn_cap(char *s)
{
    static char *c_index = "abcdefgh";
    int r, sf, ef;

    sf = strchr(c_index, s[0]) - c_index;
    ef = strchr(c_index, s[1]) - c_index;

    for (r=0;r<7;r++)
    {
	chesspiece p = getpiece(sf,r);
	if ((COLOR(p) == tomove()) &&
	    (VALUE(p) == pawn))
	{
	    chesspiece a = getpiece(ef, r+dir(COLOR(p)));
	    if (COLOR(a) != tomove())
		return MV(SQ(sf,r),SQ(ef,r+dir(COLOR(p))));
	}
    }

    return dummymove;
}

move easydecode(char *str)
{
    if (!strcmp(str,"o")||!strcmp(str,"o-o")||
	!strcmp(str,"0-0")||!strcmp("O-O",str))
    {
	return (tomove() == WHITE) ? WKC : BKC;
    }

    if (!strcmp(str,"o-")||!strcmp(str,"o-o-o")||
	!strcmp(str,"0-0-0")||!strcmp("O-O-O",str))
    {
	return (tomove() == WHITE) ? WQC : BQC;
    }

    strip(str);

    if (strlen(str)==5)
    {
	/* assume this is the form a7a8q */
	/* get promo and remove from string */
	chesspiece p = lookup(str[4]);
	promopiece = chesspiecevalue(p);

	if ((promopiece<=0)||(promopiece>king))
	    return dummymove;
	
	str[4]=0;
    }
    
    /* e4 */
    if (strlen(str)==2)
    {
	int ef, er;
	square s1, s2;

	ef = str[0]-'a';
	er = str[1]-'1';

	if (strchr("abcdefgh", str[0]) &&
	    strchr("abcdefgh", str[1]))
	{
	    return decode_pawn_cap(str);
	}
	
	if (offboardp(ef, er))
	    return dummymove;
	
	s2 = SQ(ef,er);
	s1 = findstart(makepiece(tomove(), pawn), ef, er, 8, 8);
	return MV(s1,s2);
    }

    /* Pe4 ab4 */
    if (strlen(str)==3)
    {
	int sf, ef, er;
	square s1 = dummymove, s2;

	ef = str[1]-'a';
	er = str[2]-'1';

	if (offboardp(ef, er))
	    return dummymove;

	s2 = SQ(ef,er);
	if (str[0]=='b')
	{
	    /* if its a valid pawn move it,
	       else look for valid Bishop move
	    */
	    int v = chesspiecevalue(lookup(str[0]));
	    sf = str[0] - 'a';
	    s1 = findstart(makepiece(tomove(), pawn), ef, er, sf, 8);
	    if (s1!=dummysq)
	    {
		return MV(s1,s2);
	    }
	    else if (v)
	    {
		s1 = findstart(makepiece(tomove(), v), ef, er, 8, 8);
		return MV(s1,s2);
	    }
	    else
		return dummysq;
	}
	else if (lookup(str[0]))
	{
	    int v = chesspiecevalue(lookup(str[0]));
	    s1 = findstart(makepiece(tomove(), v), ef, er, 8, 8);
	}
	else
	{
	    sf = str[0] - 'a';
	    s1 = findstart(makepiece(tomove(), pawn), ef, er, sf, 8);
	}
	return (MV(s1,s2));
    }
    
    /* e2e4 Nbd7 N8d7*/
    else if (strlen(str)==4)
    {
	int sf, sr, ef, er, v;
	square s1, s2;
	
	sf = str[0]-'a';
	sr = str[1]-'1';
	ef = str[2]-'a';
	er = str[3]-'1';

	if (offboardp(ef, er))
	    return dummymove;

	if (!offboardp(sf, sr))
	    return MV(SQ(sf,sr),SQ(ef,er));
	
	s2 = SQ(ef, er);
	s1 = SQ(sf, sr);
	
	switch (str[0])
	{
	    case 'P': case 'p':
	    case 'N': case 'n':
	    case 'B': case 'b':
	    case 'R': case 'r':
	    case 'Q': case 'q':
	    case 'K': case 'k':
		sf = str[1]-'a';
		sr = str[1]-'1';
		v = chesspiecevalue(lookup(str[0]));
		if ((sf>=0)&&(sf<=7))
		{
		    s1 = findstart(makepiece(tomove(), v), ef, er, sf, 8);
		    return MV(s1, s2);
		}
		else
		{
		    s1 = findstart(makepiece(tomove(), v), ef, er, 8, sr);
		    return MV(s1, s2);
		}
		
	    default:
		return dummymove;
	}
    }
    else
	return dummymove;
}

int validmove(move m)
{
    move_and_score *restore_sp = move_sp;

    genmoves();

    while(move_sp>restore_sp)
    {
	move vm = popmove();
	if (m == vm)
	{
	    int r = wouldbeincheckfullp(m);
	    
	    if (r == 0)
	    {
		move_sp = restore_sp;
		return 1;	/* valid */
	    }
	    else
	    {
		move_sp = restore_sp;
		return -r;	/* invalid move */
	    }
	}
    }
    return 0;			/* move was not found */
}

move command_or_move(char *s)
{
    int i;

    if (strlen(s)==0)
    {
	return 0;
    }
    for (i=0; commands[i].name; i++)
    {
	int len = strlen(commands[i].name);
	if (!strncasecmp(s, commands[i].name, len))
	{
	    commands[i].func(s+len);
	    return 0;
	}
    }

    return easydecode(s);
}

move usermove()
{
    char str[80],*p;
    move m;
    
    memset(str, 0, sizeof(str[0]));
    
    fgets(str, sizeof(str), stdin);
    
    if((p = strchr(str, '\n')) != NULL)
    {
	*p = '\0';
    }
    
    if (xboard_mode)
    {
	printf("received - '%s'\n", str);
    }
    
    m = command_or_move(str);
    if (!m)
    {
	return 0;
    }
    else
    {
	switch(validmove(m))
	{
	    case 1:		/* valid move */
		return m;
	    case 0:
		printf("Error (unknown command - try 'help'): %s\n", str);
		return 0;
	    default:
		printf("Illegal move (cannot move into check)\n");
		return 0;
	}
    }
}
