// autor: Jardel Carvalho
// ano: 2019

#ifndef _MATRIZ_TR_H_ 
#define _MATRIZ_TR_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float *submatriz(int rank, int N, int q, float *m);
void encaixa(float *rcvd_m, float *m, int N, int q, int rank);
void print_m(float *m_, int N);

#endif