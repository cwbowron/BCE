/*
  This File is part of Boo's Chess Engine
  Copyright 2000 by Christopher Bowron
*/

#include <setjmp.h>
#include <limits.h>

#ifndef BCE_H
#define BCE_H

/*
+----
| Simple defined constants
+----
*/
#define DEFAULT_RANDOM_SEED 19790130
#define WHITE (0)
#define BLACK (1)

#define STATIC_PAWN_VALUE 1000

#define WIN       (100 * STATIC_PAWN_VALUE)
#define LOSE      (-WIN)
#define DRAW	  0
#define STALEMATE 0
#define INF	  (INT_MAX - 100)

#define wqc 1
#define wkc 2
#define bqc 4
#define bkc 8
#define enp 16
#define dbl 32
#define cas 64
#define wkcastled 128
#define wqcastled 256
#define bkcastled 512
#define bqcastled 1024
#define promo_flag 2048
#define turn_flag  4096

#define EMPTY_VALUE	0
#define PAWN_VALUE	1
#define KNIGHT_VALUE	2
#define BISHOP_VALUE	3
#define ROOK_VALUE	4
#define QUEEN_VALUE	5
#define KING_VALUE	6
#define KCASTLEBONUS	(KING_VALUE+1)
#define QCASTLEBONUS	(KCASTLEBONUS+1)
#define NOCASTLEQUEEN	(QCASTLEBONUS+1)
#define NOCASTLEKING	(NOCASTLEQUEEN+1)
#define QUEEN_TROPISM	(NOCASTLEKING+1)
#define ROOK_TROPISM	(QUEEN_TROPISM+1)
#define DOUBLEDROOKS	(ROOK_TROPISM+1)
#define OPENFILE	(DOUBLEDROOKS+1)
#define SEMIOPEN	(OPENFILE+1)
#define TWOBISHOPS	(SEMIOPEN+1)
#define KNIGHT_TROPISM	(TWOBISHOPS+1)
#define ISOLATED	(KNIGHT_TROPISM+1)
#define DOUBLED		(ISOLATED+1)
#define BACKEDUP	(DOUBLED+1)
#define NOMATERIAL	(BACKEDUP+1)
#define SEVENTH_RANK_ROOK	(NOMATERIAL+1)
#define WHITE_PAWN_POS  (SEVENTH_RANK_ROOK+1)
#define BLACK_PAWN_POS	(WHITE_PAWN_POS+64)
#define KNIGHT_POS	(BLACK_PAWN_POS+64)
#define BISHOP_POS	(KNIGHT_POS+64)
#define ROOK_POS	(BISHOP_POS+64)
#define QUEEN_POS	(ROOK_POS+64)
#define KING_POS	(QUEEN_POS+64)
#define PASSED_PAWN	(KING_POS+64)
#define LOCAL_LAST_WEIGHT	(PASSED_PAWN+8)
#define END_GAME_WEIGHTS	(LOCAL_LAST_WEIGHT)
#define LAST_WEIGHT	(LOCAL_LAST_WEIGHT*2)

#define tomove()	((board->flags&turn_flag)>>12)

#define switch_sides()  (board->flags ^= turn_flag)

#define PV_LENGTH	100

#define TABLE_SIZE	 (1024*1024)

#define MAXMOVES	512

/*
+----
| Enumerations
+----
*/

enum { RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8 };
enum { FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH };

enum {
    A1, A2, A3, A4, A5, A6, A7, A8,
    B1, B2, B3, B4, B5, B6, B7, B8,
    C1, C2, C3, C4, C5, C6, C7, C8,
    D1, D2, D3, D4, D5, D6, D7, D8,
    E1, E2, E3, E4, E5, E6, E7, E8,
    F1, F2, F3, F4, F5, F6, F7, F8,
    G1, G2, G3, G4, G5, G6, G7, G8,
    H1, H2, H3, H4, H5, H6, H7, H8
};

enum { empty, pawn, knight, bishop, rook, queen, king };

enum {
    WPAWN = 2,
    BPAWN = 3,
    WKNIGHT = 4,
    BKNIGHT = 5,
    WBISHOP = 6,
    BBISHOP = 7,
    WROOK = 8,
    BROOK = 9,
    WQUEEN = 10,
    BQUEEN = 11,
    WKING = 12,
    BKING = 13
}; 

enum {
    REP_DRAW = -3,
    NON_MATERIAL = -2,
    END_STALEMATE = -1,
    IN_PROGRESS = 0,
    END_CHECKMATE = 1
};

/* game modes */
enum { OPENING, MIDGAME, ENDGAME };

