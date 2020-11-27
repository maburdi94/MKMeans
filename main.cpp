
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <regex>
//#include <mpi.h>

int main(int argc, char *argv[])
{
//    MPI_Comm everyone;           /* intercommunicator */
//    MPI_Init(&argc, &argv);

    /*
     * Read input file
     */
    std::vector<int*> data;
    std::string line;
    std::ifstream myfile ("input.txt");

    const std::regex re(R"(-?\d+)");

    if (myfile.is_open())
    {

        while ( getline (myfile,line) ) {
            std::stringstream ss(line);
            int *vec = new int[3];

            ss >> vec[0];
            ss >> vec[1];
            ss >> vec[2];

            data.push_back(vec);
        }

        myfile.close();
    }

//    for (const auto & vector : data) {
//        std::cout << "<";
//        for (int i = 0; i < 3; i++) {
//            std::cout << vector[i] << (i < 2 ? ", " : "");
//        }
//        std::cout << ">" << std::endl;
//    }


    // Define a new MPI Datatype for our vectors
//    MPI_Datatype VECTOR;
//    MPI_Type_contiguous(3, MPI_INT, &VECTOR);
//    MPI_Type_commit(&VECTOR);



 /*
  * Scatter data to all processes
  */
//    MPI_Send(data, n_points, VECTOR, 1, 0, MPI_COMM_WORLD);



    //MPI_Finalize();
    return 0;
}
