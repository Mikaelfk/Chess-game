#include "../ChessApplication/ChessApplication.h"
#include <iostream>
#include <QElapsedTimer>
#include <QThread>

#include <QApplication>
#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QTimer>
#include <cstdlib>

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

QGraphicsScene* boardScene(ChessApplication& window) {
    const auto views = window.findChildren<QGraphicsView*>();
    require(views.size() == 1, "Missing board view");
    return views.front()->scene();
}

void requirePieceImage(ChessApplication& window, int row, int column, const QString& resource) {
    for (auto* item : boardScene(window)->items()) {
        if (auto* pixmap = dynamic_cast<QGraphicsPixmapItem*>(item)) {
            if (pixmap->zValue() == 0 && pixmap->pos() == QPointF(column * 62.5, row * 62.5)) {
                require(pixmap->pixmap().toImage() == QPixmap(resource).toImage(), "Wrong piece image");
                return;
            }
        }
    }
    require(false, "Piece image not found");
}

void clickSquare(ChessApplication& window, int row, int column) {
    for (auto* item : boardScene(window)->items()) {
        if (auto* proxy = dynamic_cast<QGraphicsProxyWidget*>(item)) {
            auto* button = qobject_cast<QPushButton*>(proxy->widget());
            require(button != nullptr, "Missing board input button");
            QCursor::setPos(button->mapToGlobal(QPoint(column * 62.5 + 30, row * 62.5 + 30)));
            button->click();
            QApplication::processEvents();
            return;
        }
    }
    require(false, "Missing board input proxy");
}

void testRenderingAndCapture() {
    ChessApplication window(nullptr, false);
    window.show();
    QApplication::processEvents();
    int pixmapCount = 0;
    for (auto* item : boardScene(window)->items()) {
        if (auto* pixmap = dynamic_cast<QGraphicsPixmapItem*>(item)) {
            require(!pixmap->pixmap().isNull(), "Missing board or piece image");
            if(pixmap->zValue()==-1) require(pixmap->sceneBoundingRect()==QRectF(0,0,500,500),"Board background does not cover the squares");
            ++pixmapCount;
        }
    }
    require(pixmapCount == 33, "Expected board and 32 pieces");
    require(!window.grab().isNull(), "Window did not render");
    const char* backRank[] = {"Rook", "Knight", "Bishop", "Queen", "King", "Bishop", "Knight", "Rook"};
    for (int column = 0; column < 8; ++column) {
        requirePieceImage(window, 0, column, QString(":/Images/Black %1.png").arg(backRank[column]));
        requirePieceImage(window, 7, column, QString(":/Images/White %1.png").arg(backRank[column]));
        requirePieceImage(window, 1, column, ":/Images/Black Pawn.png");
        requirePieceImage(window, 6, column, ":/Images/White Pawn.png");
    }

    clickSquare(window, 6, 4); clickSquare(window, 4, 4); // e2-e4
    clickSquare(window, 1, 3); clickSquare(window, 3, 3); // d7-d5
    clickSquare(window, 4, 4); clickSquare(window, 3, 3); // e4xd5
    require(window.position().squares[3][3] == 1, "Pawn capture failed");
    int capturedLabels = 0;
    for (auto* label : window.findChildren<QLabel*>()) {
        if (!label->pixmap().isNull()) {
            require(label->pixmap().toImage() == QPixmap(":/Images/Black Pawn.png").toImage(),
                    "Wrong captured-piece image");
            ++capturedLabels;
        }
    }
    require(capturedLabels == 1, "Captured-piece display not updated");
}

void testPromotion(bool white, int choice) {
    chess::Position position;
    position.squares[7][7] = chess::King;
    position.squares[0][7] = chess::King+6;
    const int from = white ? 1 : 6, to = white ? 0 : 7;
    position.squares[from][0] = chess::Pawn + (white ? 0 : 6);
    position.whiteToMove = white;
    ChessApplication window(nullptr, false, position);
    window.show();
    QApplication::processEvents();
    clickSquare(window, from, 0);
    QTimer::singleShot(0, [&] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        require(dialog != nullptr, "Promotion dialog did not open");
        if (choice == -1) {
            dialog->reject();
            return;
        }
        const char* options[] = {"Queen", "Rook", "Bishop", "Knight"};
        for (auto* button : dialog->findChildren<QPushButton*>()) {
            if (button->text() == options[choice]) {
                button->click();
                return;
            }
        }
        require(false, "Promotion choice missing");
    });
    clickSquare(window, to, 0);
    const int types[] = {5, 4, 3, 2};
    const int type = types[choice < 0 ? 0 : choice] + (white ? 0 : 6);
    require(window.position().squares[to][0] == type, "Wrong promotion result");
    const char* names[] = {"Queen", "Rook", "Bishop", "Knight"};
    requirePieceImage(window, to, 0, QString(":/Images/%1 %2.png")
                      .arg(white ? "White" : "Black").arg(names[choice < 0 ? 0 : choice]));
    require(window.findChildren<QDialog*>().empty(), "Promotion dialog was not destroyed");
}

void waitForComputer(ChessApplication& window) {
    QElapsedTimer timer; timer.start();
    while(window.computerThinking() && timer.elapsed()<5000) {
        QApplication::processEvents(); QThread::msleep(5);
    }
    require(!window.computerThinking(), "Computer did not finish its move");
}

