#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ChessApplication.h"

class ChessApplication : public QMainWindow
{
    Q_OBJECT

public:
    ChessApplication(QWidget *parent = Q_NULLPTR);

private:
    Ui::ChessApplicationClass ui;
};
