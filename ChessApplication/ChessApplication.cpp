#include <QLabel>
#include <QHBoxLayout>
#include "ChessApplication.h"
#include "../ChessBackend/Board.h"

ChessApplication::ChessApplication(QWidget *parent)
    : QMainWindow(parent)
{
    Board::getInstance();
	
    // Create a label
    QLabel* label = new QLabel(this);
    QPixmap pixmap(":/Images/ChessBoard.png");
    label->setPixmap(pixmap);
    label->setScaledContents(true);
    label->setFixedWidth(600);
    label->setFixedHeight(600);
    label->setAlignment(Qt::AlignCenter);
	
    QWidget* widget = new QWidget;

    // create horizontal layout
    QHBoxLayout* layout = new QHBoxLayout;
    // Add label to the the layout
    layout->addWidget(label);

    // set layout to widget
    widget->setLayout(layout);

    setCentralWidget(widget);
}
