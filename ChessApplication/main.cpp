#include <QtWidgets/QApplication>

#include "ChessApplication.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    ChessApplication w;
    w.show();
    return a.exec();
}