/* hash hits */
enum { NO_HIT, LOWER_DEPTH, U_BOUND, EXACT, L_BOUND };


/*
+----
| Simple type definitions
+----
*/

typedef int chesspiece;		/* at least 4 bits */
typedef unsigned int square;	/* at least 6 bits */
typedef unsigned int move;	/* at least 12 bits */
typedef unsigned long long bitboard; /* 64 bits */

#define HASH_64
#ifdef HASH_64
typedef unsigned long long hashtype;
#else
typedef unsigned long int hashtype;
#endif

/*
+----
| Structures / Complex type definitions
+----
*/

typedef struct {
    move m;
    int score;
} move_and_score;

typedef struct
{
    chesspiece squares[64];
    int flags;
    hashtype hash;
    int pawns[2][10];
    int material[2];
    int position[2];
    int piececount[2];   
    int pieces[BKING+1];
    square kings[2];
    bitboard pawnbits[2];
} chessboard;

typedef struct
{
    move m;
    chesspiece captured;
    chesspiece moved;
    int flags;
} undo;

typedef struct 
{
    hashtype hash;
    int depth;
    int flag;
    int score;
    move m;
} ttable;

typedef struct
{
    hashtype hash;
    move m;
    int count;
} book_table;

typedef struct {
    int length[PV_LENGTH];
    move moves[PV_LENGTH];
    int hits;
} pv_storage;

typedef struct {
    jmp_buf restore_stack;	/* local exit for when our time expires */
    pv_storage pv;		
    int startply;
    int polling;
    long stoptime;		
    int branches;		/* nodes examined, non-quiesce */
    int nodes;			/* nodes examined */
    int stop;			/* stop thinking? */
    int move_depth[10];
} search_info_type;

typedef struct {
    int count;
    double average;
} branch_info_type;

typedef union {
    ttable tt[TABLE_SIZE];
    book_table book[TABLE_SIZE];
} book_and_table;

/*
+----
| Macros
+----
*/

#define makepiece(c, v)    (((v)<<1)|(c))
#define chesspiecevalue(p) ((p)>>1)
#define chesspiececolor(p) ((p)&1)

#define PIECE(c,v)	(((v)<<1)|(c))
#define VALUE(piece)	((piece)>>1)
#define COLOR(piece)	((piece)&1)

#define dir(color)	(((color)==WHITE) ? 1 : -1)
#define opp(color)      ((color)^1)

#define I(f,r)             (((f)<<3)|(r))
#define getpiece(f,r)      (board->squares[I(f,r)])
#define setpiece(f,r,p)    board->squares[I(f,r)]=(p)
#define getpiece__(i)      (board->squares[(i)])
#define setpiece__(i,p)    board->squares[(i)]=(p)

#define offboardp(f, r)   (((f)&0xf8)|((r)&0xf8))
#define offboardp__(i)    (i&~(077))
			   
#define emptyp(f, r)      (!getpiece((f),(r)))
#define oppenentp(f,r,c)  (getpiece((f),(r))&&(chesspiececolor(getpiece((f),(r)))^(c)))

#define SQ(f,r)          (((f)<<3)|(r))
#define MV(s,e)          (((s)<<6)|(e))
#define F(s)		 ((s)>>3)
#define R(s)             ((s)&07)
#define TO(m)            ((m)&077)
#define FR(m)            ((m)>>6)

#define FL(f) "abcdefghx"[f]
#define RK(r) "12345678x"[r]

#define signum(i)	((i>0) ? 1 : -1)
#define min(a,b)	((a)<(b)?(a):(b))
#define max(a,b)	((a)>(b)?(a):(b))

#define hash_index(h)	(h&(TABLE_SIZE-1))
#define lastmove()	(gamestack[ply-1])

#define dummysq    (SQ(8,8))
#define dummymove  (MV(dummysq, dummysq))

#define WQC MV(E1, C1)
#define WKC MV(E1, G1)
#define BQC MV(E8, C8)
#define BKC MV(E8, G8)


#define makevariables() (wqc|wkc|bqc|bkc)
#define wqcastlep()     ((board->flags)&wqc)
#define wkcastlep()     ((board->flags)&wkc)
#define bqcastlep()     ((board->flags)&bqc)
#define bkcastlep()     ((board->flags)&bkc)
#define enpassantedp()  ((board->flags)&enp)
#define doublepushp()   ((board->flags)&dbl)
#define castledp()      ((board->flags)&cas)
#define wkcastledp()    ((board->flags)&wkcastled)
#define wqcastledp()    ((board->flags)&wqcastled)
#define bkcastledp()    ((board->flags)&bkcastled)
#define bqcastledp()    ((board->flags)&bqcastled)
#define promop()	((board->flags)&promo_flag)

