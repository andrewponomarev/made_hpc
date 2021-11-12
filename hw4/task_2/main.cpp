#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include "matrix.h"
#include <iostream>


void page_rank(double* A, size_t n, double* rank, int niter, double d) {
  double* AP = new double[n * n];
  double* t = new double[n];
  double h = (1.0 - d) / n;
  #pragma omp parallel for shared(n, AP, A, d, h)
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      AP[i * n + j] = d * A[i * n + j] + h;
    }
  }
  double l1rank = l1norm(rank, n);
  #pragma omp parallel for shared(n, rank, l1rank)
  for (int i = 0; i < n; i++) {
    rank[i] = rank[i] / l1rank;
  }
  for (int iter = 0; iter < niter; iter++) {
    matrix_mul_vec(AP, rank, n, t);
    double l1t = l1norm(t, n);
  #pragma omp parallel for shared(n,rank,t, l1t)
    for (int i = 0; i < n; i++) {
      rank[i] = t[i] / l1t;
    }
  }
  double l1t = l1norm(rank, n);
  #pragma omp parallel for shared(n,rank,t, l1t)
  for (int i = 0; i < n; i++) {
    rank[i] = rank[i] / l1t * 100;
  }
  free(AP);
  free(t);
}


int main() {
  std::string fname = "input.txt";
  size_t n = 10;
  double* A = read_graph(fname, n);

  double* rank = new double[n];
  init_random_vector(rank, n);

  int niter = 100;
  double d = 0.7;

  double start = omp_get_wtime();
  page_rank(A, n, rank, niter, d);
  double end = omp_get_wtime();

  printf("\n----------------------------------");
  double sum = 0;
  printf("PageRank:\n");
  for (size_t i = 0; i < n; ++i)
  {
    printf("%2zu: %f", i, rank[i]);
    sum += rank[i];
  }
  printf("\n----------------------------------");
  printf("\nsum: %f", sum);
  printf("\nN = %zu", n);
  printf("----------------------------------\n");
  printf("Worked on %d threads, time elapsed = %f seconds\n", \
        omp_get_max_threads(), (end-start));
  printf("----------------------------------\n");

  free(A);
  return 0;
}