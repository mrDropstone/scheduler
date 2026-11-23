#include "main.h"

int main() {
    auto ui = MainWindow::create();
    const auto& actions = ui->global<AppActions>();
    actions.on_trigger([&ui](AppAction action) mutable {
        switch (action) {
            case AppAction::Constraints: 
                ui->set_page(0);
                break;
            case AppAction::Timetable: 
                ui->set_page(1);
                break;
        }
    });
    ui->run();
    return 0;
}
