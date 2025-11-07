#include "visual_country.h"

Visual_Country::Visual_Country(const std::vector<double>& loc):
	Country(loc), colour({-1.0, -1.0, -1,0})
{}

void Visual_Country::set_colour(std::vector<double>& colour)
{
	this->colour = colour;
}

std::vector<double> Visual_Country::get_colour()
{
	return this->colour;
}

void Visual_Country::add_vassal(Country* vassal)
{
	vassals.push_back(vassal);
	if (auto vc = dynamic_cast<Visual_Country*>(vassal)) 
		vc->set_colour(this->colour);
}

void Visual_Country::add_emperor(Country* emperor)
{
	this->add_emperor(emperor);
	if (auto vc = dynamic_cast<Visual_Country*>(emperor)) 
	{
		std::vector<double> temp = vc->get_colour();
		this->set_colour(temp);
	}
}

void Visual_Country::coup(Country* nearest_imperialist)
{
	if (auto vc = dynamic_cast<Visual_Country*>(nearest_imperialist))
	{
		std::vector<double> temp = vc->get_colour();
		this->set_colour(temp);
	}
	this->coup(nearest_imperialist);
}
