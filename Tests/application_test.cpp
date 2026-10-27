#include "../ChessApplication/ChessApplication.h"
#include "../ChessBackend/Board.h"
#include "../ChessBackend/Pieces/ChessKing.h"
#include "../ChessBackend/Pieces/ChessPawn.h"

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
            window.on_pushButtonClicked();
            QApplication::processEvents();
            return;
        }
    }
    require(false, "Missing board input proxy");
}

void testRenderingAndCapture() {
    ChessApplication window;
    window.show();
    QApplication::processEvents();
    int pixmapCount = 0;
    for (auto* item : boardScene(window)->items()) {
        if (auto* pixmap = dynamic_cast<QGraphicsPixmapItem*>(item)) {
            require(!pixmap->pixmap().isNull(), "Missing board or piece image");
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
    require(Board::board[3][3]->getPieceType() == 1, "Pawn capture failed");
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
    for (auto& row : Board::board) for (auto*& piece : row) {
        delete piece;
        piece = new ChessPiece();
    }
    auto place = [](int row, int column, ChessPiece* piece) {
        delete Board::board[row][column];
        Board::board[row][column] = piece;
    };
    place(7, 7, new ChessKing(7, 7, true));
    place(0, 7, new ChessKing(0, 7, false));
    const int from = white ? 1 : 6, to = white ? 0 : 7;
    place(from, 0, new ChessPawn(from, 0, white));
    Board::whiteToMove = white;
    Board::isCheckmate = Board::isStalemate = false;
    Board::isCheckOnWhite = Board::isCheckOnBlack = false;
    Board::canEnPassant = Board::enPassantHappened = false;
    Board::whiteCastledKingSide = Board::whiteCastledQueenSide = false;
    Board::blackCastledKingSide = Board::blackCastledQueenSide = false;
    Board::takenPieces.clear();

    ChessApplication window;
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
    require(Board::board[to][0]->getPieceType() == type, "Wrong promotion result");
    const char* names[] = {"Queen", "Rook", "Bishop", "Knight"};
    requirePieceImage(window, to, 0, QString(":/Images/%1 %2.png")
                      .arg(white ? "White" : "Black").arg(names[choice < 0 ? 0 : choice]));
    require(window.findChildren<QDialog*>().empty(), "Promotion dialog was not destroyed");
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    Board::getInstance();
    testRenderingAndCapture();
    for (bool white : {true, false}) for (int choice = -1; choice < 4; ++choice) {
        testPromotion(white, choice);
    }
}
