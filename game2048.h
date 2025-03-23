#ifndef GAME2048_H
#define GAME2048_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <QRandomGenerator>

class Game2048 : public QObject
{
    Q_OBJECT

public:
    enum class Direction {
        Up,
        Down,
        Left,
        Right
    };
    
    explicit Game2048(QObject *parent = nullptr);
    
    void newGame();
    bool move(Direction direction);
    
    int score() const { return m_score; }
    bool isGameOver() const { return m_gameOver; }
    int tileAt(int row, int col) const { return m_board[row][col]; }
    
signals:
    void scoreChanged(int score);
    void boardChanged();
    void gameOver();
    
private:
    void addRandomTile();
    bool moveTiles(Direction direction);
    bool mergeTiles(Direction direction);
    bool canMove() const;
    
    QVector<QVector<int>> m_board;
    int m_score;
    bool m_gameOver;
};

#endif // GAME2048_H