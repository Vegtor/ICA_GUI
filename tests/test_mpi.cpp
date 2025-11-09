#include "../ICA_GUI/pica_mp.h"
#include "gtest/gtest.h"
#include <mpi.h>
#include "testing_functions.h"

// ============================================================================
// MPI Test Fixture
// ============================================================================

class PICA_MP_Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }

    void TearDown() override
    {
        MPI_Barrier(MPI_COMM_WORLD);
    }

    int rank;
    int size;
};

// ============================================================================
// PICA_MP Constructor Tests
// ============================================================================

TEST_F(PICA_MP_Test, ConstructorInitializesCorrectly)
{
    PICA_MP pica_mp(50, 3, 100, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, 5, 20, false);

    auto solution = pica_mp.get_best_solution();
    double fitness = pica_mp.get_best_fitness();

    EXPECT_EQ(solution.size(), 3);
    EXPECT_LT(fitness, INFINITY);
}

TEST_F(PICA_MP_Test, ConstructorWithVisualMode)
{
    PICA_MP pica_mp(30, 2, 50, 2.0, 0.1, 0.1, -3.0, 3.0,
        sphere_function, 3, 10, true);

    auto solution = pica_mp.get_best_solution();
    EXPECT_EQ(solution.size(), 2);
}

// ============================================================================
// PICA_MP Basic Functionality Tests
// ============================================================================

TEST_F(PICA_MP_Test, RunCompletesWithoutErrors)
{
    PICA_MP pica_mp(40, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, 2, 15, false);

    EXPECT_NO_THROW(pica_mp.run());

    auto solution = pica_mp.get_best_solution();
    double fitness = pica_mp.get_best_fitness();

    EXPECT_EQ(solution.size(), 3);
    EXPECT_LT(fitness, INFINITY);
}

TEST_F(PICA_MP_Test, MigrationCyclesExecuteCorrectly)
{
    int migration_cycles = 3;
    PICA_MP pica_mp(30, 2, 30, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, migration_cycles, 10, false);

    EXPECT_NO_THROW(pica_mp.run());

    auto solution = pica_mp.get_best_solution();
    EXPECT_EQ(solution.size(), 2);
}

// ============================================================================
// PICA_MP Convergence Tests
// ============================================================================

TEST_F(PICA_MP_Test, ConvergenceWithSphereFunction)
{
    PICA_MP pica_mp(50, 3, 100, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, 5, 20, false);

    double initial_fitness = pica_mp.get_best_fitness();
    pica_mp.run();
    double final_fitness = pica_mp.get_best_fitness();

    // Should converge to better solution
    EXPECT_LE(final_fitness, initial_fitness);

    // For sphere function, should find reasonable solution
    EXPECT_LT(final_fitness, 10.0);
}

TEST_F(PICA_MP_Test, ConvergenceWithRastriginFunction)
{
    PICA_MP pica_mp(60, 2, 80, 2.0, 0.1, 0.1, -5.12, 5.12,
        rastrigin_function, 4, 15, false);

    pica_mp.run();
    double final_fitness = pica_mp.get_best_fitness();

    EXPECT_LT(final_fitness, INFINITY);
    EXPECT_GT(final_fitness, 0.0); // Rastrigin minimum > 0
}

// ============================================================================
// PICA_MP Communication Tests
// ============================================================================

TEST_F(PICA_MP_Test, SolutionExchangeBetweenProcesses)
{
    // Only run this test if we have at least 2 processes
    if (size >= 2)
    {
        PICA_MP pica_mp(25, 2, 20, 2.0, 0.1, 0.1, -2.0, 2.0,
            sphere_function, 2, 8, false);

        // Get initial solution
        std::vector<double> initial_solution = pica_mp.get_best_solution();
        double initial_fitness = pica_mp.get_best_fitness();

        pica_mp.run();

        std::vector<double> final_solution = pica_mp.get_best_solution();
        double final_fitness = pica_mp.get_best_fitness();

        // Solution should change due to migration
        bool solution_changed = (initial_solution != final_solution);
        EXPECT_TRUE(solution_changed || final_fitness < initial_fitness);
    }
    else
    {
        // Skip test with message
        std::cout << "Skipping multi-process test - requires at least 2 MPI processes" << std::endl;
        SUCCEED();
    }
}

TEST_F(PICA_MP_Test, NoDeadlockInCommunication)
{
    PICA_MP pica_mp(35, 3, 25, 2.0, 0.1, 0.1, -3.0, 3.0,
        sphere_function, 3, 12, false);

    // Should complete without hanging
    EXPECT_NO_THROW(pica_mp.run());

    MPI_Barrier(MPI_COMM_WORLD);
    SUCCEED();
}

// ============================================================================
// PICA_MP Visualization Tests
// ============================================================================

