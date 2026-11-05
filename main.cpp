#include "scheduler.hpp"
#include <fstream>
#include <iostream>

int main() {
    std::ifstream source("bench/10.json");
    assert(source.is_open());
    json data = json::parse(source);
    auto constraints = extract_constraints(data);
    auto schedule = build_timetable(constraints);
    std::ofstream output("result.csv");
    output << schedule.to_csv().dump();
    std::cout << schedule.to_csv().dump();
    return 0;
}
