#pragma once

#include <QtWidgets/QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>

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
    QGraphicsScene scene;
    QGraphicsView view;
	QPushButton* pushButton;

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

};
