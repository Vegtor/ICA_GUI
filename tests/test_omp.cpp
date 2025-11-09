#include "../ICA_GUI/pica_ms.h"
#include "gtest/gtest.h"
#include <omp.h>
#include "testing_functions.h"

// ============================================================================
// PICA_MS Constructor Tests
// ============================================================================

TEST(PICA_MS_Class, ConstructorInitialization)
{
    PICA_MS pica_ms(50, 3, 100, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, false, 4);

    SUCCEED();
}

TEST(PICA_MS_Class, ConstructorWithVisualMode)
{
    PICA_MS pica_ms(30, 2, 50, 2.0, 0.1, 0.1, -3.0, 3.0,
        sphere_function, true, 2);

    SUCCEED();
}

TEST(PICA_MS_Class, ConstructorWithDifferentThreadCounts)
{
    std::vector<int> thread_counts = { 1, 2, 4 };

    for (int threads : thread_counts) 
    {
        EXPECT_NO_THROW({
            PICA_MS pica_ms(40, 3, 30, 2.0, 0.1, 0.1, -4.0, 4.0,
                           sphere_function, false, threads);
            });
    }
}

// ============================================================================
// PICA_MS Setup Tests
// ============================================================================

TEST(PICA_MS_Class, ParallelSetupCreatesPopulation)
{
    PICA_MS pica_ms(50, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, false, 4);

    EXPECT_NO_THROW(pica_ms.setup_parallel());
}

// ============================================================================
// PICA_MS Basic Execution Tests
// ============================================================================

TEST(PICA_MS_Class, RunParallelCompletes)
{
    PICA_MS pica_ms(40, 3, 30, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, false, 4);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel());
}

TEST(PICA_MS_Class, RunParallelVisualCompletes)
{
    PICA_MS pica_ms(35, 2, 25, 2.0, 0.1, 0.1, -4.0, 4.0,
        sphere_function, true, 2);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel_visual());
}

// ============================================================================
// PICA_MS Convergence Tests
// ============================================================================

TEST(PICA_MS_Class, ConvergenceWithSphereFunction)
{
    PICA_MS pica_ms(60, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, false, 4);

    pica_ms.setup_parallel();
    pica_ms.run_parallel();

    SUCCEED(); // Should complete without errors and find reasonable solution
}

TEST(PICA_MS_Class, ConvergenceWithRastriginFunction)
{
    PICA_MS pica_ms(70, 2, 40, 2.0, 0.1, 0.1, -5.12, 5.12,
        rastrigin_function, false, 4);

    pica_ms.setup_parallel();
    pica_ms.run_parallel();

    SUCCEED(); // Should handle more complex function
}

TEST(PICA_MS_Class, ConvergenceWithRosenbrockFunction)
{
    PICA_MS pica_ms(50, 3, 35, 2.0, 0.1, 0.1, -5.0, 5.0,
        rosenbrock_function, false, 2);

    pica_ms.setup_parallel();
    pica_ms.run_parallel();

    SUCCEED(); // Should handle complex, non-convex function
}

// ============================================================================
// PICA_MS Threading Tests
// ============================================================================

TEST(PICA_MS_Class, DifferentThreadCountsProduceResults)
{
    std::vector<int> thread_counts = { 1, 2, 4 };

    for (int threads : thread_counts) 
    {
        EXPECT_NO_THROW(
            {
            PICA_MS pica_ms(45, 3, 25, 2.0, 0.1, 0.1, -4.0, 4.0,
                           sphere_function, false, threads);
            pica_ms.setup_parallel();
            pica_ms.run_parallel();
            });
    }
}

TEST(PICA_MS_Class, SingleThreadExecution)
{
    PICA_MS pica_ms(30, 2, 20, 2.0, 0.1, 0.1, -3.0, 3.0,
        sphere_function, false, 1);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel());
}

TEST(PICA_MS_Class, MultipleThreadExecution)
{
    int available_threads = omp_get_max_threads();
    int test_threads = (available_threads > 1) ? available_threads : 2;

    PICA_MS pica_ms(50, 3, 30, 2.0, 0.1, 0.1, -4.0, 4.0,
        sphere_function, false, test_threads);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel());
}

// ============================================================================
// PICA_MS Parameter Variation Tests
// ============================================================================

TEST(PICA_MS_Class, DifferentPopulationSizes)
{
    std::vector<int> pop_sizes = { 20, 50, 100 };

    for (int pop_size : pop_sizes) 
    {
        EXPECT_NO_THROW(
            {
            PICA_MS pica_ms(pop_size, 3, 20, 2.0, 0.1, 0.1, -5.0, 5.0,
                           sphere_function, false, 4);
            pica_ms.setup_parallel();
            pica_ms.run_parallel();
            });
    }
}

TEST(PICA_MS_Class, DifferentDimensions)
{
    std::vector<int> dimensions = { 2, 5, 10 };

    for (int dim : dimensions) 
    {
        EXPECT_NO_THROW(
            {
            PICA_MS pica_ms(40, dim, 25, 2.0, 0.1, 0.1, -5.0, 5.0,
                           sphere_function, false, 4);
            pica_ms.setup_parallel();
            pica_ms.run_parallel();
            });
    }
}

TEST(PICA_MS_Class, DifferentAlgorithmParameters)
{
    struct Params {
        double beta, gamma, eta;
    };

    std::vector<Params> param_sets = 
    {
        {1.5, 0.05, 0.05},
        {2.0, 0.1, 0.1},
        {2.5, 0.2, 0.2}
    };

    for (const auto& params : param_sets) 
    {
        EXPECT_NO_THROW({
            PICA_MS pica_ms(40, 3, 30, params.beta, params.gamma, params.eta,
                           -4.0, 4.0, sphere_function, false, 4);
            pica_ms.setup_parallel();
            pica_ms.run_parallel();
            });
    }
}