#define BIT(f,r)	((bitboard) 1<<((r)|((f)<<3)))
#define BIT__(i)	((bitboard) 1<<(i))

#define tables (mem.tt)

/*
+----
| variable declarations
+----
*/

extern int fake_count;

extern hashtype randoms[BKING+1][64];
extern book_and_table mem;

extern int highlight;
extern int searchdepth;
extern int endgamedepth;
extern int midgamedepth;
extern long int searchtime;

extern int promopiece;
extern int clockinc;

extern int xboard_mode;
extern int robo_mode;
extern int ponder_mode;
extern int thinking_mode;
extern int explain_mode;
extern int book_mode;
extern int weight_mode;
extern int gamemode;
extern int ply;

extern char *bookfile;
extern char *weightfile;

extern int stop_pondering;

extern chessboard *board;

extern int computer[2];
extern long chessclock[2];

extern int binary_book;
extern char thoughts[512];

extern move_and_score move_stack[8096];
extern move_and_score *move_sp;

extern move gamestack[MAXMOVES];
extern undo undostack[MAXMOVES];
extern hashtype positions[MAXMOVES];

extern const int knightdx[8];
extern const int knightdy[8];

extern branch_info_type branch_info;

extern int *weights;
extern int *base_weights;

extern search_info_type search_info;
extern int whitepawnsquares[64];
extern int blackpawnsquares[64];
extern int knightsquares[64];
extern int bishopsquares[64];
extern int rooksquares[64];
extern int queensquares[64];
extern int kingsquares[64];

extern int tablehits[3];

extern move_and_score killers[PV_LENGTH][2];
extern int killerhits;

extern int search_count;
extern double branch_average;

extern void (*printboard)();
extern int (*search)(int,int,int,int);

extern int seed_with_time;
extern long seed;

/*
+----
| function declarations
+----
*/

void clearboard();
char rep(chesspiece);
void printboardbig();
void printboardsmall();
void printboardblind();
void setupboard();

void genmoves();
void genattacks();
void printbitboard(bitboard);

char *squarestring(square);
char *movestring(move);

void domove(move);
void undomove(void);

void fake_domove(void);
void fake_undomove(void);

void updatematerial(int, int, chesspiece, int);
void updatematerial__(int i, chesspiece p, int add);

void updatemoved(square, square, chesspiece);
chesspiece getpromo();

int  evaluate();
int  evaluate_verbose();
void countmaterial();
int  squarevalue(int, int, chesspiece);
int  squarevalue__(int i, chesspiece p);

move bce();
void sortmoves(move_and_score *, move_and_score *, move);

int  alphabeta_search(int,int,int);
int  alphabeta_tables(int,int,int);
int  negamax_search(int, int, int);
int  negascout_search(int,int,int);
int  negascout_tables(int,int,int,int);
int  hack_move(int,int,int);	
int  qsearch(int, int, int);

int  incheckp(int);
int  fullincheckp(int);
int  intocheckp(move, int);
int  gameoverp(int);

int  wouldbeincheckp(move);
int  wouldbeincheckfullp(move);

int  playchess();
void printmoves(int);

move bookopening();
void loadbook(char *);
void hardupdatebook(int, char*);

move usermove();
move easydecode(char *);

void cmd_game(char *);
void cmd_load(char *);
void cmd_reset(char *);
void cmd_board(char *);

int validmove(move);

void pushmove(move);
move popmove(void);

void ponder();
move think();

void initialize();
void compute_hash();

long get_ms();
void tell_owner(char *format, ...);
void whisper(char *format, ...);

move command_or_move(char *s);
int draw_by_rep();
int position_seen_before();

void output(char *format, ...);
void tellics(char *format, ...);

void store_hash(int depth, int flag, int score, move m);
int retrieve_hash(int depth, int *score, move *m);
void clear_hash();
void compute_randoms();
void compute_hash();

void store_killer(move, int p, int a, int b);
void storepv(move m, int p);

double branch_factor(int nodes, int depth);

int load_weights(char *);
void save_weights(char *);
void setup_default_weights();

void init_learning();
void save_leaf(move *, int, int, int);
void update_weights(int);

void store_board(int p);

void alloc_board(int p);

int end(int, char *);

void change_gamemode(int, int);
int actualmode();

int savebook_binary(char *file);
int loadbook_binary(char *file);

void credits();

void seed_randoms();
long getrandomnumber();
void seed_rng(long);

int bioskey();

#endif
