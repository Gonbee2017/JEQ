#ifndef HISTORY_HPP
#define HISTORY_HPP

#include <memory>
#include <sstream>
#include <string>

class history {
public:
	history();
	void clear();
	std::string next();
	template <typename X>
	history &operator <<(X &&x);
protected:
	struct data_t {
		std::stringstream io;
	};
	std::shared_ptr<data_t> data;
};

template <typename X>
history &history::operator <<(X &&x) {
	data->io << x;
	return *this;
}

#endif // #ifndef HISTORY_HPP