TEST_F(PICA_MP_Test, VisualModeHistoryCollection)
{
    // Only run this test if we have at least 2 processes
    if (size >= 2)
    {
        PICA_MP pica_mp(30, 2, 20, 2.0, 0.1, 0.1, -3.0, 3.0,
            sphere_function, 2, 8, true);

        pica_mp.run();

        auto all_histories = pica_mp.gather_visualization_history();

        if (rank == 0) {
            // Root process should gather history from all processes
            EXPECT_EQ(all_histories.size(), size);
            for (const auto& history : all_histories) {
                EXPECT_FALSE(history.empty());
            }
        }
        else {
            // Non-root processes return empty vector
            EXPECT_TRUE(all_histories.empty());
        }
    }
    else
    {
        // Skip test with message
        std::cout << "Skipping multi-process visualization test - requires at least 2 MPI processes" << std::endl;
        SUCCEED();
    }
}

TEST_F(PICA_MP_Test, VisualAndNonVisualProduceSimilarResults)
{
    PICA_MP pica_visual(40, 3, 30, 2.0, 0.1, 0.1, -4.0, 4.0,
        sphere_function, 2, 10, true);
    PICA_MP pica_normal(40, 3, 30, 2.0, 0.1, 0.1, -4.0, 4.0,
        sphere_function, 2, 10, false);

    pica_visual.run();
    pica_normal.run();

    double visual_fitness = pica_visual.get_best_fitness();
    double normal_fitness = pica_normal.get_best_fitness();

    // Both should find reasonable solutions
    EXPECT_LT(visual_fitness, 10.0);
    EXPECT_LT(normal_fitness, 10.0);
}

// ============================================================================
// PICA_MP Parameter Variation Tests
// ============================================================================

TEST_F(PICA_MP_Test, DifferentPopulationSizes)
{
    std::vector<int> pop_sizes = { 20, 50, 100 };

    for (int pop_size : pop_sizes) {
        EXPECT_NO_THROW({
            PICA_MP pica_mp(pop_size, 3, 20, 2.0, 0.1, 0.1, -5.0, 5.0,
                           sphere_function, 1, 10, false);
            pica_mp.run();
            auto solution = pica_mp.get_best_solution();
            EXPECT_EQ(solution.size(), 3);
            });
    }
}

TEST_F(PICA_MP_Test, DifferentDimensions)
{
    std::vector<int> dimensions = { 2, 5, 8 };

    for (int dim : dimensions) {
        EXPECT_NO_THROW({
            PICA_MP pica_mp(40, dim, 25, 2.0, 0.1, 0.1, -5.0, 5.0,
                           sphere_function, 2, 12, false);
            pica_mp.run();
            auto solution = pica_mp.get_best_solution();
            EXPECT_EQ(solution.size(), dim);
            });
    }
}

TEST_F(PICA_MP_Test, DifferentMigrationCycles)
{
    std::vector<int> migration_cycles = { 1, 3, 5 };

    for (int cycles : migration_cycles) {
        EXPECT_NO_THROW({
            PICA_MP pica_mp(35, 3, 30, 2.0, 0.1, 0.1, -4.0, 4.0,
                           sphere_function, cycles, 10, false);
            pica_mp.run();
            });
    }
}

// ============================================================================
// PICA_MP Edge Case Tests
// ============================================================================

TEST_F(PICA_MP_Test, SingleProcessExecution)
{
    // This test is specifically for single process execution
    // It will run on any number of processes but is most meaningful with 1
    PICA_MP pica_mp(30, 2, 20, 2.0, 0.1, 0.1, -3.0, 3.0,
        sphere_function, 2, 8, false);

    EXPECT_NO_THROW(pica_mp.run());

    auto solution = pica_mp.get_best_solution();
    double fitness = pica_mp.get_best_fitness();

    EXPECT_EQ(solution.size(), 2);
    EXPECT_LT(fitness, INFINITY);
}

TEST_F(PICA_MP_Test, SmallPopulation)
{
    PICA_MP pica_mp(10, 2, 15, 2.0, 0.1, 0.1, -2.0, 2.0,
        sphere_function, 1, 5, false);

    EXPECT_NO_THROW(pica_mp.run());

    auto solution = pica_mp.get_best_solution();
    EXPECT_EQ(solution.size(), 2);
}

// ============================================================================
// PICA_MP Best Solution Tests
// ============================================================================

TEST_F(PICA_MP_Test, GetBestSolutionReturnsValidSolution)
{
    PICA_MP pica_mp(40, 3, 30, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, 2, 10, false);

    pica_mp.run();

    auto solution = pica_mp.get_best_solution();
    double fitness = pica_mp.get_best_fitness();

    EXPECT_EQ(solution.size(), 3);
    EXPECT_LT(fitness, INFINITY);

    // Check bounds
    for (double val : solution) {
        EXPECT_GE(val, -5.0);
        EXPECT_LE(val, 5.0);
    }
}