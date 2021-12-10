#include "history.hpp"

history::history() : data(new data_t) {}

void history::clear() {
	data->io.str("");
	data->io.clear();
}

std::string history::next() {
	std::string line;
	if (std::getline(data->io, line)) return line;
	return "";
}
