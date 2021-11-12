#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"
#include <sys/time.h>
#include <omp.h>

int main()
{
  size_t n = 10;
  double start, end;
  int pow = 3;

  int *A = (int *) malloc(n * n * sizeof(int));
  generate_random_matrix(&A[0], n);
  printf("\nGenerated random matrix of size= %zu:\n", n);
  print_matrix(A, n);
  printf("----------------------------------\n");

  int *R = (int *) malloc(n * n * sizeof(int));
  generate_zero_matrix(&R[0], n);

  start = omp_get_wtime();
  matrix_power(&A[0], n, pow, &R[0]);
  end = omp_get_wtime();

  printf("\nResult paths of length = %d:\n", pow);
  print_matrix(R, n);
  printf("----------------------------------\n");

  printf("Worked on %d threads, time elapsed = %f seconds\n", \
        omp_get_max_threads(), (end-start));
  printf("----------------------------------\n");

  free(A);
  free(R);
  return 0;
}
