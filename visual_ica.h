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

class Visual_ICA : public Imperialist_Competitive_Algorithm
{
private:
    std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> history;

    void state_snapshot(std::string phase_name);
    std::vector<double> random_colour();
    void empire_colouring();

public:
    using Imperialist_Competitive_Algorithm :: Imperialist_Competitive_Algorithm;
    void setup() override;
    void run() override;

    std::vector<std::pair<std::string, std::vector<Visual_Country_Snapshot>>> get_history();
};

#endif