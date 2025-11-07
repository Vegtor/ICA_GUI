#ifndef PICA_MS_H
#define PICA_MS_H

#include "ica.h"
#include "visual_ica.h"
#include <omp.h>
#include <functional>
#include <vector>

class PICA_MS : public Visual_ICA
{
private:
    std::function<double(const std::vector<double>&)> obj_func;
    int num_threads;


public:
    PICA_MS(
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
        int num_threads = 4
    );

    void setup() override;
    
    ~PICA_MS();
};

#endif