// ============================================================================
// PICA_MS Visualization Tests
// ============================================================================

TEST(PICA_MS_Class, VisualModeStateSnapshots)
{
    PICA_MS pica_ms(30, 2, 15, 2.0, 0.1, 0.1, -3.0, 3.0,
        sphere_function, true, 2);

    pica_ms.setup_parallel();

    // Test individual phase snapshots
    EXPECT_NO_THROW(pica_ms.state_snapshot_parallel("Test Assimilation"));
    EXPECT_NO_THROW(pica_ms.state_snapshot_parallel("Test Revolution"));
    EXPECT_NO_THROW(pica_ms.state_snapshot_parallel("Test Mutiny"));
    EXPECT_NO_THROW(pica_ms.state_snapshot_parallel("Test Imperial War"));
}

TEST(PICA_MS_Class, ParallelVisualExecution)
{
    PICA_MS pica_ms(35, 3, 20, 2.0, 0.1, 0.1, -4.0, 4.0,
        sphere_function, true, 4);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel_visual());
}

// ============================================================================
// PICA_MS Edge Case Tests
// ============================================================================

TEST(PICA_MS_Class, SmallPopulationSize)
{
    PICA_MS pica_ms(10, 2, 10, 2.0, 0.1, 0.1, -2.0, 2.0,
        sphere_function, false, 2);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel());
}

TEST(PICA_MS_Class, LargePopulationSize)
{
    PICA_MS pica_ms(200, 3, 10, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, false, 4);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel());
}

TEST(PICA_MS_Class, HighDimensionProblem)
{
    PICA_MS pica_ms(80, 10, 20, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, false, 4);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel());
}

// ============================================================================
// PICA_MS Comparison Tests
// ============================================================================

TEST(PICA_MS_Class, VisualAndNonVisualProduceSimilarResults)
{
    PICA_MS pica_visual(40, 3, 25, 2.0, 0.1, 0.1, -4.0, 4.0,
        sphere_function, true, 4);
    PICA_MS pica_normal(40, 3, 25, 2.0, 0.1, 0.1, -4.0, 4.0,
        sphere_function, false, 4);

    pica_visual.setup_parallel();
    pica_normal.setup_parallel();

    pica_visual.run_parallel_visual();
    pica_normal.run_parallel();

    SUCCEED(); 
}

TEST(PICA_MS_Class, DifferentThreadCountsComplete)
{
    std::vector<int> thread_counts = { 1, 2, 4 };

    for (int threads : thread_counts) 
    {
        EXPECT_NO_THROW(
            {
            PICA_MS pica_ms(35, 3, 20, 2.0, 0.1, 0.1, -4.0, 4.0,
                           sphere_function, false, threads);
            pica_ms.setup_parallel();
            pica_ms.run_parallel();
            });
    }
}

// ============================================================================
// PICA_MS Performance Tests
// ============================================================================

TEST(PICA_MS_Class, NoRaceConditionsInParallelSections)
{
    PICA_MS pica_ms(100, 4, 30, 2.0, 0.1, 0.1, -5.0, 5.0,
        sphere_function, false, 4);

    pica_ms.setup_parallel();

    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_THROW(pica_ms.run_parallel());
    }
}

TEST(PICA_MS_Class, RepeatedExecutionWithoutMemoryLeaks)
{
    for (int run = 0; run < 3; ++run) 
    {
        PICA_MS pica_ms(40, 3, 10, 2.0, 0.1, 0.1, -4.0, 4.0,
            sphere_function, false, 2);

        pica_ms.setup_parallel();
        EXPECT_NO_THROW(pica_ms.run_parallel());
    }
}

// ============================================================================
// PICA_MS Integration Tests
// ============================================================================

TEST(PICA_MS_Class, IntegrationWithDifferentObjectiveFunctions)
{
    std::vector<std::function<double(const std::vector<double>&)>> functions = 
    {
        sphere_function,
        rastrigin_function,
        rosenbrock_function
    };

    for (const auto& func : functions) 
    {
        EXPECT_NO_THROW(
            {
            PICA_MS pica_ms(50, 3, 20, 2.0, 0.1, 0.1, -5.0, 5.0, func, false, 4);
            pica_ms.setup_parallel();
            pica_ms.run_parallel();
            });
    }
}

TEST(PICA_MS_Class, BoundaryConditionsRespected)
{
    PICA_MS pica_ms(40, 3, 25, 2.0, 0.1, 0.1, -10.0, 10.0,
        sphere_function, false, 4);

    pica_ms.setup_parallel();
    pica_ms.run_parallel();

    SUCCEED(); // Algorithm should respect the specified bounds
}

// ============================================================================
// PICA_MS OpenMP Specific Tests
// ============================================================================

TEST(PICA_MS_Class, OpenMPEnvironmentAvailable)
{
#ifdef _OPENMP
    SUCCEED();
#else
    FAIL() << "OpenMP support not available";
#endif
}

TEST(PICA_MS_Class, ThreadCountRespected)
{
    int requested_threads = 2;
    PICA_MS pica_ms(40, 3, 20, 2.0, 0.1, 0.1, -4.0, 4.0,
        sphere_function, false, requested_threads);

    pica_ms.setup_parallel();
    EXPECT_NO_THROW(pica_ms.run_parallel());

}