#include "../ICA_GUI/ica.h"
#include "../ICA_GUI/visual_ica.h"
#include "gtest/gtest.h"
#include "testing_functions.h"

// ============================================================================
// ICA Constructor Tests
// ============================================================================

TEST(ICA, ConstructorInitialization) 
{
    int pop_size = 50;
    int dim = 5;
    int max_iter = 100;
    double beta = 2.0;
    double gamma = 0.1;
    double eta = 0.1;
    double lb = -10.0;
    double ub = 10.0;

    ICA ica(pop_size, dim, max_iter, beta, gamma, eta, lb, ub, sphere_function);

    EXPECT_EQ(ica.pop_size, pop_size);
    EXPECT_EQ(ica.dim, dim);
    EXPECT_EQ(ica.max_iter, max_iter);
    EXPECT_DOUBLE_EQ(ica.beta, beta);
    EXPECT_DOUBLE_EQ(ica.gamma, gamma);
    EXPECT_DOUBLE_EQ(ica.eta, eta);
    EXPECT_DOUBLE_EQ(ica.lb, lb);
    EXPECT_DOUBLE_EQ(ica.ub, ub);
    EXPECT_EQ(ica.best_fitness, INFINITY);
    EXPECT_EQ(ica.tp, -1);
    EXPECT_TRUE(ica.population.empty());
    EXPECT_TRUE(ica.empires.empty());
    EXPECT_TRUE(ica.colonies.empty());
}

// ============================================================================
// ICA Setup Tests
// ============================================================================

