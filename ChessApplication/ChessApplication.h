#pragma once

#include <QComboBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QPushButton>
#include <QParallelAnimationGroup>
#include <QMainWindow>
#include <QHBoxLayout>
#include <atomic>
#include <map>
#include <memory>
#include <optional>
#include <QPointer>
#include <QDialog>
#include "../ChessBackend/ChessAI.h"
#include "ui_ChessApplication.h"

class ChessApplication : public QMainWindow {
    Q_OBJECT
public:
    explicit ChessApplication(QWidget* parent = nullptr, bool computerOpponent = true,
                              chess::Position initial = chess::Position::initial());
    ~ChessApplication() override;
    const chess::Position& position() const { return game; }
    bool computerThinking() const { return thinking; }
    bool moveAnimating() const { return moveAnimation != nullptr; }
    bool playerIsWhite() const { return humanWhite; }

public slots:
    void on_pushButtonClicked();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Ui::ChessApplication ui;
    QGraphicsScene scene;
    QGraphicsView view;
    QPushButton* boardInput;
    QComboBox *colourChoice, *difficultyChoice, *opponentChoice;
    QLabel *status, *topPlayer, *bottomPlayer;
    QHBoxLayout *topCaptures, *bottomCaptures;
    std::vector<QWidget*> captureLabels;
    std::vector<int> displayedCaptures;
    std::optional<int> displayedBalance;
    bool displayedHumanWhite = true;
    std::vector<int> capturedPieces;
    std::vector<QGraphicsItem*> overlays;
    std::vector<QGraphicsPixmapItem*> pieceItems;
    QGraphicsPixmapItem* background;
    chess::Position game;
    chess::Difficulty difficulty = chess::Difficulty::Medium;
    bool humanWhite = true, againstComputer = true, thinking = false, gameOver = false;
    bool resultShown = false;
    QPointer<QDialog> resultDialog;
    QPointer<QDialog> replayDialog;
    QPushButton* replayButton;
    std::vector<chess::Position> history;
    std::vector<chess::Move> moveHistory;
    std::vector<chess::Move> selectedMoves;
    std::optional<QPoint> selectedSquare;
    std::map<std::string, int> repetitions;
    std::shared_ptr<std::atomic_bool> cancelled;
    unsigned generation = 0;
    QParallelAnimationGroup* moveAnimation = nullptr;
    QGraphicsPixmapItem* draggedPiece = nullptr;
    QPointF pressPosition, dragOrigin, dragOffset;
    bool boardPressed = false, dragging = false;

    QPoint displaySquare(int row, int column) const;
    QPoint boardSquare(QPointF point) const;
    bool inputAllowed() const;
    void resetDrag();
    void selectSquare(QPoint square);
    bool moveTo(QPoint square, std::optional<QPointF> dragStart = std::nullopt);
    void startGame(chess::Position position);
    void newGame();
    void render();
    void showGameResult(chess::Outcome result);
    void showReplay();
    void updateCaptureDisplay();
    void applyMove(const chess::Move& move, std::optional<QPointF> dragStart = std::nullopt);
    void stopMoveAnimation();
    void queueComputerMove();
};
