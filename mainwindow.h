#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include "game2048.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void updateBoard();
    void updateScore(int score);
    void handleGameOver();

private:
    void setupUi();
    void updateTileAppearance(QLabel *label, int value);
    QString getTileStyleSheet(int value);
    QString getTileColor(int value);
    QString getTextColor(int value);
    void animateTileMovement(int fromRow, int fromCol, int toRow, int toCol);
    void animateTileMerge(int row, int col);
    void animateNewTile(int row, int col);
    void startAnimations();
    void calculateAnimations();
    
    Game2048 *m_game;
    QWidget *m_centralWidget;
    QGridLayout *m_gridLayout;
    QLabel *m_scoreLabel;
    QPushButton *m_newGameButton;
    QLabel *m_tiles[4][4];
    
    // 动画相关
    QParallelAnimationGroup *m_animationGroup;
    bool m_animationRunning;
    QVector<QVector<int>> m_previousBoard;
    struct TileMovement {
        int fromRow;
        int fromCol;
        int toRow;
        int toCol;
    };
    QList<TileMovement> m_tileMovements;
    QList<QPair<int, int>> m_mergedTiles;
    QList<QPair<int, int>> m_newTiles;
};

#endif // MAINWINDOW_H