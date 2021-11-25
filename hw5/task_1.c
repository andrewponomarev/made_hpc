#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

const size_t MAX_NAME_LEN = 10;

int get_random_process(int prank, int psize) {
    int to_process = rand() % psize;
    while (to_process == prank) {
        to_process = rand() % psize;
    }
    return to_process;
}

void print_previous_process_names(const char* previous_process_names, int psize) {
    printf("\n");
    for (int i = 0; i < psize*MAX_NAME_LEN; ++i) {
        if (previous_process_names[i] == 0) {
            printf(".");
        } else {
            printf("%c", previous_process_names[i]);
        }
    }
    printf("\n\n");
}


int main(int argc, char ** argv) {
    int prank, psize;
    MPI_Status status;
    int ierr;
    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &psize);
    MPI_Comm_rank(MPI_COMM_WORLD, &prank);

    int to_process;

    char *process_name;
    process_name = (char *) malloc(11*sizeof(char));

    char *previous_process_names;
    previous_process_names = (char*) malloc(psize * MAX_NAME_LEN * sizeof(char));

    if (prank==0) {
        to_process = get_random_process(prank, psize);
        printf("Send name from %d to %d\n", 0, to_process);
        MPI_Ssend("process0", MAX_NAME_LEN, MPI_UNSIGNED_CHAR, to_process, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&process_name[0], MAX_NAME_LEN, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        printf("Process %d received name %s from process with rank %d.\n", prank, process_name, status.MPI_SOURCE);

        if (status.MPI_SOURCE == 0) {
            for (int i = 0; i < psize * MAX_NAME_LEN; ++i)
                previous_process_names[i] = 0;
            printf("Process %d initialize orderedNames ", prank);
        } else {
            MPI_Recv(&previous_process_names[0], psize * MAX_NAME_LEN, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            printf("Process %d received orderedNames from process with rank %d ", prank, status.MPI_SOURCE);
        }

        printf("and set %s in %d position\n", process_name, status.MPI_SOURCE);

        for (int i = 0; i < sizeof(process_name)/sizeof(char)+1; i++) {
            previous_process_names[status.MPI_SOURCE*MAX_NAME_LEN + i] = process_name[i];
        }

        print_previous_process_names(previous_process_names, psize);

        int count = 0;
        for (int i = 0; i < psize*MAX_NAME_LEN; i += MAX_NAME_LEN) {
            if(previous_process_names[i] != 0)
                count++;
        }
        printf("Said 'hello' %d processes.\n", count);

        if (count + 1 < psize) {
            to_process = get_random_process(prank, psize);
            while (previous_process_names[to_process*MAX_NAME_LEN] != 0) {
                to_process = get_random_process(prank, psize);
            }
            char *name_own;
            name_own = (char *) malloc(11*sizeof(char));
            printf("Send name from %d to %d\n", prank, to_process);
            snprintf(name_own, MAX_NAME_LEN, "process%d", prank);
            MPI_Ssend(&name_own[0], MAX_NAME_LEN, MPI_CHAR, to_process, 0, MPI_COMM_WORLD);
            MPI_Ssend(&previous_process_names[0], psize * MAX_NAME_LEN, MPI_CHAR, to_process, 0, MPI_COMM_WORLD);
        } else {
            printf("\nProcess %d knows the names of all other %d processes:\n", prank, psize-1);
            for (int i = 0; i < psize*MAX_NAME_LEN; i+=MAX_NAME_LEN) {
                if (previous_process_names[i] == 0) {
                    continue;
                } else {
                    printf("Process#%d name: ", (int)(i/MAX_NAME_LEN));
                    int j = 0;
                    while (previous_process_names[i + j] != 0){
                        printf("%c", previous_process_names[i + j]);
                        j++;
                    }
                    printf("\n");
                }
            }
        }
    }
    printf("\n");
    MPI_Finalize();
    return 0;
}