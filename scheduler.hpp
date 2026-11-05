#pragma once

#include <cassert>
#include <map>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>
#include "or-tools/include/ortools/sat/cp_model.h"
#include "or-tools/include/ortools/sat/cp_model_solver.h"
#include "or-tools/include/ortools/util/sorted_interval_list.h"
#include "csv.hpp"

using nlohmann::json;

std::string multiply_string(const std::string& string, int times);

struct Constraints {
    // The reason why I avoided using std::shared_ptr here is because I have autism and not sure if this will matter too much so I might as well just leave it like this for now and refactor if this has a serious effect on performance
    using Subject = std::string;
    using Teacher = std::string;
    struct Class {
        std::string name;
        std::map<Subject, std::vector<Teacher>> teachers;
        std::map<Subject, int> course;

        Class(const std::string& name);
        Class(const std::string& name, const std::map<Subject, std::vector<Teacher>>& teachers);
    };

    std::vector<Subject> subjects;
    std::vector<Teacher> teachers;
    std::vector<Class> classes;

    json to_json() const;
};


Constraints extract_constraints(const json& data);

struct Timetable {
    using Subject = std::string;
    using Teacher = std::string;
    using Class = std::string;

    std::map<Class, std::vector<std::vector<std::string>>> classes;

    CSV to_csv() const;
};


Timetable build_timetable(const Constraints& constraints);