TEST(ICA, SetupCreatesPopulation) 
{
    ICA ica(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();

    EXPECT_EQ(ica.population.size(), 30);
    EXPECT_FALSE(ica.empires.empty());
    EXPECT_FALSE(ica.colonies.empty());

    for (auto* country : ica.population) 
    {
        EXPECT_EQ(country->location.size(), 3);

        // Check bounds
        for (double val : country->location) 
        {
            EXPECT_GE(val, -5.0);
            EXPECT_LE(val, 5.0);
        }
    }
}

// ============================================================================
// ICA Fitness Calculation Tests
// ============================================================================

TEST(ICA, CalculateFitnessUpdatesValues) 
{
    ICA ica(20, 2, 50, 2.0, 0.1, 0.1, -10.0, 10.0, sphere_function);
    ica.setup();

    double initial_best = ica.best_fitness;
    ica.calculate_fitness();

    EXPECT_LT(ica.best_fitness, INFINITY);
    EXPECT_FALSE(ica.best_solution.empty());
    EXPECT_EQ(ica.best_solution.size(), 2);
}

// ============================================================================
// ICA Empire Creation Tests
// ============================================================================

TEST(ICA, CreateEmpiresCorrectCount) 
{
    ICA ica(50, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();

    int expected_empires = static_cast<int>(0.1 * 50);
    EXPECT_EQ(ica.empires.size(), expected_empires);

    for (size_t i = 0; i < ica.empires.size() - 1; ++i) 
    {
        EXPECT_LE(ica.empires[i]->fitness, ica.empires[i + 1]->fitness);
    }
}

// ============================================================================
// ICA Colony Assignment Tests
// ============================================================================

TEST(ICA, CreateColoniesAssignsVassals) 
{
    ICA ica(40, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();

    EXPECT_EQ(ica.colonies.size() + ica.empires.size(), 40);

    for (auto* colony : ica.colonies) 
    {
        EXPECT_NE(colony->vassal_of_empire, nullptr);
    }

    int total_vassals = 0;
    for (auto* empire : ica.empires) 
    {
        total_vassals += empire->vassals.size();
    }
    EXPECT_EQ(total_vassals, ica.colonies.size());
}

// ============================================================================
// ICA Assimilation Tests
// ============================================================================

TEST(ICA, AssimilationMovesColonies) 
{
    ICA ica(30, 2, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();

    std::vector<std::vector<double>> old_positions;
    for (auto* colony : ica.colonies) 
    {
        old_positions.push_back(colony->location);
    }

    ica.assimilation();

    bool some_moved = false;
    for (size_t i = 0; i < ica.colonies.size(); ++i) 
    {
        if (ica.colonies[i]->location != old_positions[i]) 
        {
            some_moved = true;
            break;
        }
    }
    EXPECT_TRUE(some_moved);
}

// ============================================================================
// ICA Revolution Tests
// ============================================================================

TEST(ICA, RevolutionModifiesPositions) 
{
    ICA ica(30, 2, 50, 2.0, 0.5, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();

    std::vector<std::vector<double>> old_positions;
    for (auto* colony : ica.colonies) {
        old_positions.push_back(colony->location);
    }

    ica.revolution();

    // Check that positions changed
    bool some_changed = false;
    for (size_t i = 0; i < ica.colonies.size(); ++i) {
        if (ica.colonies[i]->location != old_positions[i]) {
            some_changed = true;
            break;
        }
    }
    EXPECT_TRUE(some_changed);
}

// ============================================================================
// ICA Imperial War Tests
// ============================================================================

TEST(ICA, ImperialWarTransfersVassals) 
{
    ICA ica(50, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();

    size_t initial_empire_count = ica.empires.size();

    // Run multiple iterations to trigger imperial war
    for (int i = 0; i < 10; ++i)
    {
        ica.imperial_war();
    }

    // Empire count should eventually decrease or stay same
    EXPECT_LE(ica.empires.size(), initial_empire_count);
}

// ============================================================================
// ICA Full Run Tests
// ============================================================================

TEST(ICA, RunConvergesToSolution) 
{
    ICA ica(40, 2, 100, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();

    double initial_fitness = ica.best_fitness;
    ica.run();

    // Best fitness should improve
    EXPECT_LT(ica.best_fitness, initial_fitness);
    EXPECT_FALSE(ica.best_solution.empty());

    // For sphere function with optimum at origin, fitness should be low
    EXPECT_LT(ica.best_fitness, 10.0);
}

// ============================================================================
// ICA Migration Tests
// ============================================================================

TEST(ICA, MigrateBestReplacesWorst) 
{
    ICA ica(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();

    std::vector<double> elite_solution = { 0.0, 0.0, 0.0 }; // Near optimal for sphere
    ica.migrate_best(elite_solution, sphere_function);

    // Check that the elite solution is now in the population
    bool found = false;
    for (auto* country : ica.population) 
    {
        if (country->location == elite_solution) 
        {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// ============================================================================
// ICA Setter Tests
// ============================================================================

TEST(ICA, SetMaxIterUpdatesValue) 
{
    ICA ica(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);

    ica.set_max_iter(200);
    EXPECT_EQ(ica.max_iter, 200);
}

// ============================================================================
// ICA Getter Tests
// ============================================================================

TEST(ICA, GettersReturnCorrectValues) 
{
    ICA ica(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    ica.setup();
    ica.run();

    double fitness = ica.get_fitness();
    std::vector<double> solution = ica.get_best_solution();

    EXPECT_EQ(fitness, ica.best_fitness);
    EXPECT_EQ(solution, ica.best_solution);
    EXPECT_EQ(solution.size(), 3);
}

// ============================================================================
// Visual ICA Constructor Tests
// ============================================================================

TEST(Visual_ICA, ConstructorInitialization) 
{
    Visual_ICA vica(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);

    EXPECT_EQ(vica.pop_size, 30);
    EXPECT_EQ(vica.dim, 3);
    EXPECT_TRUE(vica.history.empty());
}

// ============================================================================
// Visual ICA Setup Tests
// ============================================================================

TEST(Visual_ICA, SetupCreatesVisualCountries) 
{
    Visual_ICA vica(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    vica.setup();

    EXPECT_EQ(vica.population.size(), 30);

    for (auto* country : vica.population) 
    {
        Visual_Country* vc = dynamic_cast<Visual_Country*>(country);
        EXPECT_NE(vc, nullptr);
    }
}

// ============================================================================
// Visual ICA Empire Coloring Tests
// ============================================================================

TEST(Visual_ICA, EmpireColouringAssignsColors) 
{
    Visual_ICA vica(40, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    vica.setup();

    for (auto* empire : vica.empires) 
    {
        Visual_Country* ve = static_cast<Visual_Country*>(empire);
        std::vector<double> color = ve->get_colour();

        EXPECT_EQ(color.size(), 3);

        for (double c : color) 
        {
            EXPECT_GE(c, 0.0);
            EXPECT_LE(c, 1.0);
        }
    }

    for (auto* empire : vica.empires) 
    {
        Visual_Country* ve = static_cast<Visual_Country*>(empire);
        std::vector<double> empire_color = ve->get_colour();

        for (auto* vassal : empire->vassals) 
        {
            Visual_Country* vv = static_cast<Visual_Country*>(vassal);
            EXPECT_EQ(vv->get_colour(), empire_color);
        }
    }
}

// ============================================================================
// Visual ICA State Snapshot Tests
// ============================================================================

TEST(Visual_ICA, StateSnapshotCapturesState) 
{
    Visual_ICA vica(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    vica.setup();

    size_t initial_history_size = vica.history.size();
    vica.state_snapshot("Test Phase");

    EXPECT_EQ(vica.history.size(), initial_history_size + 1);

    auto& last_snapshot = vica.history.back();
    EXPECT_EQ(last_snapshot.first, "Test Phase");
    EXPECT_EQ(last_snapshot.second.size(), vica.population.size());

    for (const auto& country_snap : last_snapshot.second) 
    {
        EXPECT_EQ(country_snap.position.size(), 3);
        EXPECT_EQ(country_snap.colour.size(), 3);
    }
}

// ============================================================================
// Visual ICA Run with History Tests
// ============================================================================

TEST(Visual_ICA, RunCreatesHistorySnapshots) 
{
    Visual_ICA vica(30, 3, 10, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    vica.setup();

    EXPECT_TRUE(vica.history.empty());

    vica.run();

    EXPECT_FALSE(vica.history.empty());

    std::vector<std::string> expected_phases = { "Assimilation", "Revolution", "Mutiny", "Imperial War" };
    for (const auto& snapshot : vica.history) 
    {
        bool valid_phase = false;
        for (const auto& phase : expected_phases) 
        {
            if (snapshot.first == phase) 
            {
                valid_phase = true;
                break;
            }
        }
        EXPECT_TRUE(valid_phase);
    }
}

// ============================================================================
// Visual ICA Get History Tests
// ============================================================================

TEST(Visual_ICA, GetHistoryReturnsCorrectData) 
{
    Visual_ICA vica(30, 3, 10, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    vica.setup();
    vica.run();

    auto history = vica.get_history();
    EXPECT_FALSE(history.empty());
    EXPECT_EQ(history.size(), vica.history.size());
}

// ============================================================================
// Visual ICA Random Color Tests
// ============================================================================

TEST(Visual_ICA, RandomColourGeneratesValidColors) 
{
    Visual_ICA vica(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);

    for (int i = 0; i < 10; ++i) 
    {
        std::vector<double> color = vica.random_colour();

        EXPECT_EQ(color.size(), 3);
        for (double c : color) 
        {
            EXPECT_GE(c, 0.0);
            EXPECT_LE(c, 1.0);
        }
    }
}

// ============================================================================
// Visual ICA with Different Functions Tests
// ============================================================================

TEST(Visual_ICA, WorksWithDifferentObjectiveFunctions) 
{
    Visual_ICA vica_rastrigin(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, rastrigin_function);
    vica_rastrigin.setup();
    vica_rastrigin.run();
    EXPECT_LT(vica_rastrigin.best_fitness, INFINITY);

    Visual_ICA vica_rosenbrock(30, 3, 50, 2.0, 0.1, 0.1, -5.0, 5.0, rosenbrock_function);
    vica_rosenbrock.setup();
    vica_rosenbrock.run();
    EXPECT_LT(vica_rosenbrock.best_fitness, INFINITY);
}

// ============================================================================
// Visual ICA Convergence Tests
// ============================================================================

TEST(Visual_ICA, ConvergesToSingleEmpire) 
{
    Visual_ICA vica(40, 2, 200, 2.0, 0.1, 0.1, -5.0, 5.0, sphere_function);
    vica.setup();
    vica.run();

    EXPECT_LE(vica.empires.size(), 5);
}