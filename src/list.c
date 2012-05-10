/*
  This File is part of Boo's Chess Engine
  Copyright 1999 by Christopher Bowron
*/  

#include <stdlib.h>
#include <stdio.h>
#include "bce.h"

list *alloclist()
{
    return malloc(sizeof(list));
}

void freelistitem(list *p)
{
    if (p->free)
	free(p->data.ptr);
    free(p);
}

void push (list **l, long m)
{
    list *new = alloclist();
    new->data.l = m;
    new->free = 0;
    new->next = *l;
    *l=new;
} 

long pop (list **l)
{
    long m;
    list *temp=*l;
    
    m = temp->data.l;
    *l = temp->next;
    freelistitem(temp);
    return m;
}

void push_ptr(list **l, void *ptr)
{
    list *new = alloclist();
    new->data.ptr = ptr;
    new->free = 1;
    new->next = *l;
    *l=new;
}

void *pop_ptr(list **l)
{
    void *p;
    
    list *temp=*l;
    p = temp->data.ptr;
    *l = temp->next;
    temp->free = 0;
    freelistitem(temp);
    return p;
}

list *copylist(list *l)
{
    list *n;
    
    if (!l) return NULL;
    n = malloc(sizeof(list));
    n->data = l->data;
    n->free = l->free; 
    n->next = copylist(l->next);
    return n;
}


int length(list *l)
{
    if (l == NULL) return 0;
    return length(l->next)+1;
}

long nth(list *l, int n)
{
    if (n==0)
	return l->data.l;
    return nth(l->next,n-1);
}

void killlist(list *l)
{
    list *t = l;

    if (!l)
	return;

    killlist(l->next);
    freelistitem(t);
}

