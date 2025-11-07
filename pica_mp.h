#ifndef PICA_MP_H
#define PICA_MP_H

#include "ica.h"
#include "visual_ica.h"
#include <functional>
#include <vector>

class PICA_MP
{
private:
    int rank;
    int size;
    int migration_cycles;
    int iterations_per_cycle;
    int dim;

    ICA* ica;
    std::function<double(const std::vector<double>&)> obj_func;

    void serialize_history(std::vector<double>& buffer);
    std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> deserialize_history(const std::vector<double>& buffer, int count);
    void print_results(double fitness, std::vector<double>& location);   
public:
    PICA_MP(int pop_size, int dim, int max_iter,
        double beta, double gamma, double eta,
        double lb, double ub,
        const std::function<double(const std::vector<double>&)>& obj_func,
        int migration_cycles, int iterations_per_cycle,
        bool visual);
    void run();

    std::vector<std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>>> gather_visualization_history();
    
    std::vector<double> get_best_solution() const;
    double get_best_fitness() const;

};

#endif