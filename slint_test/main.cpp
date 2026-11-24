#include "main.h"

int main() {
    auto ui = MainWindow::create();
    const auto& actions = ui->global<AppActions>();
    const auto& app_data = ui->global<AppData>();
    auto subjects_model = std::make_shared<slint::VectorModel<Subject>>();
    app_data.set_subjects(subjects_model);
    auto teachers_model = std::make_shared<slint::VectorModel<Teacher>>();
    app_data.set_teachers(teachers_model);

    actions.on_trigger([&ui, &subjects_model, &teachers_model](AppAction action) mutable {
        switch (action) {
            case AppAction::Constraints: 
                ui->set_page(0);
                break;
            case AppAction::Timetable: 
                ui->set_page(1);
                break;
            case AppAction::AddSubject: 
                subjects_model->push_back(Subject());
                break;
            case AppAction::AddTeacher: 
                teachers_model->push_back(Teacher());
                break;
        }
    });
    ui->run();
    return 0;
}
