#include "../ChessApplication/ChessApplication.h"
#include <iostream>
#include <algorithm>
#include <QElapsedTimer>
#include <QThread>

#include <QApplication>
#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QTimer>
#include <QParallelAnimationGroup>
#include <QVariantAnimation>
#include <QMouseEvent>
#include <QKeyEvent>
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

void processFor(int milliseconds) {
    QElapsedTimer timer;
    timer.start();
    while(timer.elapsed() < milliseconds) {
        QApplication::processEvents();
        QThread::msleep(5);
    }
}

void waitForAnimation(ChessApplication& window) {
    QElapsedTimer timer;
    timer.start();
    while(window.moveAnimating() && timer.elapsed() < 3000) {
        QApplication::processEvents();
        QThread::msleep(5);
    }
    require(!window.moveAnimating(), "Piece animation did not finish");
}

void clickSquare(ChessApplication& window, int row, int column, bool wait = true) {
    for (auto* item : boardScene(window)->items()) {
        if (auto* proxy = dynamic_cast<QGraphicsProxyWidget*>(item)) {
            auto* button = qobject_cast<QPushButton*>(proxy->widget());
            require(button != nullptr, "Missing board input button");
            QCursor::setPos(button->mapToGlobal(QPoint(column * 62.5 + 30, row * 62.5 + 30)));
            button->click();
            if(wait) {
                QApplication::processEvents();
                waitForAnimation(window);
            }
            return;
        }
    }
    require(false, "Missing board input proxy");
}

QGraphicsPixmapItem* pieceAt(ChessApplication& window, int row, int column) {
    const QPointF point(column*62.5,row*62.5);
    for(auto* item : boardScene(window)->items()) {
        if(auto* pixmap = dynamic_cast<QGraphicsPixmapItem*>(item)) {
            if(pixmap->zValue() != -1 && pixmap->pos() == point) return pixmap;
        }
    }
    require(false, "Piece not found on starting square");
    return nullptr;
}

void advanceAnimationHalfway(ChessApplication& window) {
    require(window.moveAnimating(), "Expected active movement animation");
    for(auto* animation : window.findChildren<QParallelAnimationGroup*>()) {
        if(animation->state() == QAbstractAnimation::Running) {
            require(animation->duration() >= 120, "Movement animation is too short to be visible");
            animation->setCurrentTime(animation->duration()/2);
            return;
        }
    }
    require(false, "Movement animation group not found");
}

