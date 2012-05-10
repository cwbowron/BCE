#include "bce.h"

int board64[64];

int board88[8][8];

chesspiece getboard64(int f, int r)
{
    return board64[(f<<3)|r];
}

chesspiece getboard88(int f, int r)
{
    return board88[f][r];
}

chesspiece curgetboard(int f, int r)
{
    return board[f+4][r+4];
}
