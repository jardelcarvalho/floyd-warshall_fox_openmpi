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

    if(my_rank == P_MASTER_RANK) {
        int N;
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

        // envia submatrizes para os demais processos
        int t = N / q;
        t *= t;
        for(int p = 0; p < num_procs; p++) {
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
    int N;
    MPI_Recv(&N, 1, MPI_INT, P_MASTER_RANK, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int t = N / q;
    t *= t;
    float *my_sm = malloc(t * sizeof(float));
    if(!my_sm) {
        printf("erro: problema ao receber submatriz");
        free(my_sm);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    MPI_Recv(my_sm, t, MPI_FLOAT, P_MASTER_RANK, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // tamanho de uma dimensão da submatriz
    int sm_dim = sqrt(t);

    // aloca matrizes auxiliares
    float *m_a = malloc(t * sizeof(float));
    float *m_b = malloc(t * sizeof(float));
    float *m_res = malloc(t * sizeof(float));
    if(!m_a) {
        printf("erro: problema ao alocar matriz a\n");
        free(my_sm);
        free(m_b);
        free(m_res);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if(!m_b) {
        printf("erro: problema ao alocar mat b\n");
        free(my_sm);
        free(m_res);
        free(m_a);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if(!m_res) {
        printf("erro: problema ao alocar matriz resultado\n");
        free(my_sm);
        free(m_b);
        free(m_a);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    memcpy(m_res, my_sm, t * sizeof(float));

    // rotinas do Floyd-Warshall usando algorítmo de Fox
    int u, r = my_rank / q;
    int rank_acima = (my_rank + q * (q - 1)) % num_procs;
    int rank_abaixo = (my_rank + q) % num_procs;

    for(int f = 1; f < N; f *= 2) {

        for(int i = 0; i < t; i++) {
            m_b[i] = my_sm[i];
        }

        for(int step = 0; step < q; step++) {
            u = (r + step) % q;

            if(u == my_rank % q) {
                // enviando submatriz escolhida para demais processos da linha
                for(int sp = q * r; sp < q * r + q; sp++) {
                    MPI_Send(my_sm, t, MPI_FLOAT, sp, 0, MPI_COMM_WORLD);
                }
            }

            // recebendo submatriz u da sua linha
            MPI_Recv(m_a, t, MPI_FLOAT, r * q + u, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Floyd-Warshall
            floyd_warshall_mmult(m_a, m_b , m_res, sm_dim);

            // envia própria matriz para processo da linha de cima
            MPI_Send(m_b, t, MPI_FLOAT, rank_acima, 0, MPI_COMM_WORLD);

            // recebe matriz do processo da linha de baixo
            MPI_Recv(m_b, t, MPI_FLOAT, rank_abaixo, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for(int i = 0; i < t; i++) {
            my_sm[i] = m_res[i];
        }
    }

    // envia submatriz para o master
    MPI_Send(m_res, t, MPI_FLOAT, P_MASTER_RANK, 0, MPI_COMM_WORLD);

    // master recebe e agrupa submatrizes
    if(my_rank == P_MASTER_RANK) {
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
        for(int p = 0; p < num_procs; p++) {
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
