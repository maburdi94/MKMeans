
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


#define THRESHOLD 0.1


 // A 3-vector implementation
typedef struct {
    int x;
    int y;
    int z;
} vec3, dvec3;


void merge(vec3** final_set, vec3* all_sets, int K, int N) {


}


int main(int argc, char* argv[])
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

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
    vec3* training = NULL;


    if (world_rank == 0) {
        // Process 0 reads input
        count = loadInputData(&training, 50);
    }

    // Wait for all processes to start the algorithm.
    MPI_Barrier(MPI_COMM_WORLD);

    // Share the count with every process so each process can figure out its portion.
    MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Wait for everyone
    MPI_Barrier(MPI_COMM_WORLD);

    // Distribute vectors among the processes
    MPI_Scatterv(training, scounts, displs, VEC3, rbuf, scounts[world_rank], VEC3, 0, MPI_COMM_WORLD);

    // Wait for everyone
    MPI_Barrier(MPI_COMM_WORLD);

    // Each process selects K out of N objects to be centroids
    vec3* centroids;

    // Calc the distance index
    double Jprime, J = 0;
    do {

        Jprime = calc_index();

        // Count the number of items in each cluster so later we can get the average
        int cluster_counts[K];

        // Initialize all counts to 1. (Later, this will be the denominator to get the mean.)
        for (int i = 0; i < K; i++) cluster_counts[i] = 1;


        // Go through all the objects...
        // Remember: The first K items are the centers, so we have to start from K + 1 (zero-indexed)
        for (int n = K; n < N; n++) {

            // ...and find the closest centroid
            double min_distance;
            int nearest_k;
            for (int k = 0; k < K; k++) {
                double distance = calc_distance(rbuf[n], centroids[k]);
                if (distance < min_distance) {
                    min_distance = distance;
                    nearest_k = k;
                }
            }

            // Add the object to the nearest cluster centroid
            centroids[nearest_k] += rbuf[n];

            // Keep track of the current cluster count
            cluster_counts[nearest_k]++;

        }


        // Update the centroids by finding the mean of all its objects
        for (int k = 0; k < K; k++) {
            centroids[k].x /= cluster_counts[k];
            centroids[k].y /= cluster_counts[k];
            centroids[k].z /= cluster_counts[k];
        }

        // Calc the distance index
        J = calc_index(rbuf, N, centroids, K);

    } while (Jprime - J >= THRESHOLD);


    vec3* all_centroids;

    // Pick the trivial answer with first vector as center.
    MPI_Gather(centroids, K, VEC3, all_centroids, K, VEC3, 0, MPI_COMM_WORLD);


    // Currently all_centroids contains N*K clusters.
    // K clusters for each of N processes.
    // [ [p11 p12 ... p1K] [p21 p22 ... p2K] ... [pN1 pN2 ...pNK] ]

    if (world_rank == 0) {
        //for each process
        for (int i = 0; i < world_size; i++) {
            printf("Process %d centroids\n", i);
            //for each centroid 
            for (int k = 0; k < K; k++) {
                print;
            }
            print new line;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    //
    //
    free(training);
    //    free(center);
    free(centroids);
    free(displs);
    free(scounts);
    free(rbuf);


    MPI_Finalize();
    return 0;
}
