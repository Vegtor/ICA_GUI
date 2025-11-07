#ifndef	VISUAL_COUNTRY_H
#define VISUAL_COUNTRY_H

#include "country.h"

class Visual_Country : public Country
{
	std::vector<double> colour;

public:
	Visual_Country(const std::vector<double>& loc);
	void set_colour(std::vector<double>& colour);
	std::vector<double> get_colour();

	void add_vassal(Country* vassal) override;
	void add_emperor(Country* emperor) override;
	void coup(Country* nearest_imperialist) override;

	~Visual_Country() = default;
};

#endif