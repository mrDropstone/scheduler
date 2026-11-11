#include "scheduler.hpp"
#include <fstream>
#include <iostream>

int main() {
    std::ifstream source("bench/50.json");
    assert(source.is_open());
    json data = json::parse(source);
    auto constraints = extract_constraints(data);
    auto timetable = build_timetable(constraints);
    std::cout << timetable.to_csv().dump() << std::endl;
    std::ofstream output("result.csv");
    output << timetable.to_csv().dump();
    return 0;
}