int pieceImageCount(ChessApplication& window, const QString& resource) {
    const QImage image = QPixmap(resource).toImage();
    int count = 0;
    for(auto* item : boardScene(window)->items()) {
        if(auto* pixmap = dynamic_cast<QGraphicsPixmapItem*>(item)) {
            if(pixmap->pixmap().toImage() == image) ++count;
        }
    }
    return count;
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
    auto* pawn = pieceAt(window,from,0);
    clickSquare(window, to, 0, false);
    require(window.moveAnimating(), "Promotion did not animate");
    advanceAnimationHalfway(window);
    require(pawn->pixmap().toImage() == QPixmap(white ? ":/Images/White Pawn.png" : ":/Images/Black Pawn.png").toImage(),
            "Pawn transformed before reaching the promotion square");
    require(pawn->pos().y() > std::min(from,to)*62.5 && pawn->pos().y() < std::max(from,to)*62.5,
            "Promoting pawn did not move through an intermediate position");
    waitForAnimation(window);
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
    while((window.computerThinking() || window.moveAnimating()) && timer.elapsed()<6000) {
        QApplication::processEvents(); QThread::msleep(5);
    }
    require(!window.computerThinking() && !window.moveAnimating(), "Computer did not finish its move");
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

void testSlidingAnimationAndInputLock() {
    ChessApplication window(nullptr,false);
    window.show(); QApplication::processEvents();
    clickSquare(window,6,4);
    auto* pawn = pieceAt(window,6,4);
    clickSquare(window,4,4,false);
    require(window.moveAnimating(), "Human move did not start an animation");
    const auto committed = window.position().repetitionKey();
    advanceAnimationHalfway(window);
    require(pawn->pos().x()==4*62.5 && pawn->pos().y()>4*62.5 && pawn->pos().y()<6*62.5,
            "Piece jumped directly to its destination");
    require(pawn->zValue()>0, "Moving piece was not raised above stationary pieces");
    clickSquare(window,1,0,false);
    window.on_pushButtonClicked(); // Also exercise the slot guard directly.
    require(window.position().repetitionKey()==committed, "Input changed the board during animation");
    waitForAnimation(window);
    requirePieceImage(window,4,4,":/Images/White Pawn.png");
    require(pieceImageCount(window,":/Images/White Pawn.png")==8, "Animation duplicated a piece");

    restart(window,false,false);
    clickSquare(window,1,3);
    pawn = pieceAt(window,1,3); // e2 from Black's viewpoint.
    clickSquare(window,3,3,false);
    advanceAnimationHalfway(window);
    require(pawn->pos().x()==3*62.5 && pawn->pos().y()>62.5 && pawn->pos().y()<3*62.5,
            "Animation did not follow the rotated board coordinates");
    waitForAnimation(window);
    requirePieceImage(window,3,3,":/Images/White Pawn.png");
}

void testCastlingAnimations() {
    for(bool white : {true,false}) for(bool kingSide : {true,false}) {
        chess::Position p;
        p.squares[7][4]=chess::King;
        p.squares[0][4]=chess::King+6;
        const int row=white?7:0, rookFrom=kingSide?7:0;
        const int kingTo=kingSide?6:2, rookTo=kingSide?5:3;
        p.squares[row][rookFrom]=chess::Rook+(white?0:6);
        p.castling[(white?0:2)+(kingSide?0:1)]=true;
        p.whiteToMove=white;
        ChessApplication window(nullptr,false,p);
        window.show(); QApplication::processEvents();
        clickSquare(window,row,4);
        auto* king = pieceAt(window,row,4);
        auto* rook = pieceAt(window,row,rookFrom);
        clickSquare(window,row,kingTo,false);
        advanceAnimationHalfway(window);
        require(king->pos().x()>std::min(4,kingTo)*62.5 && king->pos().x()<std::max(4,kingTo)*62.5,
                "Castling king did not animate");
        require(rook->pos().x()>std::min(rookFrom,rookTo)*62.5 && rook->pos().x()<std::max(rookFrom,rookTo)*62.5,
                "Castling rook did not animate alongside the king");
        waitForAnimation(window);
        requirePieceImage(window,row,kingTo,white?":/Images/White King.png":":/Images/Black King.png");
        requirePieceImage(window,row,rookTo,white?":/Images/White Rook.png":":/Images/Black Rook.png");
    }
}

void testCaptureAnimations() {
    for(bool enPassant : {false,true}) {
        chess::Position p;
        p.squares[7][7]=chess::King;
        p.squares[0][7]=chess::King+6;
        const int fromRow=enPassant?3:4, fromCol=enPassant?4:0, toRow=enPassant?2:4, toCol=3;
        p.squares[fromRow][fromCol]=enPassant?chess::Pawn:chess::Rook;
        p.squares[enPassant?3:4][3]=(enPassant?chess::Pawn:chess::Bishop)+6;
        if(enPassant) {p.enPassantRow=2;p.enPassantCol=3;}
        const QString captured=enPassant?":/Images/Black Pawn.png":":/Images/Black Bishop.png";
        ChessApplication window(nullptr,false,p);
        window.show(); QApplication::processEvents();
        clickSquare(window,fromRow,fromCol);
        auto* mover=pieceAt(window,fromRow,fromCol);
        clickSquare(window,toRow,toCol,false);
        advanceAnimationHalfway(window);
        require(mover->pos()!=QPointF(fromCol*62.5,fromRow*62.5)
                && mover->pos()!=QPointF(toCol*62.5,toRow*62.5), "Capture did not animate");
        require(pieceImageCount(window,captured)==1, "Captured piece disappeared before arrival");
        waitForAnimation(window);
        require(pieceImageCount(window,captured)==0, "Captured piece remained on the board after animation");
        requirePieceImage(window,toRow,toCol,enPassant?":/Images/White Pawn.png":":/Images/White Rook.png");
        int labels=0;
        for(auto* label:window.findChildren<QLabel*>()) if(!label->pixmap().isNull()) ++labels;
        require(labels==1,"Capture animation did not update captured-piece display");
    }
}

void testAIDelayAndAnimationCancellation() {
    ChessApplication window;
    window.show(); QApplication::processEvents();
    restart(window,false,true,0); // Easy computes almost immediately, but must still pause.
    const auto opening=window.position().repetitionKey();
    QElapsedTimer timer; timer.start();
    processFor(350);
    require(window.computerThinking() && !window.moveAnimating() && window.position().repetitionKey()==opening,
            "Computer opening move did not have a visible delay");
    QElapsedTimer searchWait; searchWait.start();
    while(window.computerThinking() && searchWait.elapsed()<5000) {
        QApplication::processEvents(); QThread::msleep(5);
    }
    require(window.moveAnimating(), "Computer move did not animate after the delay");
    advanceAnimationHalfway(window);
    bool sliding=false;
    for(auto* group:window.findChildren<QParallelAnimationGroup*>()) {
        if(group->state()!=QAbstractAnimation::Running) continue;
        auto* motion=qobject_cast<QVariantAnimation*>(group->animationAt(0));
        require(motion!=nullptr,"Computer movement track missing");
        const QPointF current=motion->currentValue().toPointF();
        require(current!=motion->startValue().toPointF() && current!=motion->endValue().toPointF(),
                "Computer movement jumped to an endpoint");
        for(auto* item:boardScene(window)->items()) {
            if(dynamic_cast<QGraphicsPixmapItem*>(item) && item->zValue()==2 && item->pos()==current) sliding=true;
        }
    }
    require(sliding,"Computer animation did not move the displayed piece");
    waitForComputer(window);
    require(timer.elapsed()>=350 && !window.position().whiteToMove, "Computer opening was not completed");

    restart(window,true,true,0);
    clickSquare(window,6,4); clickSquare(window,4,4);
    const auto afterHuman=window.position().repetitionKey();
    processFor(350);
    require(window.computerThinking() && !window.moveAnimating() && window.position().repetitionKey()==afterHuman,
            "Computer reply did not have a visible delay");
    waitForComputer(window);
    require(window.position().whiteToMove,"Delayed computer reply did not return the turn");

    restart(window,false,true,0);
    processFor(250); // Easy's result is held pending its display delay.
    require(window.computerThinking() && !window.moveAnimating(),"AI result was not pending during delay");
    restart(window,true,true,0);
    const auto initial=window.position().repetitionKey();
    processFor(700);
    require(!window.computerThinking() && !window.moveAnimating() && window.position().repetitionKey()==initial,
            "Held AI result moved pieces after New game");

    restart(window,true,false);
    clickSquare(window,6,4); clickSquare(window,4,4,false);
    advanceAnimationHalfway(window);
    restart(window,true,false);
    processFor(300);
    require(!window.moveAnimating() && window.position().repetitionKey()==chess::Position::initial().repetitionKey(),
            "Cancelled movement changed the restarted board");
    requirePieceImage(window,6,4,":/Images/White Pawn.png");

    auto closing=std::make_unique<ChessApplication>(nullptr,false);
    closing->show(); QApplication::processEvents();
    clickSquare(*closing,6,4); clickSquare(*closing,4,4,false);
    require(closing->moveAnimating(),"Close test did not start an animation");
    closing.reset();
    processFor(300); // No callback may touch the deleted moving graphics item.

    closing=std::make_unique<ChessApplication>();
    closing->show(); QApplication::processEvents();
    restart(*closing,false,true,0);
    processFor(250);
    require(closing->computerThinking(),"Close test did not leave an AI result pending");
    closing.reset();
    processFor(700);
}

void boardMouse(ChessApplication& window, QEvent::Type type, QPointF scenePoint) {
    auto* view = window.findChildren<QGraphicsView*>().front();
    const QPoint local = view->mapFromScene(scenePoint);
    const QPoint global = view->viewport()->mapToGlobal(local);
    QMouseEvent event(type,local,global,
        type == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton,
        type == QEvent::MouseButtonRelease ? Qt::NoButton : Qt::LeftButton,Qt::NoModifier);
    QApplication::sendEvent(view->viewport(),&event);
    QApplication::processEvents();
}

QPointF squareCenter(int row, int col) { return {col*62.5+31,row*62.5+31}; }

void dragSquare(ChessApplication& window, int fromRow, int fromCol, int toRow, int toCol) {
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(fromRow,fromCol));
    boardMouse(window,QEvent::MouseMove,squareCenter(toRow,toCol));
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(toRow,toCol));
    waitForAnimation(window);
}

