// autor: Jardel Carvalho
// ano: 2019

#include "matriz_tr.h"

float *submatriz(int rank, int N, int q, float *m) {
    int s = N / q;
    float *sm = malloc(s * s * sizeof(float));
    if(!sm) {
        printf("erro: problema ao alocar matriz\n");
        return NULL;
    }

    /* 
    faixa deve obrigatóriamente conter a parte inteira 
    da divisão rank / q como resultado
    se faixa for do tipo float vão surgir bugs
    */
    int faixa = rank / q;

    int k = 0, x_coord, y_coord, offset, i, f;
    for(int l = 0; l < s; l++) {
        x_coord = (rank % q) * (N / q);
        y_coord = N * l;
        offset = faixa * ((N * N) / q);

        i = x_coord + y_coord + offset;
        f = ((rank % q) + 1) * (N / q) + y_coord + offset;
        
        for(int t = i; t < f; t++) {
            sm[k] = m[t];
            k++;
        }
    }

    return sm;
}

void encaixa(float *rcvd_m, float *m, int N, int q, int rank) {
    /* 
    faixa deve obrigatóriamente conter a parte inteira 
    da divisão rank / q como resultado
    se faixa for do tipo float vão surgir bugs
    */
    int faixa = rank / q;

    int s = N / q;
    int k = 0, x_coord, y_coord, offset, i, f;
    for(int l = 0; l < s; l++) {
        x_coord = (rank % q) * (N / q);
        y_coord = N * l;
        offset = faixa * ((N * N) / q);

        i = x_coord + y_coord + offset;
        f = ((rank % q) + 1) * (N / q) + y_coord + offset;
        
        for(int t = i; t < f; t++) {
            m[t] = rcvd_m[k];
            k++;
        }
    }
}

void print_m(float *m_, int N) {
    for(int i = 0; i < N * N; i++) {
        printf("%.2f ", m_[i]);
        if((i + 1) % N == 0) {
            printf("\n");
            continue;
        }
    }
}