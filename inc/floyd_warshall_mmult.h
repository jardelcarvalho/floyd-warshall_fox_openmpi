// autor: Jardel Carvalho
// ano: 2019

#ifndef _FLOYD_WARSHALL_MMULT_H_
#define _FLOYD_WARSHALL_MMULT_H_

#include <stdlib.h>
#include <math.h>

void inicia_matriz_fl_war(float *m, int N);
void floyd_warshall_mmult(float *m_a, float *m_b, float *m_res, int N);

#endif