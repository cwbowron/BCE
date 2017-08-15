#! /bin/sh
SEEK="seek 1 0\nseek 2 0\nseek 5 0 w2\n"
./icsdrone -handle "$FICS_HANDLE" -password "$FICS_PASSWORD" -owner "$FICS_OWNER" -program "./bce" -sendLogin "$SEEK" -sendGameEnd "$SEEK" -resign off -acceptDraw off
