#include "pica_ms.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

void PICA_MS::setup()
{
    int countries_per_thread = pop_size / num_threads;
    int remainder = pop_size % num_threads;
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int start = tid * countries_per_thread;
        int count = countries_per_thread + (tid < remainder ? 1 : 0);

        for (int i = 0; i < count; ++i)
        {
            std::vector<double> pos(dim);
            for (int j = 0; j < dim; ++j)
            {
                double r = static_cast<double>(std::rand()) / (RAND_MAX + 1.0);
                pos[j] = lb + r * (ub - lb);
            }

            // this could be made better
            #pragma omp critical
            {
                population.push_back(new Country(pos));
            }
        }
    }
}

PICA_MS::PICA_MS(
    int pop_size,
    int dim,
    int max_iter,
    double beta,
    double gamma,
    double eta,
    double lb,
    double ub,
    const std::function<double(const std::vector<double>&)>& obj_func,
    bool visual,
    int num_threads
) : num_threads(num_threads), Visual_ICA(
    pop_size,
    dim,
    max_iter,
    beta,
    gamma,
    eta,
    lb,
    ub,
    obj_func)
{

    this->obj_func = obj_func;
    omp_set_num_threads(num_threads);
}