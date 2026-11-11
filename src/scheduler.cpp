#include <algorithm>
#include <cassert>
#include <map>
#include <iostream>
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <string>
#include <vector>
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model_solver.h"
#include "csv.hpp"
#include "scheduler.hpp"

using nlohmann::json;

std::string multiply_string(const std::string& string, int times) {
    std::string result;
    for (int i = 0; i < times; i++) {
        result += string;
    }
    return result;
}


Constraints::Class::Class(const std::string& name) : name(name) {}

json Constraints::to_json() const {
    json result;
    result = json::object();
    result = std::map<std::string, std::map<std::string, std::vector<std::vector<int>>>>();
    auto& json_teachers_ref = result["teachers"];
    for (auto& teacher : teachers) {
        json_teachers_ref[teacher.name] = json::object();
        json_teachers_ref[teacher.name]["unavailability"] = teacher.unavailability;
    }
    result["classes"] = json::object();
    auto& json_classes_ref = result["classes"];
    for (auto& school_class : classes) {
        auto& json_school_class_ref = json_classes_ref[school_class.name];
        json_school_class_ref["max_periods"] = school_class.max_periods;
        json_school_class_ref["days"] = school_class.days;
        std::map<std::string, std::vector<std::string>> subject_teachers;
        for (auto& [subject_id, teacher_ids] : school_class.teachers) {
            std::vector<std::string> teacher_names;
            for (int teacher_id : teacher_ids) {
                teacher_names.emplace_back(teachers.at(teacher_id).name);
            }
            subject_teachers[subjects.at(subject_id).name] = teacher_names;
        }
        json_school_class_ref["teachers"] = subject_teachers;
        std::map<std::string, int> course;
        for (auto& [subject_id, lessons] : school_class.course) {
            course[subjects.at(subject_id).name] = lessons;
        }
        json_school_class_ref["course"] = course;
    }
    return result;
}


Constraints extract_constraints(const json& data) {
    Constraints constraints;

    assert(data.contains("teachers"));
    for (auto& [teacher, teacher_constraints] : data["teachers"].items()) {
        assert(teacher_constraints["unavailability"].is_array());
        std::vector<std::vector<int>> unavailability;
        for (auto& day : teacher_constraints["unavailability"]) {
            assert(day.is_array());
            unavailability.emplace_back(day.get<std::vector<int>>());
        }
        constraints.teachers.emplace_back(Constraints::Teacher(teacher, std::move(unavailability)));
    }

    assert(data.contains("difficulties"));
    for (auto& [subject, difficulty] : data["difficulties"].items()) {
        constraints.subjects.emplace_back(Constraints::Subject(subject, difficulty));
    }

    assert(data.contains("classes"));
    for (const auto& [class_name, config] : data["classes"].items()) {
        assert(config.contains("course"));
        assert(config.contains("teachers"));
        assert(config.contains("max_periods"));
        assert(config.contains("days"));
        assert(config["teachers"].is_object());
        constraints.classes.emplace_back(Constraints::Class(class_name));
        constraints.classes.back().max_periods = config["max_periods"];
        constraints.classes.back().days = config["days"];
        for (auto& [subject_name, teachers] : config["teachers"].items()) {
            Constraints::SubjectId subject_id = std::distance(constraints.subjects.begin(), std::find_if(constraints.subjects.begin(), constraints.subjects.end(), [subject_name](const Constraints::Subject& subject) {
                return subject_name == subject.name;
            }));
            std::vector<Constraints::TeacherId> teacher_ids;
            for (int i = 0; i < teachers.size(); i++) {
                auto teacher_name = teachers.at(i).get<std::string>();
                teacher_ids.emplace_back(std::distance(constraints.teachers.begin(), std::find_if(constraints.teachers.begin(), constraints.teachers.end(), [teacher_name](const Constraints::Teacher& teacher) {
                    return teacher.name == teacher_name;
                })));
            }
            constraints.classes.back().teachers.emplace(subject_id,  teacher_ids);
        }
        for (const auto& [subject_name, load] : config["course"].items()) {
            Constraints::SubjectId subject_id = std::distance(constraints.subjects.begin(), std::find_if(constraints.subjects.begin(), constraints.subjects.end(), [subject_name](const Constraints::Subject& subject) {
                return subject_name == subject.name;
            }));
            constraints.classes.back().course.emplace(subject_id, load);
        }
    }
    return constraints;
}


