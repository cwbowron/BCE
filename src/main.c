/*
  This File is part of Boo's Chess Engine
  Copyright 2000 by Christopher Bowron

  This program or any derivative program cannot be distributed for
  commercial purposes without the consent of Christopher Bowron
*/  
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "bce.h"

long seed = 0;
int seed_with_time = 1;

/* if we get interrupt in demo mode, turn white off */
void myhandler(int interrupt)
{
  if (!robo_mode && !xboard_mode) {
    computer[1] = computer[0] = 0;
  }
  
  signal(SIGINT, myhandler);
}

void showmoves(c)
{
    move_and_score *restore_sp = move_sp;
    
    genmoves(c);
    
    while (move_sp>restore_sp)
    {
	move m;
	m = popmove();
	domove(m);
	printboard();
	undomove();
    }
}

char *formattime(int clock)
{
    static char s[80];
    sprintf(s, "%2d:%02d.%02d",
	    clock / (60*1000),
	    (clock%(60*1000))/1000,
	    (clock%(60*1000)%1000)/100);
    return s;
}

int end(int winner, char *reason)
{
/*      char *foo[] = {"1/2-1/2", "1-0", "0-1" }; */

    if (weight_mode)
    {
	update_weights(winner);
	save_weights(weightfile);
    }

/*      if ((winner>=-1)&&(winner<=1)) */
/*      { */
/*  	output("%s {%s}\n", foo[winner+1],reason); */
/*      } */
/*      else */
/*      { */
/*  	output("%s {%s}\n", "ended", reason); */
/*      } */
/*      if (((computer[WHITE]+computer[BLACK])>0)&&book_mode) */
/*      { */
/*  	hardupdatebook(WHITE, bookfile); */
/*      } */
    
    return winner;
}

void update_state(move m)
{
}

void printboard_and_time()
{
    int full, part;
    
    printboard();
    
    printf("W %s\t", formattime(chessclock[WHITE]));
    printf("B %s\n", formattime(chessclock[BLACK]));

    full = fullincheckp(tomove());
    part = ply ? incheckp(tomove()) : 0;
    
    if (full)
	printf("In Check : %d\n", full);

    if ((part &&  !full)||(full && !part))
    {
	printf("partial checktest (%d) and full checktest (%d) do no agree\n",
	       part, full);
    }
    
    printf("%c To Move\n", (tomove()==WHITE) ? 'W' : 'B');
}


/* returns -1 for stalemate or winner's color */
int playchess()
{
    int use_pondering = 0;
    
    printboard_and_time();
     
    for (;;)
    {
	long starttime, endtime;
	move m;
	int g;

	g = gameoverp(tomove());

	if (g)
	{
	    switch (g)
	    {
		case END_CHECKMATE:
		    if (tomove() == BLACK)
		    {
			return end(WHITE, "white mates");
		    }
		    else
		    {
			return end(BLACK, "black mates");
		    }
		case END_STALEMATE:
		    return end(-1, "stalemate");
		case NON_MATERIAL:
		    return end(-1, "insufficient material");
		case REP_DRAW:
		    if (!robo_mode)
		    {
			printf("drawable position\n");
		    }
		    if (computer[WHITE]||computer[BLACK])
		    {
			if (robo_mode)
			    tellics("draw\n");
			return end(-1, "draw by repetition of moves");
		    }
		    break;
	    }
	}
    
	starttime = get_ms();
	
	if (computer[tomove()])
	{
	    m = bce();

	    if ((m!=dummymove)&&(validmove(m)==1))
	    {
		printf("move %s\n", movestring(m));
	    }
	    else
	    {
		if (robo_mode)
		{
		   /*  tellics("mailmoves\n"); */
		   /*  tellics( */
			/* "message madhacker valid? = %d, move = %s, wouldbeincheckp() = %d, wouldbeinfullcheckp() = %d, pv = %s\n", */
			/* validmove(m), */
			/* movestring(m), */
			/* wouldbeincheckp(m), */
			/* wouldbeincheckfullp(m), */
			/* thoughts); */
		    tellics("abort\n");
		}
		else
		{
		    printf("BCE returned invalid move: %s\n", movestring(m));
		    printf("valid? = %d\n", validmove(m));

		    fprintf(stdout, "random seed = %ld\n", seed);
		    fprintf(stdout, "hash = %lld\n", board->hash);
		    fprintf(stdout, "draw? = %d g = %d\n",
			    draw_by_rep(), g);
		    computer[1] = computer[0] = 0;
		}
	    }
	    use_pondering = 1;
	}
	else 
	{
	    if ((ponder_mode && computer[opp(tomove())])&&
		use_pondering)
	    {
		ponder();
		use_pondering = 0;
	    }
	    
	    m = usermove();
	    use_pondering = 0;
	}
    
	endtime = get_ms();
	chessclock[tomove()] -= (endtime-starttime);
	chessclock[tomove()] += clockinc;
    
	if (m)
	{
	    domove(m);
	    update_state(m);
	    printboard_and_time();
	}
    }
}

