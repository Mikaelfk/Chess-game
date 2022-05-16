#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include "ChessApplication.h"
#include "../ChessBackend/Board.h"

ChessApplication::ChessApplication(QWidget *parent)
    : QMainWindow(parent)
{
    // Initiate the board
    Board::getInstance();



    // Create a grid
	QVBoxLayout* gridVertical = new QVBoxLayout;

	QLabel* whiteKing = new QLabel;
    QPixmap whiteKingMap(":/Images/White King.png");
    whiteKing->setPixmap(whiteKingMap);

    gridVertical->addWidget(whiteKing);



	// Create background label which contains the board
    QLabel* label = new QLabel(this);
    QPixmap pixmap(":/Images/ChessBoard.png");
    label->setPixmap(pixmap);
    label->setScaledContents(true);
    label->setFixedWidth(600);
    label->setFixedHeight(600);
    label->setAlignment(Qt::AlignCenter);

    // Create horizontal layout
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addLayout(gridVertical);
    layout->addWidget(label);

    // Create widget
    QWidget* mainWidget = new QWidget;
    mainWidget->setLayout(layout);

    setCentralWidget(mainWidget);
}
