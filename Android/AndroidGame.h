#pragma once

#include "../ChessBackend/ChessAI.h"
#include <QObject>
#include <QVariantList>
#include <atomic>
#include <map>
#include <memory>
#include <optional>

// The phone interface uses the same rules and search as the desktop application.
class AndroidGame : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList board READ board NOTIFY stateChanged)
    Q_PROPERTY(QString status READ status NOTIFY stateChanged)
    Q_PROPERTY(QString result READ result NOTIFY stateChanged)
    Q_PROPERTY(bool thinking READ thinking NOTIFY stateChanged)
    Q_PROPERTY(bool gameOver READ gameOver NOTIFY stateChanged)
    Q_PROPERTY(bool humanWhite READ humanWhite NOTIFY stateChanged)
    Q_PROPERTY(bool againstComputer READ againstComputer NOTIFY stateChanged)
    Q_PROPERTY(int difficulty READ difficulty NOTIFY stateChanged)
    Q_PROPERTY(bool promotionPending READ promotionPending NOTIFY stateChanged)
    Q_PROPERTY(bool replaying READ replaying NOTIFY stateChanged)
    Q_PROPERTY(int replayIndex READ replayIndex NOTIFY stateChanged)
    Q_PROPERTY(int replayLength READ replayLength NOTIFY stateChanged)
    Q_PROPERTY(QString material READ material NOTIFY stateChanged)
    Q_PROPERTY(QString saveError READ saveError NOTIFY stateChanged)
public:
    explicit AndroidGame(const QString& stateFile = {}, QObject* parent = nullptr);
    ~AndroidGame() override;
    QVariantList board() const;
    QString status() const;
    QString result() const;
    QString material() const;
    QString saveError() const { return storageError; }
    bool thinking() const { return searching; }
    bool gameOver() const;
    bool humanWhite() const { return playerWhite; }
    bool againstComputer() const { return computer; }
    int difficulty() const { return level; }
    bool promotionPending() const { return pending.has_value(); }
    bool replaying() const { return replay; }
    int replayIndex() const { return replayStep; }
    int replayLength() const { return int(moves.size()); }
    const chess::Position& position() const { return current; }
    QByteArray savedState() const;
    bool restoreState(const QByteArray& data);

    Q_INVOKABLE void selectSquare(int index);
    Q_INVOKABLE void promote(int piece);
    Q_INVOKABLE void cancelPromotion();
    Q_INVOKABLE void newGame(bool white, bool computerOpponent, int difficulty);
    Q_INVOKABLE void startReplay();
    Q_INVOKABLE void seekReplay(int step);
    Q_INVOKABLE void exitReplay();
    void setApplicationActive(bool active);
signals:
    void stateChanged();
private:
    chess::Position current = chess::Position::initial();
    std::vector<chess::Position> history;
    std::vector<chess::Move> moves;
    std::map<std::string, int> repetitions;
    std::optional<chess::Move> pending;
    std::shared_ptr<std::atomic_bool> cancelled;
    QString stateFile, storageError;
    int selected = -1, level = 1, replayStep = 0;
    bool playerWhite = true, computer = true, searching = false, active = true, replay = false;
    unsigned generation = 0;
    const chess::Position& displayedPosition() const;
    bool inputAllowed() const;
    void applyMove(const chess::Move& move);
    void queueComputerMove();
    void cancelSearch();
    void save();
};
