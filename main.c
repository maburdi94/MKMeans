
#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int x;
    int y;
    int z;
} vec3;



int loadInputData(vec3 **data, int capacity) {

    int count = 0;

    *data = (vec3*) malloc(sizeof(vec3) * capacity);

    char line[256], *pEnd;
    FILE *file = fopen("input.txt", "r");

    while ( fgets(line, sizeof(line), file) != NULL) {
        vec3 *vec = *data + count;

        vec->x = (int)strtol(line, &pEnd, 10);
        vec->y = (int)strtol(pEnd, &pEnd, 10);
        vec->z = (int)strtol(pEnd, NULL, 10);

        count++;

        // Allocate memory in chunks of 50 as needed
        if (count % capacity == 0) {
            capacity += capacity;
            *data = (vec3*) realloc(data, capacity);
        }
    }

    fclose(file);

    return count;
}





int main(int argc, char *argv[])
{
    int world_rank, world_size;

    MPI_Init(&argc,&argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Define a new MPI Datatype for our vectors
    MPI_Datatype VEC3;
    MPI_Type_contiguous(3, MPI_INT, &VEC3);
    MPI_Type_commit(&VEC3);

    int count;
    vec3 *training = NULL;

    /* Process 0 reads input and scatters data. */
    if (world_rank == 0) {
        count = loadInputData(&training, 50);
    }

    MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);


    MPI_Barrier(MPI_COMM_WORLD);

    int *displs = (int*)malloc(world_size*sizeof(int)),
        *scounts = (int*)malloc(world_size*sizeof(int));

    int p = count % world_size; // Number of extra data
    for (int i=0; i < world_size; ++i) {
        // Distribute extra data evenly
        scounts[i] = (i >= world_size - p) ? count/world_size + 1 : count/world_size;
        displs[i] = (i == 0) ? 0 : displs[i-1] + scounts[i-1];
    }


    printf("Process %d count = %d\n", world_rank, scounts[world_rank]);

    MPI_Barrier(MPI_COMM_WORLD);

    vec3 *recvbuf = (vec3*)malloc(sizeof(vec3) * scounts[world_rank]);

    // Give each process some of the vectors.
    // Each process will select ONE vector to be its "center".
    MPI_Scatterv(training, scounts, displs, VEC3, recvbuf, scounts[world_rank], VEC3, 0, MPI_COMM_WORLD);

    for (int i = 0; i < scounts[world_rank]; i++) {
        vec3 *vec = recvbuf + i;
        printf("P%d\t<%d,%d,%d>\n", world_rank, vec->x, vec->y, vec->z);
    }
    printf("\n");

    MPI_Barrier(MPI_COMM_WORLD);

    vec3 *centroids = (vec3*)malloc(sizeof(vec3) * world_size);
    int *rcounts = (int*) malloc(sizeof(int) * world_size);

    for (int i = 0; i < world_size; i++) {
        rcounts[i] = 1;
        displs[i] = i == 0 ? 0 : displs[i-1] + rcounts[i-1];
    }

    // Pick the trivial answer with first vector as center.
    MPI_Gatherv(recvbuf, 1, VEC3, centroids, rcounts, displs, VEC3, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        for (int i = 0; i < world_size; i++) {
            vec3 vec = centroids[i];
            printf("P%d center = \t<%d,%d,%d>\n", i, vec.x, vec.y, vec.z);
        }
        printf("\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

//
//
    free(training);
//    free(center);
    free(rcounts);
    free(centroids);
    free(displs);
    free(scounts);
    free(recvbuf);


    MPI_Finalize();
    return 0;
}
