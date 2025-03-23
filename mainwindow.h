#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include "game2048.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

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
    
    Game2048 *m_game;
    QWidget *m_centralWidget;
    QGridLayout *m_gridLayout;
    QLabel *m_scoreLabel;
    QPushButton *m_newGameButton;
    QLabel *m_tiles[4][4];
};

#endif // MAINWINDOW_H