void loadrc()
{
    FILE *resource = fopen(".bcerc", "r");

    if (resource)
    {
	char s[80], *p;

	while(fgets(s, 80, resource))
	{
	    if((p = strchr(s, '\n')) != NULL)
		*p = '\0';
	    
	    if (command_or_move(s))
		printf("unknown command in resource file: %s\n", s);
	}
    }
}

void initialize()
{
    move_sp = move_stack;
    chessclock[0]=chessclock[1]=10*60*1000;
    clockinc = 0;

    branch_info.average = 0.0;
    branch_info.count = 0;

    if (book_mode)
    {
	change_gamemode(OPENING, 0);
	loadbook(bookfile);
    }
    else
    {
    	change_gamemode(MIDGAME, 1);
    }
    
    setupboard();

    ponder_mode = 0;
    
    init_learning();
    
    ply = 0;
    searchdepth = midgamedepth;

    computer[WHITE]=0;
    computer[BLACK]=searchdepth;
    
    loadrc();
}



void exit_engine(void)
{
    if (weight_mode)
    {
	save_weights(weightfile);
    }
    if (board)
    {
	free(board);
    }
    credits();
}

void setupfilenames()
{
    char *etcbook = "/etc/book.txt";
    char *etcweight = "/etc/bce.weight";

    char *bcehome = getenv("BCEHOME");
    if (bcehome) {
	printf("BCEHOME found = %s\n", bcehome);
	
	bookfile = malloc(strlen(bcehome)+strlen(etcbook)+1);
	weightfile = malloc(strlen(bcehome)+strlen(etcweight)+1);
	strcpy(bookfile, bcehome);
	strcpy(weightfile, bcehome);
	
	strcat(bookfile, etcbook);
	strcat(weightfile, etcweight);
    }
    else {
	bookfile   = "book.txt";
	weightfile = "bce.weights";
    }
}

int main(int argc, char**argv)
{
  char *savedgame = NULL;
  int usesignal = 1;
  int i;

  setupfilenames();

  for (i=1;i<argc;i++)
  {
      if (!strcmp(argv[i], "-load"))
	  savedgame = argv[++i];
      else if (!strcmp(argv[i], "-xsignal"))
	  usesignal = 0;
      else if (!strcmp(argv[i], "-book"))
	  bookfile = argv[++i];
      else if (!strcmp(argv[i], "-binbook"))
      {
	  binary_book = 1;
	  bookfile = "book.bin";
      }
      else if (!strcmp(argv[i], "-weights"))
	  weightfile = argv[++i];
      else if (!strcmp(argv[i], "-xbook"))
	  book_mode = 0;
      else if (!strcmp(argv[i], "-xweights"))
	  weight_mode = 0;
      else if (!strcmp(argv[i], "-seed"))
      {
	  seed = atol(argv[++i]);
	  seed_with_time = 0;
      }
      else
	  printf("unknown command line switch: %s\n", argv[i]);
  }

  printf("Book: %s\n", bookfile);
  printf("Weight: %s\n", weightfile);

  if (usesignal)
      signal(SIGINT, myhandler);

  atexit(exit_engine);

  /* turn off buffering */
  setbuf(stdout, NULL);		
  setbuf(stderr, NULL);

  if (!weight_mode||!load_weights(weightfile))
  {
      if (weight_mode)
	  fprintf(stderr,"main: could not load weights, using defaults\n");
      else
	  printf("ignoreing weight file, using defaults\n");
      
      setup_default_weights();
  }

  board = malloc(sizeof(chessboard));

  /* default seed for binary book */
  seed_rng(DEFAULT_RANDOM_SEED);         
  compute_randoms();

  /* randomize for random book moves */
  seed_randoms();		

  for (;;)
  {
      if (!savedgame)
	  initialize();
      else 
	  cmd_load(savedgame-1);
      playchess();
  }
  return 0;
}

