#include "pica_mp.h"
#include <mpi.h>
#include <iostream>
#include <cassert>

PICA_MP::PICA_MP(int pop_size, int dim, int max_iter,
    double beta, double gamma, double eta,
    double lb, double ub,
    const std::function<double(const std::vector<double>&)>& obj_func,
    int migration_cycles, int iterations_per_cycle,
    bool visual)
    : dim(dim), migration_cycles(migration_cycles), iterations_per_cycle(iterations_per_cycle)
{
    this->obj_func = obj_func;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if(visual)
        this->ica = new Visual_ICA(pop_size, dim, max_iter, beta, gamma, eta, lb, ub, obj_func);
    else
        this->ica = new ICA(pop_size, dim, max_iter, beta, gamma, eta, lb, ub, obj_func);
    this->ica->setup();
}

void PICA_MP::serialize_history(std::vector<double>& buffer)
{
    auto* visual_ica = static_cast<Visual_ICA*>(ica);
    const auto& history = visual_ica->get_history();
    buffer.clear();
    buffer.push_back(static_cast<double>(history.size())); // Number of snapshots

    for (const auto& snapshot : history)
    {
        const std::string& phase_name = snapshot.first;
        const std::vector<Visual_Country_Snapshot>& countries = snapshot.second;

        buffer.push_back(static_cast<double>(phase_name.length()));
        for (char c : phase_name)
            buffer.push_back(static_cast<double>(c));

        buffer.push_back(static_cast<double>(countries.size()));
        for (const auto& country : countries)
        {
            buffer.push_back(static_cast<double>(country.position.size()));
            for (double pos : country.position)
                buffer.push_back(pos);

            buffer.push_back(country.is_emperor ? 1.0 : 0.0);

            buffer.push_back(country.colour[0]);
            buffer.push_back(country.colour[1]);
            buffer.push_back(country.colour[2]);
        }
    }
}

std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> PICA_MP::deserialize_history(const std::vector<double>& buffer, int count)
{
    std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> history;
    size_t idx = 0;
    int num_snapshots = static_cast<int>(buffer[idx++]);

    for (int snap = 0; snap < num_snapshots; ++snap)
    {
        int name_length = static_cast<int>(buffer[idx++]);
        std::string phase_name;
        for (int i = 0; i < name_length; ++i)
            phase_name += static_cast<char>(buffer[idx++]);

        int num_countries = static_cast<int>(buffer[idx++]);
        std::vector<Visual_Country_Snapshot> countries;

        for (int c = 0; c < num_countries; ++c)
        {
            Visual_Country_Snapshot country;
            int dim = static_cast<int>(buffer[idx++]);
            country.position.resize(dim);
            for (int d = 0; d < dim; ++d)
                country.position[d] = buffer[idx++];

            country.is_emperor = (buffer[idx++] > 0.5);

            country.colour.resize(3);
            country.colour[0] = buffer[idx++];
            country.colour[1] = buffer[idx++];
            country.colour[2] = buffer[idx++];

            countries.push_back(std::move(country));
        }
        history.emplace_back(std::move(phase_name), std::move(countries));
    }
    return history;
}

std::vector<std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>>> PICA_MP::gather_visualization_history()
{
    std::vector<double> send_buffer;
    serialize_history(send_buffer);
    int local_size = send_buffer.size();

    std::vector<int> all_sizes;
    if (rank == 0)
        all_sizes.resize(size);

    MPI_Gather(&local_size, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    std::vector<int> processes_buffer_indexes;
    int total_size = 0;
    if (rank == 0)
    {
        processes_buffer_indexes.resize(size);
        for (int i = 0; i < size; ++i)
        {
            processes_buffer_indexes[i] = total_size;
            total_size += all_sizes[i];
        }
    }

    std::vector<double> recv_buffer;
    if (rank == 0)
        recv_buffer.resize(total_size);

    MPI_Gatherv(send_buffer.data(), local_size, MPI_DOUBLE, recv_buffer.data(), all_sizes.data(), processes_buffer_indexes.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if (rank == 0)
    {
        std::vector<std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>>> all_histories;

        for (int i = 0; i < size; ++i)
        {
            std::vector<double> processes_buffer(
                recv_buffer.begin() + processes_buffer_indexes[i],
                recv_buffer.begin() + processes_buffer_indexes[i] + all_sizes[i]
            );

            auto processes_history = deserialize_history(processes_buffer, all_sizes[i]);
            all_histories.push_back(std::move(processes_history));
        }
        return all_histories;
    }
    return {};
}

void PICA_MP::print_results(double fitness, std::vector<double>& location)
{
    std::cout << "Best fitness: " << fitness << std::endl;
    std::cout << "Best solution location: [";
    for (size_t i = 0; i < location.size(); ++i)
    {
        std::cout << location[i];
        if (i != location.size() - 1)
            std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

void PICA_MP::run()
{
    ica->run();
    ica->set_max_iter(iterations_per_cycle);
    MPI_Barrier(MPI_COMM_WORLD);

    for (int cycle = 0; cycle < migration_cycles; ++cycle) 
    {
        int prev = (rank - 1 + size) % size;
        int next = (rank + 1) % size;
        int tag = 0;

        std::vector<double> send_solution = ica->get_best_solution();
        std::vector<double> recv_solution(dim);

        for (int r = 0; r < size; ++r)
        {
            MPI_Barrier(MPI_COMM_WORLD);
        }

        int err = MPI_Sendrecv(send_solution.data(), dim, MPI_DOUBLE, next, tag, recv_solution.data(), dim, MPI_DOUBLE, prev, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        ica->migrate_best(recv_solution, obj_func);
        ica->run();
    }

    std::vector<double> local_best = ica->get_best_solution();
    double local_fitness = ica->get_fitness();

    std::vector<double> all_best_solutions;
    std::vector<double> all_fitnesses;

    if (rank == 0)
    {
        all_best_solutions.resize(size * dim);
        all_fitnesses.resize(size);
    }

    MPI_Gather(local_best.data(), dim, MPI_DOUBLE, all_best_solutions.data(), dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&local_fitness, 1, MPI_DOUBLE, all_fitnesses.data(), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) 
    {
        int best_index = 0;
        double best_fitness = all_fitnesses[0];
        for (int i = 1; i < size; ++i) 
        {
            if (all_fitnesses[i] < best_fitness) 
            {
                best_fitness = all_fitnesses[i];
                best_index = i;
            }
        }

        std::vector<double> global_best(dim);
        std::copy(all_best_solutions.begin() + best_index * dim, all_best_solutions.begin() + (best_index + 1) * dim, global_best.begin());
    }
}

std::vector<double> PICA_MP::get_best_solution() const 
{
    return ica->get_best_solution();
}

double PICA_MP::get_best_fitness() const 
{
    return ica->get_fitness();
}