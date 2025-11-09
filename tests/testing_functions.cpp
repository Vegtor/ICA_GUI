#include "testing_functions.h"


// ============================================================================
// Test Objective Functions
// ============================================================================

double sphere_function(const std::vector<double>& x)
{
    double sum = 0.0;
    for (double val : x) {
        sum += val * val;
    }
    return sum;
}

double rastrigin_function(const std::vector<double>& x)
{
    double A = 10.0;
    double result = A * x.size();
    for (double xi : x) {
        result += xi * xi - A * std::cos(2 * 3.14159 * xi);
    }
    return result;
}

double rosenbrock_function(const std::vector<double>& x)
{
    double sum = 0.0;
    for (size_t i = 0; i < x.size() - 1; ++i) {
        sum += 100.0 * std::pow(x[i + 1] - x[i] * x[i], 2) + std::pow(1 - x[i], 2);
    }
    return sum;
}
