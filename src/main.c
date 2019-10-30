// autor: Jardel Carvalho
// ano: 2019

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mpi.h"

#include "arquivo.h"
#include "matriz_tr.h"
#include "floyd_warshall_mmult.h"

#define P_MASTER_RANK 0


int main(int argc, char **argv) {
    if(argc != 2) {
        printf("erro: parâmetros insuficientes\n");
        return 0;
    }

    int num_procs;
    int my_rank;

    double t_ini;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int q = sqrt(num_procs);
    int N, t;
    float *my_sm;

    if(my_rank == P_MASTER_RANK) {
        float *m = le_matriz(argv[1], &N);
        if(!m) {
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
        
        // inicia medição do tempo
        t_ini = MPI_Wtime();

        // condição para uso do algoritmo de Fox
        if(!(q * q == num_procs && N % q == 0)) {
            printf("erro: condição de Fox não foi satisfeita\n");
            free(m);
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }

        // inicia matriz com pesos infinitos onde (i = j) o peso é 0
        // inicialização Floyd-Warshall
        inicia_matriz_fl_war(m, N);

        // master pegando sua submatriz
        my_sm = submatriz(P_MASTER_RANK, N, q, m);

        // envia submatrizes para os demais processos
        t = N / q;
        t *= t;
        for(int p = 1; p < num_procs; p++) {
            float *sm = submatriz(p, N, q, m);
            if(!sm) {
                free(m);
                MPI_Finalize();
                exit(EXIT_FAILURE);
            }
            // enviando valor de N
            MPI_Send(&N, 1, MPI_INT, p, 0, MPI_COMM_WORLD);

            // enviando submatriz
            MPI_Send(sm, t, MPI_FLOAT, p, 0, MPI_COMM_WORLD);

            // desalocando submatriz enviada
            free(sm);
        }

        // desaloca matriz base
        free(m);
    }

    // o código a partir daqui será executado por todos os processos
    // com exceção dos condicionais para P_MASTER_RANK

    // recebe dados enviados pelo master
    if(my_rank != P_MASTER_RANK) {
        MPI_Recv(&N, 1, MPI_INT, P_MASTER_RANK, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        t = N / q;
        t *= t;
        my_sm = malloc(t * sizeof(float));
        if(!my_sm) {
            printf("erro: problema ao receber submatriz");
            free(my_sm);
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
        MPI_Recv(my_sm, t, MPI_FLOAT, P_MASTER_RANK, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // tamanho de uma dimensão da submatriz
    int sm_dim = sqrt(t);

    // aloca matrizes auxiliares
    float *m_a = malloc(t * sizeof(float));
    float *m_b = malloc(t * sizeof(float));
    float *m_b_cpy = malloc(t * sizeof(float));
    float *m_res = malloc(t * sizeof(float));
    if(!m_a) {
        printf("erro: problema ao alocar matriz a\n");
        free(m_a);
        free(m_b);
        free(m_b_cpy);
        free(m_res);
        free(my_sm);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if(!m_b) {
        printf("erro: problema ao alocar mat b\n");
        free(m_a);
        free(m_b);
        free(m_b_cpy);
        free(m_res);
        free(my_sm);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if(!m_b_cpy) {
        printf("erro: problema ao alocar matriz resultado\n");
        free(m_a);
        free(m_b);
        free(m_b_cpy);
        free(m_res);
        free(my_sm);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if(!m_res) {
        printf("erro: problema ao alocar matriz resultado\n");
        free(m_a);
        free(m_b);
        free(m_b_cpy);
        free(m_res);
        free(my_sm);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    memcpy(m_res, my_sm, t * sizeof(float));

    // rotinas do Floyd-Warshall usando algorítmo de Fox
    int u, r = my_rank / q;
    int rank_acima = (my_rank + q * (q - 1)) % num_procs;
    int rank_abaixo = (my_rank + q) % num_procs;
    MPI_Request request;
    MPI_Status status;

    for(int f = 1; f < N; f *= 2) {

        for(int i = 0; i < t; i++) {
            m_b[i] = my_sm[i];
        }

        for(int step = 0; step < q; step++) {
            u = (r + step) % q;

            if(u == my_rank % q) {
                memcpy(m_a, my_sm, t * sizeof(float));

                // enviando submatriz escolhida para demais processos da linha
                for(int sp = q * r; sp < q * r + q; sp++) {
                    if(sp != my_rank) {
                        MPI_Send(my_sm, t, MPI_FLOAT, sp, 0, MPI_COMM_WORLD);
                    }
                }
            } else {
                // recebendo submatriz u da sua linha
                MPI_Recv(m_a, t, MPI_FLOAT, r * q + u, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // Floyd-Warshall
            floyd_warshall_mmult(m_a, m_b , m_res, sm_dim);

            // envia própria matriz para processo da linha de cima, não bloqueante
            MPI_Isend(m_b, t, MPI_FLOAT, rank_acima, 0, MPI_COMM_WORLD, &request);

            // copia m_b para m_b_cpy para caso a chamada mpi wait necessite reenviar m_b
            memcpy(m_b_cpy, m_b, t * sizeof(float));

            // recebe matriz do processo da linha de baixo
            MPI_Recv(m_b, t, MPI_FLOAT, rank_abaixo, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // aguarda até que a linha de cima tenha recebido a matriz
            MPI_Wait(&request, &status);
        }

        for(int i = 0; i < t; i++) {
            my_sm[i] = m_res[i];
        }
    }

    if(my_rank != P_MASTER_RANK) {
        // envia submatriz para o master
        MPI_Send(m_res, t, MPI_FLOAT, P_MASTER_RANK, 0, MPI_COMM_WORLD);
    } else {
        // master recebe e agrupa submatrizes
        float *m = malloc(N * N * sizeof(float));
        if(!m) {
            printf("erro: problema ao agrupar as submatrizes\n");
            free(m_a);
            free(m_b);
            free(m_res);
            free(my_sm);
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }

        // master encaixando a sua submatriz
        encaixa(m_res, m, N, q, P_MASTER_RANK);

        for(int p = 1; p < num_procs; p++) {
            MPI_Recv(m_res, t, MPI_FLOAT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            encaixa(m_res, m, N, q, p);
        }

        // encerra a contagem do tempo
        double t_fim = MPI_Wtime();

        // imprime matriz
        print_m(m, N);
        printf("\n");

        printf("t = %.15lfs\n", t_fim - t_ini);	

        free(m);
    }

    // desalocando dados
    free(m_a);
    free(m_b);
    free(m_res);
    free(my_sm);

    MPI_Finalize();

    return 0;
}
