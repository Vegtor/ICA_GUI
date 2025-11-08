#ifndef ICA_H
#define ICA_H

#include "Country.h"
#include <vector>
#include <functional>

class ICA
{
public:
    int pop_size;
    int dim; 
    int max_iter;
    std::function<double(const std::vector<double>&)> obj_func;
    std::vector<Country*> population, empires, colonies;

    std::vector<double> best_solution;
    double best_fitness;
    double tp;

    double beta;
    double gamma;
    double eta;

    double lb;
    double ub;

    void calculate_fitness();

    virtual void create_empires();

    virtual void create_colonies();

    void assimilation();

    void assimilation_of_empire(int idx);

    void revolution();

    void revolution_of_empire(int idx);

    void mutiny();

    void imperial_war();

    ICA(int pop_size, int dim, int max_iter, double beta, double gamma, double eta, double lb, double ub, const std::function<double(const std::vector<double>&)>& obj_func);

    virtual void setup();

    virtual void run();

    void migrate_best(const std::vector<double>& elite_solution, const std::function<double(const std::vector<double>&)>& obj_func);

    double get_fitness();
    std::vector<double> get_best_solution();

    void set_max_iter(int max_iter);

    void check(int rank);

    ~ICA();
};

#endif // ICA_H