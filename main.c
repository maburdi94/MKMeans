
#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "math.h"


/*
 * Based on a parallel K-means algorithm found here:
 * A Parallel Clustering Algorithm with MPI – MKmeans
 * http://www.jcomputers.us/vol8/jcp0801-02.pdf
 *
 * However, the implementation of the algorithm is left unspecified
 * by the researchers. That is what we have come up with here.
 */



// A 3-vector implementation
typedef struct {
    int x;
    int y;
    int z;
} vec3, dvec3;


/**
 * Calculate the square Euclidean distance between two points (vectors)
 * @param v1 - First vector
 * @param v2 - Second vector
 * @return The Euclidean distance between the two input points
 */
double calc_distance(const vec3 v1, const vec3 v2) {
    return pow(v2.x - v1.x, 2) + pow(v2.y - v1.y, 2) + pow(v2.z - v1.z, 2);
}

/**
 * Calculate the distance index of all objects in a process.
 * @param objects
 * @param N - number of objects
 * @param centroids
 * @param K - number of clusters (centroids)
 * @return The distance index
 */
double calc_index(vec3 *objects, int N, vec3 *centroids, int K) {
    double sum = 0;
    for (int n = 0; n < N; n++) {
        for (int k = 0; k < K; k++) {
            sum += calc_distance(objects[n], centroids[k]);
        }
    }
    return sum;
}


/**
 * Load in data from a text file where every line contains 3 numerical integer values separated by spaces.
 * @param data - A pointer to an array that will be filled with the values retrieved.
 * @param capacity - The initial expected size of the data. If data is larger,
 * then the array will be reallocated with {capacity} more elements.
 * @return The exact number of items read from the file.
 */
int loadInputData(vec3 **data, int capacity) {

    // Track number of items read
    int count = 0;

    // Allocate enough space for expected capacity
    *data = (vec3*) malloc(sizeof(vec3) * capacity);

    char line[256], *pEnd;

    // Open file
    FILE *file = fopen("input3.txt", "r");

    // Line-by-line extract 3 values and add push them into array
    while ( fgets(line, sizeof(line), file) != NULL) {
        vec3 *vec = *data + count;

        vec->x = (int)strtol(line, &pEnd, 10);
        vec->y = (int)strtol(pEnd, &pEnd, 10);
        vec->z = (int)strtol(pEnd, NULL, 10);

        count++;

        // Allocate memory in chunks of capacity as needed
        if (count % capacity == 0) {
            capacity += capacity;
            *data = (vec3*) realloc(data, capacity);
        }
    }

    // Close file
    fclose(file);


    return count;
}





int main(int argc, char *argv[])
{
    int world_rank, world_size;

    MPI_Init(&argc,&argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Define a new MPI Datatype for a 3-vector (3 contiguous integers in memory)
    MPI_Datatype VEC3;
    MPI_Type_contiguous(3, MPI_INT, &VEC3);
    MPI_Type_commit(&VEC3);

    int count,

    K = (argc > 1) ? // Did user provide number of clusters?
        (int)strtol(argv[1], NULL, 10) : // Get user-provided K
        3; // Otherwise, use default value of 3.

    // The whole set of data before scattering
    vec3 *training = NULL;


    if (world_rank == 0) {
        // Process 0 reads input
        count = loadInputData(&training, 50);

        printf("Number of clusters: %d\n", K);

        // We cannot have more clusters than the number of vectors per each process.
        // Solution: Either reduce number of processes or use less clusters.
        if (K > count/world_size) {
            fprintf(stderr, "K must be less than the number of elements / number of processes, but %d > %d. \n", K, count/world_size);

            MPI_Finalize();

            // Terminate with error.
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    // Wait for all processes to start the algorithm.
    MPI_Barrier(MPI_COMM_WORLD);

    // Share the count with every process so each process can figure out its portion.
    MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);


    int *displs = (int*)malloc(world_size*sizeof(int)),
        *scounts = (int*)malloc(world_size*sizeof(int));

    // Count is indivisible by number of processes.
    int p = count % world_size;

    // Distribute the items among processes and give one more at a time
    // to processes where number is not evenly divisible.
    for (int i=0; i < world_size; ++i) {
        scounts[i] = (i >= world_size - p) ? count/world_size + 1 : count/world_size;
        displs[i] = (i == 0) ? 0 : displs[i-1] + scounts[i-1];
    }

    // Each process shares how many items it will handle
    printf("Process %d count = %d\n", world_rank, scounts[world_rank]);

    // Wait for everyone
    MPI_Barrier(MPI_COMM_WORLD);

    // Allocate memory for each process to retrieve its items from the master root process
    vec3 *rbuf = (vec3*)malloc(sizeof(vec3) * scounts[world_rank]);

    // Distribute vectors among the processes
    MPI_Scatterv(training, scounts, displs, VEC3, rbuf, scounts[world_rank], VEC3, 0, MPI_COMM_WORLD);

    // Print out all the vectors
    for (int i = 0; i < scounts[world_rank]; i++) {
        vec3 *vec = rbuf + i;
        printf("P%d\t<%d,%d,%d>\n", world_rank, vec->x, vec->y, vec->z);
    } printf("\n");

    // Wait for everyone
    MPI_Barrier(MPI_COMM_WORLD);

    // Each process selects K out of N objects to be centroids
    vec3 *centroids = (vec3*)malloc(sizeof(vec3) * K);

    // The trivial solution is to just select the first K vectors.
    // Do a byte-by-byte copy from buffer to centroids.
    memcpy(centroids, rbuf, sizeof(vec3)*K);

    // Calc the distance index
    double Jprime = calc_index(rbuf, scounts[world_rank], centroids, K);

    // CONT here on step 7 of MKmeans


    int *rcounts = (int*) malloc(sizeof(int) * world_size);

    for (int i = 0; i < world_size; i++) {
        rcounts[i] = 1;
        displs[i] = i == 0 ? 0 : displs[i-1] + rcounts[i-1];
    }

    // Pick the trivial answer with first vector as center.
    MPI_Gatherv(rbuf, 1, VEC3, centroids, rcounts, displs, VEC3, 0, MPI_COMM_WORLD);

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
    free(rbuf);


    MPI_Finalize();
    return 0;
}
