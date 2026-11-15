#include <algorithm>
#include <cassert>
#include <iterator>
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
        json_school_class_ref["undesired"] = school_class.undesired;
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

int Constraints::get_subject_id(const std::string& name) const {
    return std::distance(subjects.begin(), std::find_if(subjects.begin(), subjects.end(), [name](const Subject& subject) {
            return name == subject.name;
        }
    ));
}

// For debugging
Constraints extract_constraints(const json& data) {
    Constraints constraints;
    assert(data.contains("teachers"));
    assert(data.contains("difficulties"));
    assert(data.contains("classes"));

    for (auto& [teacher, teacher_constraints] : data["teachers"].items()) {
        assert(teacher_constraints["unavailability"].is_array());
        std::vector<std::vector<int>> unavailability;
        for (auto& day : teacher_constraints["unavailability"]) {
            assert(day.is_array());
            unavailability.emplace_back(day.get<std::vector<int>>());
        }
        constraints.teachers.emplace_back(Constraints::Teacher(teacher, std::move(unavailability)));
    }

    for (auto& [subject, difficulty] : data["difficulties"].items()) {
        constraints.subjects.emplace_back(Constraints::Subject(subject, difficulty));
    }

    for (const auto& [class_name, config] : data["classes"].items()) {
        assert(config.contains("course"));
        assert(config.contains("teachers"));
        assert(config.contains("max_periods"));
        assert(config.contains("days"));
        assert(config.contains("max_daily_difficulty"));
        assert(config.contains("undesired"));
        assert(config.contains("preferred"));

        assert(config["undesired"].is_array());
        assert(config["preferred"].is_array());
        assert(config["teachers"].is_object());

        constraints.classes.emplace_back(Constraints::Class(class_name));
        constraints.classes.back().max_periods = config["max_periods"];
        constraints.classes.back().days = config["days"];
        constraints.classes.back().max_daily_difficulty = config["max_daily_difficulty"];

        for (auto& day : config["undesired"]) {
            constraints.classes.back().undesired.emplace_back(std::vector<std::vector<int>>());
            for (auto& period : day) {
                std::vector<Constraints::SubjectId> subjects;
                for (auto& subject_name : period) {
                    Constraints::SubjectId subject_id = constraints.get_subject_id(subject_name);
                    subjects.emplace_back(subject_id);
                }
                constraints.classes.back().undesired.back().emplace_back(subjects);
            }
        }

        for (auto& day : config["preferred"]) {
            constraints.classes.back().preferred.emplace_back(std::vector<std::optional<int>>());
            assert(day.is_array());
            for (auto period : day) {
                if (period.is_string()) {
                    auto subject_name = period.get<std::string>();
                    Constraints::SubjectId subject_id = constraints.get_subject_id(subject_name);
                    constraints.classes.back().preferred.back().emplace_back(std::optional<int>(subject_id));
                } else if (period.is_null()) {
                    constraints.classes.back().preferred.back().emplace_back();
                } else {
                    assert("Fumble town!!");
                }
            }
        }

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
            Constraints::SubjectId subject_id = constraints.get_subject_id(subject_name);
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
    int max_days = 0;
    int max_periods = 0;
    for (auto& school_class : constraints.classes) {
        max_periods = std::max(max_periods, school_class.max_periods);
        max_days = std::max(max_days, school_class.days);
    }

    using TimeSlot = std::vector<std::vector<sat::BoolVar>>;
    struct Lesson {
        TimeSlot timeslot;
        std::vector<sat::BoolVar> day_table;
        int subject_id;
    };
    struct Teacher {
        std::vector<std::vector<std::vector<sat::BoolVar>>> booking;
    };

    sat::CpModelBuilder cp_model;
    std::vector<std::vector<Lesson>> lessons;
    std::vector<Teacher> teacher_booking;
    std::vector<sat::LinearExpr> soft_constraints;

    auto gen_timeslot = [&cp_model](int days, int periods) {
        std::vector<sat::BoolVar> exactly_one;
        TimeSlot timeslot;
        timeslot.reserve(days);
        for (int day = 0; day < days; day++) {
            timeslot.emplace_back();
            timeslot.back().reserve(periods);
            for (int period = 0; period < periods; period++) {
                auto var = cp_model.NewBoolVar();
                exactly_one.emplace_back(var);
                timeslot.back().emplace_back(var);
            }
        }
        return timeslot;
    };


    // Generate teacher timeslots
    for (auto& teacher : constraints.teachers) {
        teacher_booking.emplace_back(Teacher(std::vector<std::vector<std::vector<sat::BoolVar>>>(max_days, std::vector<std::vector<sat::BoolVar>>(max_periods, std::vector<sat::BoolVar>()))));
    }

    for (auto& school_class : constraints.classes) {
        lessons.emplace_back();

        int lessons_per_week = 0;
        for (auto& [subject_id, lessons] : school_class.course) {
            lessons_per_week += lessons;
        }
        //int max_periods = school_class.max_periods;
        int periods_mean = lessons_per_week / school_class.max_periods;
        // Make lessons
        for (auto& [subject_id, weekly_lessons] : school_class.course) {
            for (int i = 0; i < weekly_lessons; i++) {
                // Create slot table
                std::vector<sat::BoolVar> exactly_one;
                TimeSlot timeslot(school_class.days, std::vector<sat::BoolVar>());
                std::vector<sat::BoolVar> day_table;
                for (int day = 0; day < school_class.days; day++) {
                    for (int period = 0; period < school_class.max_periods; period++) {
                        auto& teacher_ids = school_class.teachers.at(subject_id);
                        bool is_unavailable = false;
                        for (auto& teacher_id : teacher_ids) {
                            auto& unavailability = constraints.teachers.at(teacher_id).unavailability.at(day);
                            if (std::find(unavailability.begin(), unavailability.end(), period) != unavailability.end()) {
                                is_unavailable = true;
                                break;
                            }
                        }
                        if (is_unavailable) {
                            timeslot.at(day).emplace_back(cp_model.FalseVar());
                            exactly_one.emplace_back(cp_model.FalseVar());
                            continue;
                        }
                        auto var = cp_model.NewBoolVar();
                        exactly_one.emplace_back(var);
                        timeslot.at(day).emplace_back(var);
                        for (auto& teacher_id : teacher_ids) {
                            teacher_booking.at(teacher_id).booking.at(day).at(period).emplace_back(var);
                        }
                        // Undesired
                        auto& undesired = school_class.undesired.at(day).at(period);
                        if (std::find(undesired.begin(), undesired.end(), subject_id) != undesired.end()) {
                            soft_constraints.emplace_back(50 * var);
                        }
                        // Preferred
                        auto preferred = school_class.preferred.at(day).at(period);
                        if (!preferred.has_value()) {
                            continue;
                        } else if (preferred.value() == subject_id) {
                            soft_constraints.emplace_back(-50 * var);
                        }
                    }
                    auto var = cp_model.NewBoolVar();
                    day_table.emplace_back(var);
                    cp_model.AddEquality(var, sat::LinearExpr::Sum(timeslot.back()));
                }
                cp_model.AddExactlyOne(exactly_one);
                lessons.back().emplace_back(Lesson(std::move(timeslot), std::move(day_table), subject_id));
            }
        }
        // Prevent class double-booking
        for (int day_index = 0; day_index < school_class.days; day_index++) {
            for (int period_index = 0; period_index < school_class.max_periods; period_index++) {
                std::vector<sat::BoolVar> at_most_one_lesson; // To prevent two classes having two lessons at the same time.
                for (auto& lesson : lessons.back()) {
                    at_most_one_lesson.emplace_back(lesson.timeslot.at(day_index).at(period_index));
                }
                cp_model.AddAtMostOne(at_most_one_lesson);
            }
        }

        // Prevent gaps between lessons for students
        for (int day_index = 0; day_index < school_class.days; day_index++) {
            std::vector<sat::BoolVar> lesson_map;
            for (int i = 0; i < school_class.max_periods; i++) {
                lesson_map.emplace_back(cp_model.NewBoolVar());
            }
            for (int period_index = 0; period_index < school_class.max_periods; period_index++) {
                std::vector<sat::BoolVar> slot_vars;
                for (auto& subject : lessons.back()) {
                    slot_vars.emplace_back(subject.timeslot.at(day_index).at(period_index));
                }
                cp_model.AddEquality(lesson_map.at(period_index), sat::LinearExpr::Sum(slot_vars));
                if (period_index >= periods_mean) {
                    for (int i = 0; i < period_index + periods_mean + 1; i++) {
                        soft_constraints.emplace_back(lesson_map.at(period_index));
                    }
                }
            }
            for (int period_index = school_class.max_periods - 1; period_index > 0; period_index--) {
                cp_model.AddImplication(lesson_map.at(period_index), lesson_map.at(period_index - 1));
            }
        }

        // Ensure first lesson is always present
        for (int day_index = 0; day_index < school_class.days; day_index++) {
            std::vector<sat::BoolVar> first_period;
            for (auto& lesson : lessons.back()) {
                first_period.emplace_back(lesson.timeslot.at(day_index).front());
            }
            cp_model.AddExactlyOne(first_period);
        }

        // max daily difficulty limit
        for (int day = 0; day < school_class.days; day++) {
            std::vector<sat::LinearExpr> difficulty;
            for (auto& lesson : lessons.back()) {
                difficulty.emplace_back(constraints.subjects.at(lesson.subject_id).difficulty * lesson.day_table.at(day));
            }
        }
    }
    // Prevent teacher double-booking
    for (auto& teacher : teacher_booking) {
        for (int day_index = 0; day_index < max_days; day_index++) {
            for (int period_index = 0; period_index < max_periods; period_index++) {
                cp_model.AddAtMostOne(teacher.booking.at(day_index).at(period_index));
            }
        }
    }

    sat::LinearExpr soft_constraints_sum = 0;
    for (auto& constraint : soft_constraints) {
        soft_constraints_sum += constraint;
    }
    cp_model.Minimize(soft_constraints_sum);
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

    std::cout << sat::SolutionIntegerValue(response, soft_constraints_sum);

    Timetable timetable;
    int class_id = 0;
    for (auto& school_class : constraints.classes) {
        int max_periods = school_class.max_periods;
        int days = school_class.days;
        timetable.classes.emplace(school_class.name, std::vector<std::vector<std::string>>(days, std::vector<std::string>(max_periods, "")));
        int lesson_index = 0;
        for (auto& [subject_id, weekly_lessons] : school_class.course) {
            for (int i = 0; i < weekly_lessons; i++) {

                for (int day_index = 0; day_index < days; day_index++) {
                    bool found_timeslot = false;

                    for (int period_index = 0; period_index < max_periods; period_index++) {
                        if (sat::SolutionBooleanValue(response, lessons.at(class_id).at(lesson_index).timeslot.at(day_index).at(period_index))) {
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

