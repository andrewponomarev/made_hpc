#include <string>

#pragma once
void matrix_mul_vec(double* A, double* v, size_t n, double* R);
double l1norm(double* a, int n);
double* read_graph(std::string& fname, size_t N);
void init_random_vector(double* a, size_t);
