#pragma once

#include <cassert>
#include <map>
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <string>
#include <vector>
#include "csv.hpp"

using nlohmann::json;

std::string multiply_string(const std::string& string, int times);

struct Constraints {
    struct Teacher {
        std::string name;
        std::vector<std::vector<int>> unavailability;
    };
    struct Subject {
        std::string name;
        int difficulty;
    };
    using SubjectId = int;
    using TeacherId = int;
    struct Class {
        std::string name;
        std::map<SubjectId, std::vector<TeacherId>> teachers;
        std::map<SubjectId, int> course;
        int max_periods;
        int days;

        Class(const std::string& name);
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
