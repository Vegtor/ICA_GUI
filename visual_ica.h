#ifndef VISUAL_ICA_H
#define VISUAL_ICA_H

#include "ica.h"
#include "visual_country.h"
#include <string>

struct Visual_Country_Snapshot
{
    std::vector<double> position;
    std::vector<double> colour;
    bool is_emperor;
};

class Visual_ICA : public ICA
{
public:
    std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> history;

    void state_snapshot(std::string phase_name);
    std::vector<double> random_colour();
    void empire_colouring();

    Visual_ICA(int pop_size, int dim, int max_iter, double beta, double gamma, double eta, double lb, double ub, const std::function<double(const std::vector<double>&)>& obj_func);
    void setup() override;
    void run() override;

    std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> get_history();
};

#endif