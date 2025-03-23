#include "game2048.h"
#include <QDebug>

Game2048::Game2048(QObject *parent)
    : QObject(parent)
    , m_score(0)
    , m_gameOver(false)
{
    m_board.resize(4);
    for (int i = 0; i < 4; ++i) {
        m_board[i].resize(4);
    }
    
    newGame();
}

void Game2048::newGame()
{
    // 清空游戏板
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            m_board[row][col] = 0;
        }
    }
    
    m_score = 0;
    m_gameOver = false;
    
    // 添加两个初始方块
    addRandomTile();
    addRandomTile();
    
    emit scoreChanged(m_score);
    emit boardChanged();
}

void Game2048::addRandomTile()
{
    QVector<QPair<int, int>> emptyCells;
    
    // 找出所有空白格子
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (m_board[row][col] == 0) {
                emptyCells.append(qMakePair(row, col));
            }
        }
    }
    
    if (emptyCells.isEmpty()) {
        return;
    }
    
    // 随机选择一个空白格子
    int index = QRandomGenerator::global()->bounded(emptyCells.size());
    int row = emptyCells[index].first;
    int col = emptyCells[index].second;
    
    // 90%概率生成2，10%概率生成4
    m_board[row][col] = (QRandomGenerator::global()->bounded(10) < 9) ? 2 : 4;
}

bool Game2048::move(Direction direction)
{
    if (m_gameOver) {
        return false;
    }
    
    bool moved = moveTiles(direction);
    bool merged = mergeTiles(direction);
    bool moved2 = moveTiles(direction); // 合并后再次移动
    
    if (moved || merged || moved2) {
        addRandomTile();
        emit boardChanged();
        
        if (!canMove()) {
            m_gameOver = true;
            emit gameOver();
        }
        
        return true;
    }
    
    return false;
}

bool Game2048::moveTiles(Direction direction)
{
    bool moved = false;
    
    switch (direction) {
    case Direction::Up:
        for (int col = 0; col < 4; ++col) {
            for (int row = 1; row < 4; ++row) {
                if (m_board[row][col] != 0) {
                    int newRow = row;
                    while (newRow > 0 && m_board[newRow - 1][col] == 0) {
                        m_board[newRow - 1][col] = m_board[newRow][col];
                        m_board[newRow][col] = 0;
                        newRow--;
                        moved = true;
                    }
                }
            }
        }
        break;
        
    case Direction::Down:
        for (int col = 0; col < 4; ++col) {
            for (int row = 2; row >= 0; --row) {
                if (m_board[row][col] != 0) {
                    int newRow = row;
                    while (newRow < 3 && m_board[newRow + 1][col] == 0) {
                        m_board[newRow + 1][col] = m_board[newRow][col];
                        m_board[newRow][col] = 0;
                        newRow++;
                        moved = true;
                    }
                }
            }
        }
        break;
        
    case Direction::Left:
        for (int row = 0; row < 4; ++row) {
            for (int col = 1; col < 4; ++col) {
                if (m_board[row][col] != 0) {
                    int newCol = col;
                    while (newCol > 0 && m_board[row][newCol - 1] == 0) {
                        m_board[row][newCol - 1] = m_board[row][newCol];
                        m_board[row][newCol] = 0;
                        newCol--;
                        moved = true;
                    }
                }
            }
        }
        break;
        
    case Direction::Right:
        for (int row = 0; row < 4; ++row) {
            for (int col = 2; col >= 0; --col) {
                if (m_board[row][col] != 0) {
                    int newCol = col;
                    while (newCol < 3 && m_board[row][newCol + 1] == 0) {
                        m_board[row][newCol + 1] = m_board[row][newCol];
                        m_board[row][newCol] = 0;
                        newCol++;
                        moved = true;
                    }
                }
            }
        }
        break;
    }
    
    return moved;
}

bool Game2048::mergeTiles(Direction direction)
{
    bool merged = false;
    
    switch (direction) {
    case Direction::Up:
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 3; ++row) {
                if (m_board[row][col] != 0 && m_board[row][col] == m_board[row + 1][col]) {
                    m_board[row][col] *= 2;
                    m_board[row + 1][col] = 0;
                    m_score += m_board[row][col];
                    merged = true;
                }
            }
        }
        break;
        
    case Direction::Down:
        for (int col = 0; col < 4; ++col) {
            for (int row = 3; row > 0; --row) {
                if (m_board[row][col] != 0 && m_board[row][col] == m_board[row - 1][col]) {
                    m_board[row][col] *= 2;
                    m_board[row - 1][col] = 0;
                    m_score += m_board[row][col];
                    merged = true;
                }
            }
        }
        break;
        
    case Direction::Left:
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 3; ++col) {
                if (m_board[row][col] != 0 && m_board[row][col] == m_board[row][col + 1]) {
                    m_board[row][col] *= 2;
                    m_board[row][col + 1] = 0;
                    m_score += m_board[row][col];
                    merged = true;
                }
            }
        }
        break;
        
    case Direction::Right:
        for (int row = 0; row < 4; ++row) {
            for (int col = 3; col > 0; --col) {
                if (m_board[row][col] != 0 && m_board[row][col] == m_board[row][col - 1]) {
                    m_board[row][col] *= 2;
                    m_board[row][col - 1] = 0;
                    m_score += m_board[row][col];
                    merged = true;
                }
            }
        }
        break;
    }
    
    if (merged) {
        emit scoreChanged(m_score);
    }
    
    return merged;
}

bool Game2048::canMove() const
{
    // 检查是否有空格子
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (m_board[row][col] == 0) {
                return true;
            }
        }
    }
    
    // 检查是否有相邻的相同数字
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (m_board[row][col] == m_board[row][col + 1]) {
                return true;
            }
        }
    }
    
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 3; ++row) {
            if (m_board[row][col] == m_board[row + 1][col]) {
                return true;
            }
        }
    }
    
    return false;
}