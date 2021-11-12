#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "matrix.h"
#include <math.h>
#include "string"
#include <fstream>


void matrix_mul_vec(double* A, double* v, size_t n, double* R) {
  #pragma omp parallel for shared(R, A, v, n)
  for (size_t i = 0; i < n; ++i) {
    R[i] = 0;
    for (int j = 0; j < n; ++j)
      R[i] += A[i * n + j] * v[j];
  }
}

double l1norm(double* a, int n) {
  double res = 0;
  #pragma omp parallel for reduction(+:res)
  for (int i = 0; i < n; ++i)
  {
    res += fabs(a[i]);
  }
  return res;
}

double* read_graph(std::string& fname, size_t N) {
  int n;
  int m;
  std::ifstream fin(fname);
  if (!fin) return nullptr;
  fin >> n >> m;
  double *A = new double [n * n];
  memset(A, 0, n * n * sizeof(double));
  int i, j;
  double v;
  for (int e = 0; e < m; e++) {
    fin >> i >> j >> v;
    A[j * n + i] = v;
  }
  n = n;
  return A;
}

void init_random_vector(double* v, size_t n) {
  srand(n);
  for(int i = 0; i < n; i++) {
    v[i] = rand();
  }
}