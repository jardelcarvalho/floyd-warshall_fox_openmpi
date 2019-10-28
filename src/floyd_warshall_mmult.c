// autor: Jardel Carvalho
// ano: 2019

#include "floyd_warshall_mmult.h"

void inicia_matriz_fl_war(float *m, int N) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {   
            if(i == j) {
                // para este programa a diagonal principal é sempre zero
                m[i + i * N] = 0;
            } else if(m[i + j * N] == 0) {
                m[i + j * N] = INFINITY;
            } else {
                m[i + j * N] = m[i + j * N];
            }
        }
    }
}


void floyd_warshall_mmult(float *m_a, float *m_b, float *m_res, int N) {
    float s, d;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {

            d = m_res[j + i * N];
            for(int k = 0; k < N; k++) {
                s = m_a[k + i * N] + m_b[j + k * N];
                if(d > s) {
                    m_res[j + i * N] = s;
                    d = s;
                }
            }
        }
    }
}