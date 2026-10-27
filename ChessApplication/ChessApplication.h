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

private:
    Ui::ChessApplication ui;
    QGraphicsScene scene;
    QGraphicsView view;
    QPushButton* boardInput;
    QComboBox *colourChoice, *difficultyChoice, *opponentChoice;
    QLabel *status, *topPlayer, *bottomPlayer;
    QHBoxLayout *topCaptures, *bottomCaptures;
    std::vector<QLabel*> captureLabels;
    std::vector<int> capturedPieces;
    std::vector<QGraphicsItem*> overlays;
    std::vector<QGraphicsPixmapItem*> pieceItems;
    QGraphicsPixmapItem* background;
    chess::Position game;
    chess::Difficulty difficulty = chess::Difficulty::Medium;
    bool humanWhite = true, againstComputer = true, thinking = false, gameOver = false;
    std::vector<chess::Move> selectedMoves;
    std::map<std::string, int> repetitions;
    std::shared_ptr<std::atomic_bool> cancelled;
    unsigned generation = 0;
    QParallelAnimationGroup* moveAnimation = nullptr;

    QPoint displaySquare(int row, int column) const;
    void startGame(chess::Position position);
    void newGame();
    void render();
    void applyMove(const chess::Move& move);
    void stopMoveAnimation();
    void queueComputerMove();
};
