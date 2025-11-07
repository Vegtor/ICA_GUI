#include "visual_ica.h"
#include <random>


void Visual_ICA::state_snapshot(std::string phase_name)
{
    std::vector<Visual_Country_Snapshot> step;
    for (auto* c : population) {
        Visual_Country_Snapshot country_snapshot;
        country_snapshot.position = c->location;
        country_snapshot.is_emperor = (c->vassal_of_empire == nullptr);

        if (auto vc = dynamic_cast<Visual_Country*>(c)) 
            country_snapshot.colour = vc->get_colour();
        else 
            country_snapshot.colour = { 0.0, 0.0, 0.0 }; 

        step.push_back(std::move(country_snapshot));
    }
    history.emplace_back(phase_name, std::move(step));
}

std::vector<double> Visual_ICA::random_colour()
{
    static std::default_random_engine gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return { dist(gen), dist(gen), dist(gen) };
}

void Visual_ICA::empire_colouring()
{
    if (!dynamic_cast<Visual_Country*>(empires.front()))
        return;

    for (auto& e : empires)
    {
        auto* empire = dynamic_cast<Visual_Country*>(e);
        std::vector<double> temp_colour = this->random_colour();
        empire->set_colour(temp_colour);
        for (auto& v : e->vassals)
        {
            auto* vassal = dynamic_cast<Visual_Country*>(v);
            vassal->set_colour(temp_colour);
        }
    }
}

Visual_ICA::Visual_ICA(
    int pop_size, 
    int dim, 
    int max_iter, 
    double beta, 
    double gamma, 
    double eta, 
    double lb, 
    double ub, 
    const std::function<double(const std::vector<double>&)>& obj_func)
    :ICA(pop_size, dim, max_iter, beta, gamma, eta, lb, ub, obj_func)
{}

void Visual_ICA::setup()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (size_t i = 0; i < pop_size; ++i)
    {
        std::vector<double> pos(dim);
        for (int j = 0; j < dim; ++j)
        {
            double r = static_cast<double>(std::rand()) / (RAND_MAX + 1.0);
            pos[j] = lb + r * (ub - lb);
        }
        population.push_back(new Visual_Country(pos));
    }

    calculate_fitness();
    std::sort(population.begin(), population.end(), [](Country* a, Country* b)
        {
            return a->fitness < b->fitness;
        });
    create_empires();
    create_colonies();
}

void Visual_ICA::run()
{
    for (int i = 0; i < max_iter; ++i)
    {
        calculate_fitness();
        assimilation();
        state_snapshot("Assimilation");
        revolution();
        state_snapshot("Revolution");
        mutiny();
        state_snapshot("Mutiny");
        imperial_war();
        state_snapshot("Imperial War");
        if (empires.size() == 1)
            break;
    }
}

std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> Visual_ICA::get_history()
{
    return this->history;
}