CSV Timetable::to_csv() const {
    CSV csv;
    csv.resize( 6 + 1, 10 * classes.size());
    int y = 0;
    for (auto& school_class : classes) {
        csv.at(0, y) = "Class " + school_class.first;
        for (int i = 0; i < school_class.second.size(); i++) {
            for (int j = 0; j < school_class.second.back().size(); j++) {
                csv.at(i + 1, y + j) = school_class.second.at(i).at(j);
            }
        }
        y += 10;
    }
    return csv;
}

namespace sat = operations_research::sat;


// This is the biggest nonsense I've ever written but it works for now
Timetable build_timetable(const Constraints& constraints) {
    int max_days = 6;
    int absolute_max_periods = 10;

    using SlotTable = std::vector<std::vector<sat::BoolVar>>;
    struct Lesson {
        SlotTable slot_table;
    };


    sat::CpModelBuilder cp_model;
    std::vector<std::vector<SlotTable>> lesson_timeslot_tables;
    std::vector<sat::BoolVar> soft_constraints;
    std::vector<std::vector<std::vector<std::vector<sat::BoolVar>>>> teacher_booking;
    for (auto& teacher : constraints.teachers) {
        teacher_booking.emplace_back(std::vector<std::vector<std::vector<sat::BoolVar>>>(max_days, std::vector<std::vector<sat::BoolVar>>(absolute_max_periods)));
    }
    for (auto& school_class : constraints.classes) {
        lesson_timeslot_tables.emplace_back();

        int all_lessons = 0;
        for (auto& [subject, load] : school_class.course) {
            all_lessons += load;
        }
        int max_periods = school_class.max_periods;
        int days = school_class.days;
        int periods_mean = all_lessons / max_periods;
        // Make lesson_timeslot_tables
        for (auto& [subject, count] : school_class.course) {
            for (int i = 0; i < count; i++) {
                std::vector<sat::BoolVar> exactly_one; // For ensuring that only one is true
                SlotTable week;
                for (int day_index = 0; day_index < days; day_index++) {
                    std::vector<sat::BoolVar> day;
                    for (int period_index = 0; period_index < max_periods; period_index++) {
                        auto& teacher_ids = school_class.teachers.at(subject);
                        bool is_unavailable = false;
                        for (auto& teacher_id : teacher_ids) {
                            auto& unavailability = constraints.teachers.at(teacher_id).unavailability.at(day_index);
                            if (std::find(unavailability.begin(), unavailability.end(), period_index) != unavailability.end()) {
                                is_unavailable = true;
                                break;
                            }
                        }
                        if (is_unavailable) {
                            day.emplace_back(cp_model.FalseVar());
                            exactly_one.emplace_back(day.back());
                            continue;
                        }

                        auto lesson_var = cp_model.NewBoolVar();
                        day.emplace_back(lesson_var);

                        for (auto& teacher_id : teacher_ids) {
                            teacher_booking.at(teacher_id).at(day_index).at(period_index).emplace_back(lesson_var);
                        }
                        exactly_one.emplace_back(day.back());
                    }
                    week.emplace_back(std::move(day));
                }
                lesson_timeslot_tables.back().emplace_back(std::move(week));
                // Ensure exactly one lesson per week
                cp_model.AddExactlyOne(exactly_one);
            }
        }
        // Prevent class double-booking
        for (int day_index = 0; day_index < days; day_index++) {
            for (int period_index = 0; period_index < max_periods; period_index++) {
                std::vector<sat::BoolVar> at_most_one_lesson; // To prevent two classes having two lesson_timeslot_tables at the same time.
                for (auto& subject : lesson_timeslot_tables.back()) {
                    at_most_one_lesson.emplace_back(subject.at(day_index).at(period_index));
                }
                cp_model.AddAtMostOne(at_most_one_lesson);
            }
        }

        // Ensure no gaps between lessons
        for (int day_index = 0; day_index < days; day_index++) {
            std::vector<sat::BoolVar> lesson_map;
            for (int i = 0; i < max_periods; i++) {
                lesson_map.emplace_back(cp_model.NewBoolVar());
            }
            for (int period_index = 0; period_index < max_periods; period_index++) {
                std::vector<sat::BoolVar> slot_vars;
                for (auto& subject : lesson_timeslot_tables.back()) {
                    slot_vars.emplace_back(subject.at(day_index).at(period_index));
                }
                cp_model.AddEquality(lesson_map.at(period_index), sat::LinearExpr::Sum(slot_vars));
                if (period_index >= periods_mean) {
                    for (int i = 0; i < period_index + periods_mean + 1; i++) {
                        soft_constraints.emplace_back(lesson_map.at(period_index));
                    }
                }
            }
            for (int period_index = max_periods - 1; period_index > 0; period_index--) {
                cp_model.AddImplication(lesson_map.at(period_index), lesson_map.at(period_index - 1));
            }
        }

        // Ensure first lesson is always present
        for (int day_index = 0; day_index < days; day_index++) {
            std::vector<sat::BoolVar> first_period;
            for (auto& subject : lesson_timeslot_tables.back()) {
                first_period.emplace_back(subject.at(day_index).front());
            }
            cp_model.AddExactlyOne(first_period);
        }
    }
    // Prevent teacher double-booking
    for (auto& booking : teacher_booking) {
        for (int day_index = 0; day_index < max_days; day_index++) {
            for (int period_index = 0; period_index < absolute_max_periods; period_index++) {
                cp_model.AddAtMostOne(booking.at(day_index).at(period_index));
            }
        }
    }


    cp_model.Minimize(sat::LinearExpr::Sum(soft_constraints));
    auto response = sat::Solve(cp_model.Build());
    if (response.status() == sat::CpSolverStatus::OPTIMAL) {
        std::cout << "Optimal" << "\n";
    } else if (response.status() == sat::CpSolverStatus::FEASIBLE) {
        std::cout << "Feasible" << "\n";
    } else if (response.status() == sat::CpSolverStatus::INFEASIBLE) {
        std::cout << "Infeasible" << "\n";
        return Timetable();
    } else {
        std::cout << "Other" << "\n";
        return Timetable();
    }


    Timetable timetable;
    int class_id = 0;
    for (auto& school_class : constraints.classes) {
        int max_periods = school_class.max_periods;
        int days = school_class.days;
        timetable.classes.emplace(school_class.name, std::vector<std::vector<std::string>>(days, std::vector<std::string>(max_periods, "")));
        int lesson_index = 0;
        for (auto& [subject_id, count] : school_class.course) {
            for (int i = 0; i < count; i++) {

                for (int day_index = 0; day_index < days; day_index++) {
                    bool found_timeslot = false;

                    for (int period_index = 0; period_index < max_periods; period_index++) {
                        if (sat::SolutionBooleanValue(response, lesson_timeslot_tables.at(class_id).at(lesson_index).at(day_index).at(period_index))) {
                            timetable.classes.at(school_class.name).at(day_index).at(period_index) = constraints.subjects.at(subject_id).name;
                            found_timeslot = true;
                            break;
                        }
                    }
                    if (found_timeslot) {
                        break;
                    }
                }
                lesson_index++;
            }
        }
        class_id++;
    }
    return timetable;
}

