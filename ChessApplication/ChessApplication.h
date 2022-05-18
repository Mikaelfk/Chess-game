#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ChessApplication.h"

class ChessApplication : public QMainWindow
{
    Q_OBJECT

public:
    ChessApplication(QWidget *parent = Q_NULLPTR);
    void updateBoard();
    bool pieceChosen = false;
    std::vector<std::pair<int, int>> legalMoves = std::vector<std::pair<int, int>>();
    std::pair<int, int> activePiecePosition = std::make_pair(0, 0);

public slots:
    void on_pushButton_clicked();

private:
    Ui::ChessApplicationClass ui;

};