void testDragAndCaptureHints() {
    ChessApplication window(nullptr,false);
    window.show(); QApplication::processEvents();
    const auto initial = window.position().repetitionKey();
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(6,4));
    boardMouse(window,QEvent::MouseMove,squareCenter(4,4));
    bool follows = false;
    for (auto* item : boardScene(window)->items())
        if (item->zValue() == 4 && item->pos() == QPointF(250,250)) follows = true;
    require(follows,"Dragged piece does not follow the pointer");
    require(window.position().repetitionKey() == initial,"Dragging changed the game before release");
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(4,4));
    waitForAnimation(window);
    require(window.position().squares[4][4] == chess::Pawn,"Drag did not play e2-e4");
    dragSquare(window,1,3,3,3);
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(4,4));
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(4,4));
    int rings=0,dots=0;
    for (auto* item : boardScene(window)->items()) if(auto* hint=dynamic_cast<QGraphicsEllipseItem*>(item)) {
        if(hint->brush().style() == Qt::NoBrush) {
            ++rings;
            require(hint->rect().center() == squareCenter(3,3)+QPointF(.25,.25),"Capture ring is on the wrong square");
        } else ++dots;
    }
    require(rings==1 && dots==1,"Capture and quiet move hints are not distinct");
    const auto before = window.position().repetitionKey();
    dragSquare(window,4,4,2,4);
    require(window.position().repetitionKey()==before,"Illegal drop changed the position");
    requirePieceImage(window,4,4,":/Images/White Pawn.png");
    dragSquare(window,4,4,-1,4);
    require(window.position().repetitionKey()==before,"Off-board drop changed the position");
    requirePieceImage(window,4,4,":/Images/White Pawn.png");
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(4,4));
    boardMouse(window,QEvent::MouseMove,squareCenter(3,3));
    QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
    QApplication::sendEvent(&window,&escape);
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(3,3));
    require(window.position().repetitionKey()==before,"Escape did not cancel the drag");
    requirePieceImage(window,4,4,":/Images/White Pawn.png");
    dragSquare(window,4,4,3,3);
    require(window.position().squares[3][3]==chess::Pawn,"Drag capture failed");

    restart(window,false,false);
    dragSquare(window,1,3,3,3); // White e2-e4 on the rotated board.
    dragSquare(window,6,3,4,3); // Black e7-e5.
    require(window.position().squares[4][4]==chess::Pawn && window.position().squares[3][4]==chess::Pawn+6,
            "Drag coordinates did not follow Black's orientation");
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(1,0));
    boardMouse(window,QEvent::MouseMove,squareCenter(2,0));
    restart(window,true,false);
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(2,0));
    require(window.position().repetitionKey()==initial,"Release after restart moved a stale piece");
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(6,4));
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(6,4));
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(4,4));
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(4,4));
    require(window.moveAnimating(),"Mouse click-to-move stopped working");
    const auto moving = window.position().repetitionKey();
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(1,3));
    boardMouse(window,QEvent::MouseMove,squareCenter(3,3));
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(3,3));
    require(window.position().repetitionKey()==moving,"Drag input was accepted during an animation");
    waitForAnimation(window);
}

