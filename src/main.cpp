/*
#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "pica_mp.h" 

// Rastrigin function
double rastrigin(const std::vector<double>& x) 
{
    double A = 10.0;
    double result = A * x.size();
    for (double xi : x) 
    {
        result += xi * xi - A * std::cos(2 * 3.14159265358979323846 * xi);
    }
    return result;
}

int main(int argc, char** argv) 
{
    double start, end;
    MPI_Init(&argc, &argv);
    
    start = MPI_Wtime();
    int pop_size = 80;
    int dim = 12;
    int max_iter = 150;
    double lb = -10;
    double ub = 10;

    // assim coef
    double beta = 2.0;
    // rev rate
    double gamma = 0.1;
    // imp comp rate
    double eta = 0.1;

    int migration_cycles = 30;
    int iterations_per_cycle = 150;

    bool visual = true;

    MPI_Barrier(MPI_COMM_WORLD);    

    PICA_MP pica_mpi(pop_size, dim, max_iter, beta, gamma, eta, lb, ub, rastrigin, migration_cycles, iterations_per_cycle, visual);
    pica_mpi.run();
    std::vector<std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>>> visualization = pica_mpi.gather_visualization_history();

    end = MPI_Wtime();
    MPI_Finalize();

    std::cout << "Time: " << end - start << std::endl;

    return 0;
}

*/