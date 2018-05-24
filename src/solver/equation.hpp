#ifndef EQUATION_HPP
#define EQUATION_HPP

#include <algorithm>
#include <boost/multiprecision/cpp_int.hpp>
#include <exception>
#include <istream>
#include <map>
#include <numeric>
#include <vector>

class equation {
public:
	using index_type = unsigned short;
	using power_type = int;
	using numeric_type = boost::multiprecision::cpp_int;
	using name_type = std::string;
	using error_type = boost::multiprecision::cpp_int;
	struct operand {
		index_type id;
		numeric_type factor;
		power_type power;
	};
	struct named_operand {
		numeric_type factor;
		name_type name;
		power_type power;
	};

	equation() : raw_number_id_{var_id("")} {}
	equation(std::initializer_list<named_operand> l) : equation{} {
		data_.reserve(l.size());
		std::transform(l.begin(), l.end(), std::back_inserter(data_),
		               [this](const named_operand &op) { return unname(op); });
	}
	template <typename It> equation(It begin, It end) : equation{} {
		std::transform(begin, end, std::back_inserter(data_),
		               [this](const named_operand &op) { return unname(op); });
	}

	named_operand name(const operand &op) const {
		return {op.factor, var_names_[op.id], op.power};
	}
	operand unname(const named_operand &op) {
		return {var_id(op.name), op.factor, op.power};
	}
	template <typename It> error_type loss(It begin) const;
	auto &operands() const { return data_; }
	auto num_vars() const { return var_names_.size(); }

private:
	std::vector<std::string> var_names_;
	std::vector<operand> data_;
	index_type raw_number_id_;

	index_type var_id(const std::string &name) {
		auto i = std::find(var_names_.begin(), var_names_.end(), name);
		if (i == var_names_.end()) {
			var_names_.push_back(name);
			i = --var_names_.end();
		}
		return distance(var_names_.begin(), i);
	}
};

inline equation::error_type binpow(equation::error_type a,
                                   equation::error_type b) {
	equation::error_type result = 1;
	while (b > 0) {
		if (b & 1)
			result *= a;
		a *= a;
		b >>= 1;
	}
	return result;
}
template <typename It> equation::error_type equation::loss(It begin) const {
	// return 5;
	error_type result =
	    std::accumulate(data_.begin(), data_.end(), error_type{0},
	                    [begin](const error_type &a, const operand &b) {
		                    return a + b.factor * binpow(begin[b.id], b.power);
	                    });
	return abs(result);
}

inline std::ostream &operator<<(std::ostream &os,
                                const equation::named_operand &op) {
	os << op.factor;
	if (!op.name.empty())
		os << '*' << op.name << '^' << op.power;
	return os;
}
inline std::ostream &operator<<(std::ostream &os, const equation &eq) {
	if (eq.operands().empty()) {
		os << '0';
	} else {
		os << eq.name(eq.operands()[0]);
		std::for_each(eq.operands().begin() + 1, eq.operands().end(),
		              [&](auto &op) { os << '+' << eq.name(op); });
	}
	return os << "=0";
}

class idn_reader {
	std::string *name_;
	idn_reader(std::string &name) : name_(&name) {}
	friend idn_reader identifier(std::string &);
	friend std::istream &operator>>(std::istream &, const idn_reader &);
};
inline idn_reader identifier(std::string &name) { return idn_reader{name}; }
inline std::istream &operator>>(std::istream &is, const idn_reader &idn) {
	*idn.name_ = "";
	is >> std::ws;
	while (is && !is.eof() &&
	       (isalpha(is.peek()) || (idn.name_->size() > 0 && isalnum(is.peek()))))
		*idn.name_ += is.get();
	// if (idn.name_->size() == 0)
	// 	is.setstate(is.rdstate() | std::ios_base::failbit);
	return is;
}
inline std::istream &operator>>(std::istream &is, equation::named_operand &op) {
	is >> op.factor >> std::ws;
	if (is) {
		if (is.eof() || is.peek() != '*') {
			op.name = "";
			op.power = 0;
			return is;
		} else {
			is.get();
		}
	} else {
		is.clear(std::ios::goodbit);
		op.factor = 1;
	}
	is >> identifier(op.name) >> std::ws;
	if (!is.eof() && is && is.peek() == '^') {
		is.get();
		is >> op.power;
	} else
		op.power = 1;
	return is;
}

inline std::istream &operator>>(std::istream &is, equation &eq) {
	using std::move;
	std::vector<equation::named_operand> ops;
	equation::named_operand buf;
	is >> buf;
	ops.push_back(move(buf));
	if (!is)
		return is;
	is >> std::ws;
	while (!is.eof() && is.peek() == '+') {
		is.get();
		is >> buf;
		ops.push_back(move(buf));
		if (!is)
			return is;
	}
	is >> std::ws;
	if (is.eof() || is.get() != '=') {
		is.setstate(is.rdstate() | std::ios_base::failbit);
		return is;
	}
	is >> buf;
	buf.factor *= -1;
	ops.push_back(move(buf));
	if (!is)
		return is;
	eq = equation(ops.begin(), ops.end());
	return is;
}

#endif /* end of include guard: EQUATION_HPP */
