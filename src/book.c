/*
  This File is part of Boo's Chess Engine
  Copyright 2000 by Christopher Bowron
*/  
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#include "bce.h"

int index_count;
int binary_book = 0;

move bookopening()
{
    int i;
    
    int total = 0;

/*      printf("bookopening (%d)\n", index_count); */
/*      printf("%lld, %d, %s\n", mem.book[0].hash, */
/*  	   mem.book[0].count, movestring(mem.book[0].m)); */
    
    for (i=0;i<index_count;i++)
    {
	if (mem.book[i].hash == board->hash)
	{
	    int j;
	    
	    for (j = i; ; j++)
	    {
		if (mem.book[j].hash != board->hash)
		    break;
		total += mem.book[j].count;
	    }

/*  	    printf("%d possible moves, %d variations\n", */
/*  		   j-i, total); */
	    
	    if (j == i+1) /* only one opening */
	    {
		return mem.book[i].m;
	    }
	    else		/* pick random opening  */
	    {
		int n = getrandomnumber() % total;
/*  		printf("using variant %d\n", n); */
		
		for (j=i;;j++)
		{
		    n -= mem.book[j].count;
		    if (n<=0)
			return mem.book[j].m;
		}
	    }
	}

	if (mem.book[i].count == 0)
	    return dummymove;
    }
    return dummymove;
}

int process_opening(char *str)
{
    int c;
    char *movestr;
    int movecount;

    setupboard();
    ply = 0;
    
    if (str==NULL)
	return 0;

    for(movecount=0,c=WHITE, movestr=strtok(str, " \n\t");;
	movecount++,c=opp(c),movestr=strtok(NULL, " \n\t"))
    {
	move m;
	int i;
	
	if (movestr==NULL)
	    break;

	m = easydecode(movestr);
	
	if (offboardp(F(TO(m)),R(TO(m)))||
	    offboardp(F(FR(m)),R(FR(m))))
	{
/*  	    printboard(); */
/*  	    printf("m = %s\tmovestr = %s\n", movestring(m), movestr); */
	    return 0;
	}
	
	for (i=0;i<TABLE_SIZE;i++)
	{
	    if ((mem.book[i].hash == board->hash)&&
		(mem.book[i].m == m))
	    {
		mem.book[i].count++;
		break;
	    }
	    if (mem.book[i].hash == 0)
	    {
		mem.book[i].hash = board->hash;
		mem.book[i].m = m;
		mem.book[i].count = 1;
		
		index_count++;
		break;
	    }
	}
		
	domove(m);
    }
    return 1;
}

int bk_cmp(const void *a, const void *b)
{
    const book_table
	*bt_a = a,
	*bt_b = b;
    return (bt_a -> hash < bt_b -> hash);
}


void loadbook_text(char *name)
{
    char str[1024];
    FILE *in = fopen(name, "r");
    int count = 0;
    int line_index = 0;
    
    if (!in)
    {
	printf("loadbook(): error opening book: %s\n", name);
	return;
    }

    printf("Loading Book\n");

    clear_hash();
    index_count = 0;
    
    while (!feof(in))
    {
	fgets(str, 256, in);
	line_index ++;
	
	if (str[0]=='#')
	    continue;
	if (str[0]=='[')
	    continue;
	
	if (!process_opening(str))
	    printf("Error line %d of %s\n", line_index, bookfile);
	else
	    count ++;
    }
    fclose(in);

    qsort(mem.book, index_count, sizeof(mem.book[0]), bk_cmp);
    
    printf("Book Loaded (%s: %d openings, %d unique positions)\n",
	   name, count, index_count);
    
    setupboard();
}

int savebook_binary(char *file)
{
    FILE *bin;
    size_t item_size = sizeof(mem.book[0]);
    size_t items_saved;
    
    bin = fopen (file, "w+");
    if (!bin)
    {
	fprintf(stderr, "Error opening binary book %s\n", file);
	return 0;
    }

    items_saved = fwrite(mem.book, item_size, index_count, bin);
    fclose(bin);

    if (index_count != items_saved)
	return 0;
    return 1;
}

int loadbook_binary(char *file)
{
    FILE *bin;
    size_t item_size = sizeof(mem.book[0]);
    size_t items;
    
    bin = fopen (file, "r");
    if (bin == 0)
    {
	fprintf(stderr, "Error opening binary book: %s\n", file);
	return 0;
    }
    
    fseek(bin, 0, SEEK_END);
    
    items = ftell(bin) / item_size;
    fseek(bin, 0, SEEK_SET);
    
    clear_hash();
    
    index_count = fread(mem.book, item_size, items, bin);
    fclose(bin);
    
    printf("%d unique positions loaded (%s)\n", index_count, file);

    return 1;
}

void loadbook(char *s)
{
    if (binary_book)
    {
	loadbook_binary(s);
    }
    else
    {
	loadbook_text(s);
    }
}

void hardupdatebook(int winner, char *name)
{
    return ;
}
