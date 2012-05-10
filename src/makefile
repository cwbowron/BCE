PKGNAME = BCE
VERSION = 1.8
HFILES = bce.h
CFILES = board.c types.c main.c movegen.c domove.c user.c search.c \
	vars.c book.c checktest.c eval.c hash.c branch.c learn.c \
	compileinfo.c random.c util.c
OFILES = ${CFILES:.c=.o}
CC = gcc
# CFLAGS=-Wall -O3 -finline-functions -funroll-loops
CFLAGS = -Wall -O3 -s -finline-functions -funroll-loops
PFLAGS = -g -pg
DFLAGS = -g
default:
	@touch compileinfo.c
	@make bce

clients: $(BCEHOME)/bin/BACE $(BCEHOME)/bin/ddlchess

bce:	$(OFILES) $(HFILES)
	$(CC) -o bce $(CFLAGS) $(OFILES) -lm

$(BCEHOME)/bin/BACE:	$(OFILES) $(HFILES)
	$(CC) -DLEARNING $(CFLAGS) -c compileinfo.c
	$(CC) -DLEARNING $(CFLAGS) -c learn.c
	$(CC) -o $(BCEHOME)/bin/BACE $(CFLAGS) $(OFILES) -lm

$(BCEHOME)/bin/ddlchess: $(OFILES) $(HFILES)
	$(CC) -ULEARNING $(CFLAGS) -c compileinfo.c
	$(CC) -ULEARNING $(CFLAGS) -c learn.c
	$(CC) -o $(BCEHOME)/bin/ddlchess $(CFLAGS) $(OFILES) -lm
fresh:	
	@touch *.c
	@make bce

pbce:	$(OFILES)
	$(CC) -o pbce $(PFLAGS) $(CFILES) -lm


dbce:	$(OFILES)
	$(CC) -o dbce $(DFLAGS) $(CFILES) -lm

clean:
	rm bce $(BCEHOME)/bin/ddlchess $(BCEHOME)/bin/BACE
	rm *.o

eval.o: eval.c evaluator.inc.c
	$(CC) $(CFLAGS) -c -o eval.o eval.c

.o:	$(HFILES)
	$(CC) $(CFLAGS) -c $<

%.s:	%.c $(HFILES)
	$(CC) $(CFLAGS) -S $*.c