void restart(ChessApplication& window, bool white, bool computer, int difficulty = 0) {
    window.findChild<QComboBox*>("colourChoice")->setCurrentIndex(white ? 0 : 1);
    window.findChild<QComboBox*>("opponentChoice")->setCurrentIndex(computer ? 0 : 1);
    window.findChild<QComboBox*>("difficultyChoice")->setCurrentIndex(difficulty);
    window.findChild<QPushButton*>("newGame")->click();
    QApplication::processEvents();
}

void testComputerAndOrientation() {
    ChessApplication window;
    window.show(); QApplication::processEvents();
    clickSquare(window,6,4); clickSquare(window,4,4);
    require(window.computerThinking(), "AI not queued after human move");
    const auto before=window.position().repetitionKey();
    clickSquare(window,1,0); clickSquare(window,2,0);
    require(window.position().repetitionKey()==before, "User moved computer pieces while thinking");
    waitForComputer(window);
    require(window.position().whiteToMove, "AI did not hand turn back to White");
    restart(window,false,true);
    require(!window.playerIsWhite() && window.computerThinking(), "Black game did not start with computer White");
    for(auto* item:boardScene(window)->items()) {
        if(item->zValue()==-1) require(item->sceneBoundingRect()==QRectF(0,0,500,500),"Rotated board background moved out of bounds");
    }
    requirePieceImage(window,7,3,":/Images/Black King.png");
    requirePieceImage(window,0,3,":/Images/White King.png");
    waitForComputer(window);
    require(!window.position().whiteToMove, "Computer did not make White's opening move");
    // Black a7-a5 is at the bottom right in the rotated view.
    clickSquare(window,6,7); clickSquare(window,4,7);
    require(window.position().squares[3][0]==chess::Pawn+6, "Rotated Black input selected the wrong square");
    waitForComputer(window);
    require(!window.position().whiteToMove, "Computer did not reply to Black");
    // Start another search, then restart before its result can be applied.
    restart(window,false,true,2);
    require(window.computerThinking(),"Restart did not schedule AI");
    restart(window,true,true);
    restart(window,false,true,2);
    QElapsedTimer timer; timer.start();
    while(timer.elapsed()<150) { QApplication::processEvents(); QThread::msleep(5); }
    restart(window,true,true);
    const auto initial=window.position().repetitionKey();
    timer.restart();
    while(timer.elapsed()<1800) { QApplication::processEvents(); QThread::msleep(5); }
    require(window.position().repetitionKey()==initial && !window.computerThinking(), "Old AI result changed a new game");
    restart(window,false,false);
    // White e2-e4 from a Black viewpoint: screen (1,3) to (3,3).
    clickSquare(window,1,3); clickSquare(window,3,3);
    require(window.position().squares[4][4]==chess::Pawn, "Rotated White move mapped incorrectly");
    bool hint=false;
    clickSquare(window,6,2); // Black f7, with f5 and f6 hints.
    for(auto* item:boardScene(window)->items()) {
        if(dynamic_cast<QGraphicsEllipseItem*>(item) && item->pos()==QPointF(0,0)
           && item->boundingRect().contains(QPointF(2*62.5+31,4*62.5+31))) hint=true;
    }
    require(hint,"Rotated move hint missing");
    clickSquare(window,4,2); // f7-f5
    clickSquare(window,0,4); clickSquare(window,4,0); // Qd1-h5+
    require(window.position().inCheck(false), "Check setup failed");
    bool warning=false;
    for(auto* item:boardScene(window)->items()) {
        if(auto* rect=dynamic_cast<QGraphicsRectItem*>(item)) {
            if(rect->rect().topLeft()==QPointF(3*62.5,7*62.5)) warning=true;
        }
    }
    require(warning,"Check highlight was not rotated with the king");
}

void testGameOver() {
    chess::Position p;
    p.squares[0][0]=chess::King+6; p.squares[2][2]=chess::King; p.squares[2][1]=chess::Queen; p.whiteToMove=false;
    ChessApplication window(nullptr,true,p);
    window.show(); QApplication::processEvents();
    require(!window.computerThinking(),"Computer started after stalemate");
    require(window.findChild<QLabel*>("gameStatus")->text().contains("stalemate"),"Stalemate status missing");
    const auto before=window.position().repetitionKey();
    clickSquare(window,0,0); clickSquare(window,0,1);
    require(window.position().repetitionKey()==before,"Game accepted a move after stalemate");
}

void testRepetitionAndReset() {
    ChessApplication window(nullptr,false);
    window.show(); QApplication::processEvents();
    for(int repeat=0;repeat<2;++repeat) {
        clickSquare(window,7,6); clickSquare(window,5,5);
        clickSquare(window,0,6); clickSquare(window,2,5);
        clickSquare(window,5,5); clickSquare(window,7,6);
        clickSquare(window,2,5); clickSquare(window,0,6);
    }
    require(window.findChild<QLabel*>("gameStatus")->text().contains("threefold"),"Repetition draw missing");
    restart(window,true,false);
    require(!window.findChild<QLabel*>("gameStatus")->text().contains("Draw"),"New game retained old draw");
    const auto before=window.position().repetitionKey();
    clickSquare(window,-1,0); clickSquare(window,8,0);
    require(before==window.position().repetitionKey(),"Off-board click changed position");
    clickSquare(window,6,4); clickSquare(window,4,4);
    require(!window.position().whiteToMove,"New game did not accept input");
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    testRenderingAndCapture();
    testComputerAndOrientation();
    testGameOver();
    testRepetitionAndReset();
    for (bool white : {true, false}) for (int choice = -1; choice < 4; ++choice) {
        testPromotion(white, choice);
    }
}
