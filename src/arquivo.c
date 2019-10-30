// autor: Jardel Carvalho
// ano: 2019

#include "arquivo.h"

float *le_matriz(char *caminho, int *N) {
    float *m;

    FILE *f = fopen(caminho, "r");
    if(!f) {
        printf("erro: problema na leitura de %s\n", caminho);
        return NULL;
    }

    // caminha até o fim do arquivo e conta os bytes
    fseek(f, 0L, SEEK_END);
    int n_bytes = ftell(f);
    rewind(f);
    // lê o arquivo pra a memória
    char bytes[n_bytes + 1];
    fread(bytes, 1, n_bytes, f);
    fclose(f);
    bytes[n_bytes] = '\0';

    // caminha até a primeira quebra de linha
    int i = 0;
    for(; bytes[i] != '\n'; i++);
    bytes[i] = '\0';
    i++;
    
    // converte substring que contém o valor de N para int
    *N = atoi(bytes);
    // aloca a matriz
    m = malloc((*N * *N) * sizeof(float));
    if(!m) {
        printf("erro: problema ao alocar matriz\n");
        return NULL;
    }

    // grava valores da matriz no vetor
    int j = i, k = 0;
    for(; bytes[i] != '\0'; i++) {
        if(bytes[i] == ' ' || bytes[i] == '\n') {
            bytes[i] = '\0';
            float v = atof(bytes + j);
            m[k] = v;
            k++;
            j = i + 1;
        }
    }
    
    return m;
}