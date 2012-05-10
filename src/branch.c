#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bce.h"

#define EPSILON 0.001

/*  int main() */
/*  { */
/*      printf("%f\n", branch_factor(52,5)); */
/*  } */

   
double fn(double b, double d, double n)
{
    return pow(b, d+1) - n*b + n - 1;
}

double fn_prime(double b, double d, double n)
{
    return (d+1) * pow(b, d) - n;
}

double branch_factor(int nodes, int depth)
{
    double b = pow(nodes, 1.0/depth);
    
    int i;
    
    for (i=0;i<1000;i++)
    {
	double denom =  fn_prime(b, depth, nodes);
	double num = fn(b, depth, nodes);
	double b_next;
	double diff;
	
	if (denom == 0.0) denom = 0.001;
	
	b_next = b - num / denom;

	diff = (b>b_next) ? b-b_next : b_next -b;
	
	if (diff<EPSILON)
	    return b;
	
	b = b_next;
    }

    return -1;
}
