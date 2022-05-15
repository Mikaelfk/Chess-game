#include "ChessApplication.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChessApplication w;
    w.show();
    return a.exec();
}