void testDragSpecialMoves() {
    chess::Position castle;
    castle.squares[7][4]=chess::King; castle.squares[7][7]=chess::Rook;
    castle.squares[0][4]=chess::King+6; castle.castling[0]=true;
    ChessApplication window(nullptr,false,castle);
    window.show(); QApplication::processEvents();
    dragSquare(window,7,4,7,6);
    require(window.position().squares[7][6]==chess::King && window.position().squares[7][5]==chess::Rook,
            "Drag castling failed");

    chess::Position ep=chess::Position::initial();
    for(auto move : {chess::Move{6,4,4,4},{1,0,2,0},{4,4,3,4},{1,3,3,3}}) require(ep.play(move),"En passant setup failed");
    ChessApplication enPassant(nullptr,false,ep);
    enPassant.show(); QApplication::processEvents();
    clickSquare(enPassant,3,4);
    int epRings=0;
    for(auto* item:boardScene(enPassant)->items()) if(auto* hint=dynamic_cast<QGraphicsEllipseItem*>(item))
        if(hint->brush().style()==Qt::NoBrush) ++epRings;
    require(epRings==1,"En passant destination is not marked as a capture");
    dragSquare(enPassant,3,4,2,3);
    require(enPassant.position().squares[2][3]==chess::Pawn && enPassant.position().squares[3][3]==chess::Empty,
            "Drag en passant failed");

    chess::Position promotion;
    promotion.squares[7][7]=chess::King; promotion.squares[0][7]=chess::King+6;
    promotion.squares[1][0]=chess::Pawn;
    ChessApplication promote(nullptr,false,promotion);
    promote.show(); QApplication::processEvents();
    boardMouse(promote,QEvent::MouseButtonPress,squareCenter(1,0));
    int promotionHints=0;
    for(auto* item:boardScene(promote)->items()) if(dynamic_cast<QGraphicsEllipseItem*>(item)) ++promotionHints;
    require(promotionHints==1,"Promotion choices duplicate the destination hint");
    boardMouse(promote,QEvent::MouseMove,squareCenter(0,0));
    QTimer::singleShot(0,[]{
        auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
        require(dialog!=nullptr,"Drag promotion dialog missing");
        for(auto* button:dialog->findChildren<QPushButton*>()) if(button->text()=="Knight") { button->click(); return; }
        require(false,"Knight promotion choice missing");
    });
    boardMouse(promote,QEvent::MouseButtonRelease,squareCenter(0,0));
    waitForAnimation(promote);
    require(promote.position().squares[0][0]==chess::Knight,"Drag promotion failed");
}

