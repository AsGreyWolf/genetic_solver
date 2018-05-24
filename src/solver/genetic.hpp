#ifndef GENETIC_HPP
#define GENETIC_HPP

/**
 * SCORING
 * dataset -> accuracy
 * SELECTION
 * dataset, accuracy -> pairset
 * CROSSOVER
 * dataset, pairset -> dataset
 * MUTATION
 * dataset -> dataset
 */
#include "equation.hpp"
#include <random>
#include <vector>

class equation_processor {
public:
	struct population {
		using value_type = long long;
		using substit_type = std::vector<value_type>;
		using data_type = std::vector<substit_type>;
		data_type data;
	};
	struct stat {
		long long min;
		long long mean;
		long long max;
		population::substit_type values;
	};

	equation_processor(equation eq, size_t population_size,
	                   population::value_type min, population::value_type max);
	stat step(population &current);
	population identity();

private:
	template <typename It1, typename It2, typename It3>
	void crossover(It1 a, It2 b, It3 dist);
	template <typename It> void mutation(It a, It b);

	equation eq_;
	size_t population_size_;

	std::mt19937_64 random_engine_;
	std::uniform_int_distribution<population::value_type> value_distribution_;
};

#endif /* end of include guard: GENETIC_HPP */
