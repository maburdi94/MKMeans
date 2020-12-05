
#include <fstream>

#include "mpi.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char *argv[])
{
    MPI_Comm everyone;           /* intercommunicator */
    MPI_Init(NULL, NULL);

    int world_rank, world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Define a new MPI Datatype for our vectors
    MPI_Datatype VEC3;
    MPI_Type_contiguous(3, MPI_LONG, &VEC3);
    MPI_Type_commit(&VEC3);

    long *training;

    /* Process 0 reads input and scatters data. */
    int count = 0, capacity = 50;

    if (world_rank == 0) {

        training = (long*) malloc(sizeof(long)*3 * capacity);

        char line[256], *pEnd;
        FILE *file = fopen("input3.txt", "r");

        long *vec = (long*) calloc(3, sizeof(long));
        while ( fgets(line, sizeof(line), file) != nullptr) {

            vec[0] = strtol(line, &pEnd, 10);
            vec[1] = strtol(pEnd, &pEnd, 10);
            vec[2] = strtol(pEnd, nullptr, 10);

            memcpy(training + count*3, vec, sizeof(long)*3);
            count++;

            // Allocate memory in chunks of 50 as needed
            if (count % capacity == 0) {
                training = (long *) realloc(training, capacity += capacity);
            }
        }

        fclose(file);

    }

    MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);


    int *displs, *scounts;

    displs = (int *)malloc(world_size*sizeof(int));
    scounts = (int *)malloc(world_size*sizeof(int));

    int p = count % world_size; // Number of extra data
    for (int i=0; i < world_size; ++i) {
        // Distribute extra data evenly
        scounts[i] = (i >= world_size - p) ? count/world_size + 1 : count/world_size;
        displs[i] = (i == 0) ? 0 : displs[i-1] + scounts[i-1];
    }

//    printf("P%d scounts[]=%d,%d\n", world_rank, scounts[0], scounts[1]);
//    printf("P%d displs[]=%d,%d\n", world_rank, displs[0], displs[1]);

    long *recbuf = (long*)malloc(sizeof(long) * count/world_size + 1);
//    int recvct = scounts[world_rank];

    MPI_Scatterv(training, scounts, displs, VEC3, recbuf, scounts[world_rank], VEC3, 0, MPI_COMM_WORLD);


    printf("P%d count = %d\n", world_rank, scounts[world_rank]);
    for (int i = 0; i < scounts[world_rank]; i++) {
        long *vec = (recbuf + i*3);
        printf("<%ld,%ld,%ld>\n", vec[0], vec[1], vec[2]);
    }
    printf("\n");

//    int _sum = sum(*sub_data);

//    auto v1 = *sub_data, v2 = *(sub_data + 6 - 1);
//    printf("P%d got <%d, %d, %d> to <%d, %d, %d>.\n", world_rank, v1[0], v1[1], v1[2], v2[0], v2[1], v2[2]);


    MPI_Barrier(MPI_COMM_WORLD);

//    int *sums = (int*)malloc(world_size * sizeof(int));

//    MPI_Gather(&_sum, world_size, MPI_INT, sums, );





 /*
  * Scatter data to all processes
  */

    free(displs);
    free(scounts);
    free(training);

    free(recbuf);


    MPI_Finalize();
    return 0;
}
