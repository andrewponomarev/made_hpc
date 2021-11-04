#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "omp.h"

double get_rand(unsigned int *seed) {
  return (double) rand_r(seed) / (double) RAND_MAX;
}

bool is_point_accepted(double r, double x, double y) {
  return (x * x + y * y) < r;
}

double pi_monte_carlo(double r, unsigned int total_points, unsigned int n_threads) {
  unsigned int accepted_points = 0;
  unsigned int seed_x, seed_y;
  double x, y;
  double pi;
  unsigned int i, tid;

#pragma omp parallel \
    private(x, y, seed_x, seed_y, i, tid) \
    shared(total_points, accepted_points, r2) \
    default(none) \
    num_threads(n_threads)
  {
    tid = omp_get_thread_num();
    seed_x = (time(NULL) & 0xFFFFFFF0) | (tid + 1);
    seed_y = (time(NULL) & 0xFFF0F0FF) | (tid + 51);

#pragma omp for reduction(+:accepted_points)
    for (i = 0; i < total_points; ++i) {

      x = get_rand(&seed_x);
      y = get_rand(&seed_y);
      accepted_points += is_point_accepted(r, x, y);
    }
  }
  pi = (accepted_points / (double) total_points) * 4;
  return pi;
}

int main() {
  const double r = 1;
  const unsigned int n_threads = 4;
  const unsigned int total_points = 100000000;

  double start = omp_get_wtime();
  double pi = pi_monte_carlo(r, total_points, n_threads);
  double end = omp_get_wtime();

  printf("Pi=%f\n", pi);
  printf("Time: %f s\n", end - start);

  return 0;
}