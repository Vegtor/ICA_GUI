#ifndef PICA_MS_H
#define PICA_MS_H

#include "ica.h"
#include "visual_ica.h"
#include <omp.h>
#include <functional>
#include <vector>

struct Mutiny_Action {
    Country* colony;               
    Country* new_empire;      
    bool empire_swap;     
};

class PICA_MS
{
private:
    ICA* ica;
    std::function<double(const std::vector<double>&)> obj_func;
    int num_threads;

    void mutiny_buffer();


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

    void setup();
    void run();
    
    ~PICA_MS() = default;
};

#endif