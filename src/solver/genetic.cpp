#include "genetic.hpp"
#include <algorithm>
#include <iterator>
#include <numeric>

static std::random_device rd;
equation_processor::equation_processor(equation eq, size_t population_size,
                                       population::value_type min,
                                       population::value_type max)
    : eq_{std::move(eq)}, population_size_{population_size},
      random_engine_{rd()},
      value_distribution_{
          min, max} { //	std::numeric_limits<population::value_type>::max()
}

static auto score(const equation_processor::population &current,
                  const equation &eq) {
	std::vector<equation::error_type> scores;
	scores.reserve(current.data.size());
	std::transform(current.data.begin(), current.data.end(),
	               std::back_inserter(scores),
	               [&eq](const auto &line) { return eq.loss(line.begin()); });
	return scores;
}
template <typename It1, typename It2, typename It3>
void equation_processor::crossover(It1 a, It2 b, It3 dist) {
	std::uniform_int_distribution<int> position_distribution{0, eq_.num_vars()};
	auto m = position_distribution(random_engine_);
	auto mi = std::copy(a, a + m, dist);
	std::copy(b + m, b + eq_.num_vars(), mi);
}
/*
6*x+3*y=1200 ;
13*x0+23*x1+19*x2=7717 ;
13*x0^2+23*x1^3=3938146378 ;
13*x0+23*x1+19*x2=78809 ;
59*x0+83*x1+19*x2+89*x3=98897 ;
577*x0+691*x1+461*x2+647*x3+617*x4+193*x5+359*x6+823*x7+769*x8+647*x9=856369 ;
 */
template <typename It> void equation_processor::mutation(It a, It b) {
	for (auto it = a; it != b; it++) {
		auto r = std::uniform_real_distribution<double>{0, 1}(random_engine_);
		if (r < 0.2)
			(*it)[std::uniform_int_distribution<size_t>{0, it->size() - 1}(
			    random_engine_)] = value_distribution_(random_engine_);
	}

	// for (auto &b : *it) {
	// 	auto r = std::uniform_real_distribution<double>{0, 1}(random_engine_);
	// 	if (r < 0.1)
	// 		b = value_distribution_(random_engine_);
	// 	if (r < 0.05) {
	// 		b++;
	// 	} else if (r < 0.1) {
	// 		if (b == 0)
	// 			b = value_distribution_(random_engine_);
	// 		else
	// 			b--;
	// 	} else if (r < 0.13)
	// 		b = value_distribution_(random_engine_);
	// }
}
// #include <iostream>
template <typename T, typename U> T fit(const U &number) {
	if constexpr (std::is_floating_point_v<T>)
		return static_cast<T>(number);
	else {
		if (number > std::numeric_limits<T>::max())
			return std::numeric_limits<T>::max();
		return static_cast<T>(number);
	}
}
// static void prune(equation_processor::population &current, size_t size,
//                   const equation &eq) {
// 	current.data.erase(std::unique(current.data.begin(), current.data.end()),
// 	                   current.data.end());
// 	auto scores = score(current, eq);
// 	std::vector<std::pair<equation::error_type, size_t>> temp;
// 	temp.reserve(current.data.size());
// 	for (size_t i = 0; i < current.data.size(); i++)
// 		temp.emplace_back(scores[i], i);
// 	auto m = size > temp.size() ? temp.end() : temp.begin() + size;
// 	std::nth_element(temp.begin(), m, temp.end());
// 	equation_processor::population::data_type result;
// 	result.reserve(size);
// 	std::transform(temp.begin(), m, std::back_inserter(result),
// 	               [&](auto &a) { return current.data[a.second]; });
// 	current.data = std::move(result);
// }
equation_processor::stat equation_processor::step(population &current) {
	// prune(current, population_size_, eq_);
	current.data.erase(std::unique(current.data.begin(), current.data.end()),
	                   current.data.end());
	auto scores = score(current, eq_);
	auto [minIt, maxIt] = std::minmax_element(scores.begin(), scores.end());
	// std::cout << *minIt << std::endl;
	stat result = {fit<long long>(*minIt)};
	result.values = current.data[std::distance(scores.begin(), minIt)];
	result.max = {fit<long long>(*maxIt)};
	// for (auto &a : result.values)
	// 	std::cout << a << ' ';
	// std::cout << std::endl;
	std::vector<long long> acumulator;
	acumulator.reserve(scores.size());
	{
		auto s =
		    std::accumulate(scores.begin(), scores.end(), equation::error_type{0});
		std::transform(scores.begin(), scores.end(), std::back_inserter(acumulator),
		               [&s, m = *std::max_element(scores.begin(), scores.end())](
		                   const equation::error_type &a) {
			               auto factor = m;
			               // if (factor < 1000000)
			               //     factor = 1000000;
			               return static_cast<int>((factor - a) *
			                                       std::numeric_limits<int>::max() / factor);
		               });
		result.mean = fit<long long>(s / scores.size());
	}
	std::partial_sum(acumulator.begin(), acumulator.end(), acumulator.begin());
	auto prev_data = current.data;
	// auto m = prev_data.begin() + population_size_ / 2;
	// std::nth_element(prev_data.begin(), m, prev_data.end());
	current.data = {};
	// current.data.assign(prev_data.begin(), m);
	if (acumulator.back() == 0)
		acumulator.back() = 1;
	std::uniform_int_distribution<long long> selection_distribution{
	    0, acumulator.back() - 1};
	// for (size_t i = 0; i < population_size_; i++) {
	// 	auto l =
	// 	    std::distance(acumulator.begin(),
	// 	                  std::upper_bound(acumulator.begin(), acumulator.end(),
	// 	                                   selection_distribution(random_engine_)));
	// 	current.data.push_back(prev_data[l]);
	// }
	for (int i = 0; i < 2; i++)
		for (size_t i = 0; i < population_size_; i++) {
			auto l =
			    std::distance(acumulator.begin(),
			                  std::upper_bound(acumulator.begin(), acumulator.end(),
			                                   selection_distribution(random_engine_)));
			auto r =
			    std::distance(acumulator.begin(),
			                  std::upper_bound(acumulator.begin(), acumulator.end(),
			                                   selection_distribution(random_engine_)));
			current.data.emplace_back();
			crossover(prev_data[l].begin(), prev_data[r].begin(),
			          std::back_inserter(current.data.back()));
		}
	mutation(current.data.begin(), current.data.end());
	return result;
}
equation_processor::population equation_processor::identity() {
	population::data_type data;
	data.reserve(population_size_);
	std::generate_n(std::back_inserter(data), population_size_, [this] {
		population::substit_type result;
		result.reserve(eq_.num_vars());
		std::generate_n(std::back_inserter(result), eq_.num_vars(),
		                [this] { return value_distribution_(random_engine_); });
		return result;
	});
	return {move(data)};
}
