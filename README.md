# Projeto

Algoritmo de Floyd-Warshall paralelo com OpenMPI através do algoritmo de Fox. Implementação sem o uso de comunicadores.

### Prerequisitos

Para compilar e executar

```
make
gcc
mpicc
mpirun
```

### Compilação

make

## Execução

mpirun openmpi-flags main matriz.txt

## Entrada (matriz.txt)

6\
0 2 0 5 0 0\
0 0 0 0 0 0\
0 2 0 0 0 5\
0 0 0 0 1 0\
3 9 3 0 0 0\
0 0 0 0 1 0

```
Primeira linha indica o tamanho de uma dimensão da matriz
```
