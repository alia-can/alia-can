#include <QApplication>
#include "mainwindow.hpp"
#include "shelldetector.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    try {
        MainWindow window;
        window.show();
        return app.exec();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
