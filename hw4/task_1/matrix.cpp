#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "matrix.h"
#include <math.h>
#include "string"
#include <fstream>

void matrix_mult(int* A, int* B, size_t n, int *R) {

  int *temp = (int *) malloc(n * n * sizeof(int));
  generate_zero_matrix(&temp[0], n);

  #pragma omp parallel for shared(temp, A, B)
  for (int k = 0; k < n; k++)
    for (size_t i = 0; i < n; i++)
    {
      for (size_t j = 0; j < n; j++)
        temp[i * n + j] = temp[i * n + j] + A[i * n + k] * B[k * n + j];
    }

  #pragma omp parallel for shared(temp, R)
  for (size_t i = 0; i < n; i++)
    for (size_t j = 0; j < n; j++)
      R[i * n + j] = temp[i * n + j];
}

void matrix_power(int* A, size_t n, int pow, int* R) {

  if (pow == 1) {
  #pragma omp parallel for shared(A, R)
    for (size_t i = 0; i < n; i++)
      for (size_t j = 0; j < n; j++)
      {
        R[i * n + j] = A[i * n + j];
      }
    return;
  }
  matrix_power(A, n, pow / 2, R);
  matrix_mult(R, R, n, R);
  if (pow % 2 != 0)
    matrix_mult(R, A, n, R);
}

void print_matrix(int* A, int n) {
  for (int i = 0; i < n; i++)
  {
    for (size_t j = 0; j < n; j++)
      printf("%d\t", A[n * i + j]);
    printf("\n");
  }
}

void generate_random_matrix(int * A, size_t n) {
  srand(time(NULL));

  #pragma omp parallel for shared(A)
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      A[i * n + j] = (int) (rand() % 2);
    }
  }
}


void generate_random_matrix(double * A, size_t n) {
  srand(time(NULL));

  #pragma omp parallel for shared(A)
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      A[i * n + j] = (int) (rand() % 2);
    }
  }
}

void generate_zero_matrix(int * A, size_t n) {
  #pragma omp parallel for shared(A)
  for (size_t i = 0; i < n; i++)
  {
    for (size_t j = 0; j < n; j++)
    {
      A[i * n + j] = 0;
    }
  }
}