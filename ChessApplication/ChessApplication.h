#pragma once

#include <QtWidgets/QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include "ui_ChessApplication.h"


class ChessApplication : public QMainWindow
{
    Q_OBJECT

public:
    ChessApplication(QWidget *parent = Q_NULLPTR);

public slots:
    void on_pushButton_clicked();

private:
    QGraphicsScene scene;
    QGraphicsView view;
	QPushButton* pushButton;

	QHBoxLayout* playerInfoLayout1;
	QHBoxLayout* playerInfoLayout2;

	// 2d vector for QGraphicsPixmapItem pointers
	std::vector<std::vector<QGraphicsPixmapItem*>> board = {
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
		{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
	};
	// vector of move hints
	std::vector<QGraphicsEllipseItem*> moveHints = {};
	// red sqaure on king in check
	QGraphicsRectItem* kingInCheckWarning = nullptr;

	Ui::ChessApplication ui;

	void showTakenPiece(int &pieceType);

    void updateBoard();
    bool pieceChosen = false;
    std::vector<std::pair<int, int>> legalMoves = std::vector<std::pair<int, int>>();
    std::pair<int, int> activePiecePosition = std::make_pair(0, 0);
};