void requireSelection(ChessApplication& window, int row = -1, int col = -1) {
    int count = 0;
    for (auto* item : boardScene(window)->items()) {
        if (auto* rect = dynamic_cast<QGraphicsRectItem*>(item); rect && rect->zValue() == -0.5) {
            ++count;
            require(rect->rect() == QRectF(col*62.5,row*62.5,62.5,62.5),"Selection highlights the wrong square");
            require(rect->brush().style() != Qt::NoBrush && rect->opacity() > 0,"Selection is not visible");
        }
    }
    require(count == (row < 0 ? 0 : 1),"Unexpected number of selection highlights");
}

void testSelectionHighlight() {
    ChessApplication window(nullptr,false);
    window.show(); QApplication::processEvents();
    requireSelection(window);
    clickSquare(window,6,4);
    requireSelection(window,6,4);
    clickSquare(window,7,2); // A blocked bishop still has a selected square.
    requireSelection(window,7,2);
    clickSquare(window,4,2);
    requireSelection(window);
    clickSquare(window,1,4); // Opponent's piece cannot be selected.
    requireSelection(window);
    boardMouse(window,QEvent::MouseButtonPress,squareCenter(6,4));
    boardMouse(window,QEvent::MouseMove,squareCenter(3,4));
    requireSelection(window,6,4); // Highlight stays at the origin while dragging.
    boardMouse(window,QEvent::MouseButtonRelease,squareCenter(3,4));
    requireSelection(window,6,4); // Illegal drop retains selection.
    clickSquare(window,4,4,false);
    requireSelection(window); // Clear as soon as a move starts.
    waitForAnimation(window);
    requireSelection(window);
    restart(window,false,false);
    clickSquare(window,1,3); // White e2 on the rotated board.
    requireSelection(window,1,3);
    restart(window,true,false);
    requireSelection(window);
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    testSelectionHighlight();
    testDragAndCaptureHints();
    testDragSpecialMoves();
    testRenderingAndCapture();
    testSlidingAnimationAndInputLock();
    testCastlingAnimations();
    testCaptureAnimations();
    testAIDelayAndAnimationCancellation();
    testComputerAndOrientation();
    testGameOver();
    testRepetitionAndReset();
    for (bool white : {true, false}) for (int choice = -1; choice < 4; ++choice) {
        testPromotion(white, choice);
    }
}
