#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int FIELD_SIZE = 120;
const size_t STEPS = 100;


void init_field(int *field, int size, int value) {
    for (int i = 0; i < size; ++i) {
        field[i] = value;
    }
}


void print_field(int *field, int size) {
    for (int i = 0; i < size; ++i) {
        if(field[i] == 1) {
            printf("%s", "■");
        } else {
            printf("%s", "□");
        }
    }
    printf("\n");
}

int rule110(int a, int b, int c) {
    if (a == 1 && b == 1 && c == 1)
        return 0;
    else if (a == 1 && b == 1 && c == 0)
        return 1;
    else if (a == 1 && b == 0 && c == 1)
        return 1;
    else if (a == 1 && b == 0 && c == 0)
        return 0;
    else if (a == 0 && b == 1 && c == 1)
        return 1;
    else if (a == 0 && b == 1 && c == 0)
        return 1;
    else if (a == 0 && b == 0 && c == 1)
        return 1;
    else if (a == 0 && b == 0 && c == 0)
        return 0;
}

int rule121(int a, int b, int c) {
    if (a == 1 && b == 1 && c == 1)
        return 0;
    else if (a == 1 && b == 1 && c == 0)
        return 1;
    else if (a == 1 && b == 0 && c == 1)
        return 1;
    else if (a == 1 && b == 0 && c == 0)
        return 1;
    else if (a == 0 && b == 1 && c == 1)
        return 1;
    else if (a == 0 && b == 1 && c == 0)
        return 0;
    else if (a == 0 && b == 0 && c == 1)
        return 0;
    else if (a == 0 && b == 0 && c == 0)
        return 1;
}

void rule_step(int *field, int *field_tmp, int size, int rule_id) {
    int a, b, c;

    for (int i = 1; i < size-1; ++i) {
        a = field[i-1];
        b = field[i];
        c = field[i+1];

        if (rule_id == 110)
            field_tmp[i] = rule110(a, b, c);
        else if (rule_id == 121)
            field_tmp[i] = rule121(a, b, c);
        else
            printf("ERROR! Unknown rule id %d.\n", rule_id);
    }
}

int main(int argc, char ** argv) {
    int prank, psize;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &psize);
    MPI_Comm_rank(MPI_COMM_WORLD, &prank);

    srand(time(NULL) + prank);

    int FIELD_SIZE_local = FIELD_SIZE;

    if (FIELD_SIZE % psize != 0) {
        if (prank == 0) {
            printf("WARNING! The field of length %d cannot be divided equally between %d processes. ", FIELD_SIZE, psize);
            printf("Change field length or number of processes\n");
        }
        MPI_Finalize();
        return 0;
    } else {
        FIELD_SIZE_local = (int)(FIELD_SIZE / psize);
    }

    int *chunk;
    chunk = (int *) malloc((FIELD_SIZE_local+2)*sizeof(int));
    init_field(chunk, FIELD_SIZE_local+2, rand() % 2);

    int *chunk_next;
    chunk_next = (int *) malloc((FIELD_SIZE_local+2)*sizeof(int));
    init_field(chunk_next, FIELD_SIZE_local+2, 0);

    int *tmp_chunk;
    tmp_chunk = (int *) malloc((FIELD_SIZE_local)*sizeof(int));

    int *field;
    field = (int *) malloc(FIELD_SIZE*sizeof(int));

    for (int step = 0; step < STEPS; ++step) {
        init_field(field, FIELD_SIZE, 0);

        if (psize == 2) {
            if (prank == 0) {
                MPI_Send(&chunk[1], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
                MPI_Send(&chunk[FIELD_SIZE_local], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

                MPI_Recv(&chunk[FIELD_SIZE_local+1], 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
                MPI_Recv(&chunk[0], 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            }

            if (prank == 1) {
                MPI_Recv(&chunk[FIELD_SIZE_local+1], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                MPI_Recv(&chunk[0], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

                MPI_Send(&chunk[1], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                MPI_Send(&chunk[FIELD_SIZE_local], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
        } else if (psize > 2) {
            if (prank == 0) {
                MPI_Send(&chunk[1], 1, MPI_INT, psize-1, 0, MPI_COMM_WORLD);
            } else {
                MPI_Send(&chunk[1], 1, MPI_INT, prank-1, 0, MPI_COMM_WORLD);
            }

            if (prank == psize-1) {
                MPI_Send(&chunk[FIELD_SIZE_local], 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // FIELD_SIZE_local - 1 + 1
            } else {
                MPI_Send(&chunk[FIELD_SIZE_local], 1, MPI_INT, prank+1, 0, MPI_COMM_WORLD); // FIELD_SIZE_local - 1 + 1
            }

            if (prank == 0) {
                MPI_Recv(&chunk[0], 1, MPI_INT, psize-1, 0, MPI_COMM_WORLD, &status);
            } else {
                MPI_Recv(&chunk[0], 1, MPI_INT, prank-1, 0, MPI_COMM_WORLD, &status);
            }

            if (prank == psize-1) {
                MPI_Recv(&chunk[FIELD_SIZE_local+1], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            } else {
                MPI_Recv(&chunk[FIELD_SIZE_local+1], 1, MPI_INT, prank+1, 0, MPI_COMM_WORLD, &status);
            }
        }

        rule_step(chunk, chunk_next, FIELD_SIZE_local+2, 121);

        int *tmp;
        tmp = chunk;
        chunk = chunk_next;
        chunk_next = tmp;

        if (prank == 0) {
            for (int i = 0; i < FIELD_SIZE_local; ++i) {
                field[i] = chunk[i+1];
            }

            for (int p_count = 1; p_count < psize; ++p_count) {
                MPI_Recv(&tmp_chunk[0], FIELD_SIZE_local, MPI_INT, p_count, 0, MPI_COMM_WORLD, &status);
                for (int i = 0; i < FIELD_SIZE_local; ++i) {
                    field[status.MPI_SOURCE*FIELD_SIZE_local + i] = tmp_chunk[i];
                }
            }

            print_field(field, FIELD_SIZE);
        } else {
            MPI_Send(&chunk[1], FIELD_SIZE_local, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    return 0;
}