#include <QApplication>
#include "Mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Hoja de Cálculo — Matriz Dispersa");
    app.setOrganizationName("Algoritmos y Estructura de Datos");
    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}