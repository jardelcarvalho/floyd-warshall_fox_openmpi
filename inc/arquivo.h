// autor: Jardel Carvalho
// ano: 2019

#ifndef _ARQUIVO_H_
#define _ARQUIVO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

float *le_matriz(char *caminho, int *N);
void escreve_matriz(float *m, int N, char *caminho);

#endif