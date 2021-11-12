#include <string>

#pragma once
void matrix_mult(int* A, int* B, size_t n, int *R);
void matrix_power(int* A, size_t n, int p, int* R);
void print_matrix(int* A, int n);
void generate_random_matrix(int * A, size_t n);
void generate_zero_matrix(int * A, size_t n);