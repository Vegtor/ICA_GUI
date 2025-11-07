#ifndef ICA_H
#define ICA_H

#include "Country.h"
#include <vector>
#include <functional>

class ICA
{
protected:
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
    
    // country/empire fitness calculation
    void calculate_fitness();
    // setup of n first countries as emperors
    virtual void create_empires();
    // setup of colonies to empires
    virtual void create_colonies();
    // process of assimilation of small empires
    void assimilation();
    // movement of colony
    void revolution();
    // revolt against emperor
    void mutiny();
    // war between empires - colonies shift between empires
    void imperial_war();
public:
    // setup of ICA class
    ICA(int pop_size, int dim, int max_iter, double beta, double gamma, double eta, double lb, double ub, const std::function<double(const std::vector<double>&)>& obj_func);

    // setup of colonies
    virtual void setup();

    // algorithm run
    virtual void run();

    // adding best empire from different proces
    void migrate_best(const std::vector<double>& elite_solution, const std::function<double(const std::vector<double>&)>& obj_func);

    // getters for fitness, solution coordinates
    double get_fitness();
    std::vector<double> get_best_solution();

    // setter for max iterations
    void set_max_iter(int max_iter);

    void check(int rank);

    ~ICA();
};

#endif // ICA_H