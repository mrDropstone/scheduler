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
Constraints::Class::Class(const std::string& name, const std::map<Subject, std::vector<Teacher>>& teachers) : name(name), teachers(teachers) {}

json Constraints::to_json() const {
    json result;
    result = json::object();
    result["subjects"] = subjects;
    result["teachers"] = teachers;
    result["classes"] = json::object();
    auto& json_classes_ref = result["classes"];
    for (auto& school_class : classes) {
        auto& json_school_class_ref = json_classes_ref[school_class.name];
        json_school_class_ref["teachers"] = school_class.teachers;
        json_school_class_ref["course"] = school_class.course;
    }
    return result;
}


Constraints extract_constraints(const json& data) {
    Constraints constraints;

    assert(data.contains("teachers"));
    for (auto& teacher : data["teachers"]) {
        constraints.teachers.emplace_back(teacher.get<std::string>());
    }

    assert(data.contains("subjects"));
    for (auto& subject : data["subjects"]) {
        constraints.subjects.emplace_back(subject.get<std::string>());
    }

    assert(data.contains("classes"));
    for (const auto& [class_name, config] : data["classes"].items()) {
        assert(config.contains("teachers"));
        assert(config.contains("course"));
        constraints.classes.emplace_back(Constraints::Class(class_name));
        for (const auto& [subject, teachers] : config["teachers"].items()) {
            constraints.classes.back().teachers.emplace(subject,  teachers.get<std::vector<std::string>>());
        }
        for (const auto& [subject, load] : config["course"].items()) {
            constraints.classes.back().course.emplace(subject, load);
        }
    }
    return constraints;
}


CSV Schedule::to_csv() const {
    CSV csv;
    csv.resize( 5 + 1, 10 * classes.size());
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
Schedule build_schedule(const Constraints& constraints) {
    int max_periods = 7;
    int max_days = 5;

    using SlotTable = std::vector<std::vector<sat::BoolVar>>;

    sat::CpModelBuilder cp_model;
    std::vector<std::vector<SlotTable>> lesson_timeslot_tables;
    std::vector<sat::BoolVar> soft_constraints;
    std::map<std::string, std::vector<std::vector<std::vector<sat::BoolVar>>>> teacher_booking;
    for (auto& teacher : constraints.teachers) {
        teacher_booking.emplace(teacher, std::vector<std::vector<std::vector<sat::BoolVar>>>(max_days, std::vector<std::vector<sat::BoolVar>>(max_periods)));
    }
    for (auto& school_class : constraints.classes) {
        lesson_timeslot_tables.emplace_back();

        int all_lessons = 0;
        for (auto& [subject, load] : school_class.course) {
            all_lessons += load;
        }
        int periods_mean = all_lessons / max_periods;
        
        // Make lesson_timeslot_tables
        for (auto& subject : school_class.course) {
            for (int i = 0; i < subject.second; i++) {
                std::vector<sat::BoolVar> exactly_one; // For ensuring that only one is true
                SlotTable week;
                for (int day_index = 0; day_index < max_days; day_index++) {
                    std::vector<sat::BoolVar> day;
                    for (int period_index = 0; period_index < max_periods; period_index++) {
                        auto lesson_var = cp_model.NewBoolVar();
                        day.emplace_back(lesson_var);

                        auto& teachers = school_class.teachers.at(subject.first);
                        for (auto& teacher : teachers) {
                            teacher_booking.at(teacher).at(day_index).at(period_index).emplace_back(lesson_var);
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
        for (int day_index = 0; day_index < max_days; day_index++) {
            for (int period_index = 0; period_index < max_periods; period_index++) {
                std::vector<sat::BoolVar> at_most_one_lesson; // To prevent two classes having two lesson_timeslot_tables at the same time.
                for (auto& subject : lesson_timeslot_tables.back()) {
                    at_most_one_lesson.emplace_back(subject.at(day_index).at(period_index));
                }
                cp_model.AddAtMostOne(at_most_one_lesson);
            }
        }

        // Ensure no gaps between lessons
        for (int day_index = 0; day_index < max_days; day_index++) {
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
        for (int day_index = 0; day_index < max_days; day_index++) {
            std::vector<sat::BoolVar> first_period;
            for (auto& subject : lesson_timeslot_tables.back()) {
                first_period.emplace_back(subject.at(day_index).front());
            }
            cp_model.AddExactlyOne(first_period);
        }
    }
    // Prevent teacher double-booking
    for (auto& [teacher_name, booking] : teacher_booking) {
        for (int day_index = 0; day_index < max_days; day_index++) {
            for (int period_index = 0; period_index < max_periods; period_index++) {
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
        return Schedule();
    } else {
        std::cout << "Other" << "\n";
        return Schedule();
    }


    Schedule schedule;
    int class_id = 0;
    for (auto& school_class : constraints.classes) {
        schedule.classes.emplace(school_class.name, std::vector<std::vector<std::string>>(max_days, std::vector<std::string>(max_periods, "")));
        int subject_id = 0;
        for (auto& subject : school_class.course) {
            for (int i = 0; i < subject.second; i++) {
                for (int day_index = 0; day_index < max_days; day_index++) {
                    bool should_break = false;
                    for (int period_index = 0; period_index < max_periods; period_index++) {
                        if (sat::SolutionBooleanValue(response, lesson_timeslot_tables.at(class_id).at(subject_id).at(day_index).at(period_index))) {
                            schedule.classes.at(school_class.name).at(day_index).at(period_index) = subject.first;
                            should_break = true;
                            break;
                        }
                    }
                    if (should_break) {
                        break;
                    }
                }
                subject_id++;
            }
        }
        class_id++;
    }
    return schedule;
}
