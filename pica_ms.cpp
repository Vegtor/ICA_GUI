#include "pica_ms.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

void PICA_MS::setup_parallel()
{
    int countries_per_thread = ica->pop_size / num_threads;
    int remainder = ica->pop_size % num_threads;
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int start = tid * countries_per_thread;
        int count = countries_per_thread + (tid < remainder ? 1 : 0);

        for (int i = 0; i < count; ++i)
        {
            std::vector<double> pos(ica->dim);
            for (int j = 0; j < ica->dim; ++j)
            {
                double r = static_cast<double>(std::rand()) / (RAND_MAX + 1.0);
                pos[j] = ica->lb + r * (ica->ub - ica->lb);
            }

            // this could be made better
            #pragma omp critical
            {
                ica->population.push_back(new Country(pos));
            }
        }
    }
}

void PICA_MS::run_parallel()
{
    for (int iter = 0; iter < ica->max_iter; ++iter) 
    {
        calculate_fitness_parallel();
        #pragma omp for
        for (size_t i = 0; i < ica->empires.size(); ++i)
        {
            ica->assimilation_of_empire(i);
            ica->revolution_of_empire(i);
        }
        mutiny_parallel();
        ica->imperial_war();

        if (ica->empires.size() == 1)
            break;
    }
}

void PICA_MS::run_parallel_visual()
{
    for (int iter = 0; iter < ica->max_iter; ++iter)
    {
        calculate_fitness_parallel();

        #pragma omp for
        for (size_t i = 0; i < ica->empires.size(); ++i)
        {
            ica->assimilation_of_empire(i);
        }
        state_snapshot_parallel("Assimilation");

        #pragma omp for
        for (size_t i = 0; i < ica->empires.size(); ++i)
        {
            ica->revolution_of_empire(i);
        }
        state_snapshot_parallel("Revolution");

        mutiny_parallel();
        state_snapshot_parallel("Mutiny");

        ica->imperial_war(); 
        state_snapshot_parallel("Imperial War");

        if (ica->empires.size() == 1)
            break;
    }
}

void PICA_MS::state_snapshot_parallel(std::string phase_name)
{
    auto visual_ica = static_cast<Visual_ICA*>(ica);
    std::vector<Visual_Country_Snapshot> step;
    #pragma omp parallel
    {
        std::vector<Visual_Country_Snapshot> local_step;

        #pragma omp for
        for (auto* c : visual_ica->population)
        {
            Visual_Country_Snapshot country_snapshot;
            country_snapshot.position = c->location;
            country_snapshot.is_emperor = (c->vassal_of_empire == nullptr);

            auto vc = static_cast<Visual_Country*>(c);
            country_snapshot.colour = vc->get_colour();

            local_step.push_back(std::move(country_snapshot));
        }

        #pragma omp critical
        {
            step.insert(step.end(),
                std::make_move_iterator(local_step.begin()),
                std::make_move_iterator(local_step.end()));
        }
    }
    visual_ica->history.emplace_back(phase_name, std::move(step));
}


void PICA_MS::mutiny_parallel()
{
    std::vector<std::vector<Mutiny_Action>> thread_buffers(num_threads);

    #pragma omp for
    for (size_t i = 1; i < ica->empires.size(); ++i)
    {
        int tid = omp_get_thread_num();
        std::vector<Mutiny_Action>& local_buffer = thread_buffers[tid];

        auto* empire = ica->empires[i];
        for (size_t j = 1; j < empire->vassals.size(); ++j)
        {
            auto* vassal = empire->vassals[j];
            Mutiny_Action temp;
            temp.colony = vassal;
            auto* nearest_imperialist = *std::min_element(ica->empires.begin(), ica->empires.end(), [&](Country* a, Country* b)
                {
                    double country_a = 0;
                    double country_b = 0;
                    for (size_t k = 0; k < ica->dim; ++k)
                    {
                        country_a += std::pow(a->location[k] - vassal->location[k], 2);
                        country_b += std::pow(b->location[k] - vassal->location[k], 2);
                    }
                    return country_a < country_b;
                });
            if (vassal->vassal_of_empire != nearest_imperialist)
                temp.new_empire = nearest_imperialist;
            else
                temp.new_empire = vassal->vassal_of_empire;

            temp.empire_swap = (vassal->fitness < nearest_imperialist->fitness);

            if (!(temp.new_empire == vassal->vassal_of_empire && temp.empire_swap == false))
                local_buffer.push_back(temp);

        }
    }

    std::vector<Mutiny_Action> mutiny_buffer;
    for (auto& buf : thread_buffers)
        mutiny_buffer.insert(mutiny_buffer.end(), buf.begin(), buf.end());

    for (auto& action : mutiny_buffer)
    {
        #pragma omp critical
        {
            auto& vassals = action.colony->vassal_of_empire->vassals;
            vassals.erase(std::remove(vassals.begin(), vassals.end(), action.colony), vassals.end());

            if (action.empire_swap)
            {
                action.colony->coup(action.new_empire);

                auto emp_idx = std::find(ica->empires.begin(), ica->empires.end(), action.new_empire);
                if (emp_idx != ica->empires.end())
                    *emp_idx = action.colony;

                auto col_idx = std::find(action.new_empire->vassals.begin(), action.new_empire->vassals.end(), action.colony);
                if (col_idx != action.new_empire->vassals.end())
                    *col_idx = action.new_empire;
            }
            else
            {
                action.new_empire->add_vassal(action.colony);
                action.colony->add_emperor(action.new_empire);
            }
        }
    }
}

void PICA_MS::calculate_fitness_parallel()
{
    #pragma omp for
    for (auto& c : ica->population)
    {
        c->evaluate_fitness(obj_func);
        if (c->fitness < ica->best_fitness)
        {
            ica->best_fitness = c->fitness;
            ica->best_solution = c->location;
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
) : num_threads(num_threads), obj_func(obj_func), visual(visual)
{
    if(visual)
        ica = new Visual_ICA(pop_size, dim, max_iter, beta, gamma, eta, lb, ub, obj_func);
    else
        ica = new ICA(pop_size, dim, max_iter, beta, gamma, eta, lb, ub, obj_func);
    omp_set_num_threads(num_threads);
}