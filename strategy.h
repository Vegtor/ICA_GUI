#ifndef ISTRATEGY_H
#define ISTRATEGY_H

#include <vector>
#include <functional>

class Imperialist_Competitive_Algorithm;

class IStrategy
{
protected:
	Imperialist_Competitive_Algorithm* ica;
	std::function<double(const std::vector<double>&)> obj_func;
public:
	virtual ~IStrategy() = default;
	virtual void run() = 0;
};


#endif // ISTRATEGY_H