#include "gui/main_window.h"
#include <QApplication>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // set format before creating application to avoid platform specific problems
        QSurfaceFormat format;
        format.setSamples(4); // 4x MSAA
        QSurfaceFormat::setDefaultFormat(format);

        QApplication app(argc, argv);
        MainWindow main_window = MainWindow();
        main_window.show();

        return app.exec();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
