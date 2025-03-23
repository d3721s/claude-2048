#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_game(new Game2048(this))
    , m_animationGroup(new QParallelAnimationGroup(this))
    , m_animationRunning(false)
{
    setupUi();
    
    // 初始化动画相关的成员变量
    m_previousBoard.resize(4);
    for (int i = 0; i < 4; ++i) {
        m_previousBoard[i].resize(4);
    }
    
    // 连接信号和槽
    connect(m_game, &Game2048::boardChanged, this, &MainWindow::updateBoard);
    connect(m_game, &Game2048::scoreChanged, this, &MainWindow::updateScore);
    connect(m_game, &Game2048::gameOver, this, &MainWindow::handleGameOver);
    connect(m_newGameButton, &QPushButton::clicked, m_game, &Game2048::newGame);
    connect(m_newGameButton, &QPushButton::clicked, this, [this]() {
        // 确保点击新游戏按钮后窗口重新获得焦点
        QTimer::singleShot(10, [this](){ this->setFocus(); });
    });
    
    // 初始化游戏界面
    updateBoard();
    updateScore(0);
    
    // 确保窗口获得焦点
    this->setFocus();
    this->activateWindow();
    
    // 安装事件过滤器，捕获所有子控件的键盘事件
    m_centralWidget->installEventFilter(this);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    // 设置窗口属性
    setWindowTitle("2048 Game");
    resize(400, 500);
    
    // 设置焦点策略，确保可以接收键盘事件
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    
    // 创建顶部布局（分数和新游戏按钮）
    QHBoxLayout *topLayout = new QHBoxLayout();
    mainLayout->addLayout(topLayout);
    
    // 创建分数标签
    m_scoreLabel = new QLabel("Score: 0", this);
    m_scoreLabel->setFont(QFont("Arial", 16, QFont::Bold));
    topLayout->addWidget(m_scoreLabel);
    
    // 创建新游戏按钮
    m_newGameButton = new QPushButton("New Game", this);
    m_newGameButton->setFont(QFont("Arial", 12));
    m_newGameButton->setFocusPolicy(Qt::NoFocus); // 防止按钮抢占焦点
    topLayout->addWidget(m_newGameButton);
    
    // 创建游戏网格布局
    m_gridLayout = new QGridLayout();
    m_gridLayout->setSpacing(10);
    mainLayout->addLayout(m_gridLayout);
    
    // 创建游戏方块
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            m_tiles[row][col] = new QLabel(this);
            m_tiles[row][col]->setAlignment(Qt::AlignCenter);
            m_tiles[row][col]->setFont(QFont("Arial", 20, QFont::Bold));
            m_tiles[row][col]->setMinimumSize(80, 80);
            m_tiles[row][col]->setMaximumSize(80, 80);
            m_gridLayout->addWidget(m_tiles[row][col], row, col);
            
            // 设置初始样式
            updateTileAppearance(m_tiles[row][col], 0);
        }
    }
    
    // 添加一些说明
    QLabel *instructionLabel = new QLabel("使用方向键移动方块", this);
    instructionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(instructionLabel);
    
    // 连接动画完成信号
    connect(m_animationGroup, &QParallelAnimationGroup::finished, this, [this]() {
        m_animationRunning = false;
        updateBoard(); // 确保所有方块显示正确的值
    });
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_game->isGameOver() || m_animationRunning) {
        return;
    }
    
    // 保存当前棋盘状态，用于后续动画计算
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            m_previousBoard[row][col] = m_game->tileAt(row, col);
        }
    }
    
    // 清空动画相关的列表
    m_tileMovements.clear();
    m_mergedTiles.clear();
    m_newTiles.clear();
    
    bool moved = false;
    switch (event->key()) {
    case Qt::Key_Up:
        moved = m_game->move(Game2048::Direction::Up);
        break;
    case Qt::Key_Down:
        moved = m_game->move(Game2048::Direction::Down);
        break;
    case Qt::Key_Left:
        moved = m_game->move(Game2048::Direction::Left);
        break;
    case Qt::Key_Right:
        moved = m_game->move(Game2048::Direction::Right);
        break;
    default:
        QMainWindow::keyPressEvent(event);
        return;
    }
    
    if (moved) {
        // 计算方块移动、合并和新方块的位置
        calculateAnimations();
        // 开始动画
        startAnimations();
    }
}

void MainWindow::updateBoard()
{
    // 如果动画正在运行，不更新界面
    if (m_animationRunning) {
        return;
    }
    
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int value = m_game->tileAt(row, col);
            updateTileAppearance(m_tiles[row][col], value);
        }
    }
}

// 计算需要动画的方块
void MainWindow::calculateAnimations()
{
    // 找出移动的方块
    for (int newRow = 0; newRow < 4; ++newRow) {
        for (int newCol = 0; newCol < 4; ++newCol) {
            int newValue = m_game->tileAt(newRow, newCol);
            
            // 跳过空格子
            if (newValue == 0) {
                continue;
            }
            
            // 如果这个位置原来是空的，可能是新方块或者移动过来的方块
            if (m_previousBoard[newRow][newCol] == 0) {
                // 查找这个值是否是从其他位置移动过来的
                bool foundSource = false;
                
                for (int oldRow = 0; oldRow < 4 && !foundSource; ++oldRow) {
                    for (int oldCol = 0; oldCol < 4 && !foundSource; ++oldCol) {
                        // 跳过当前位置
                        if (oldRow == newRow && oldCol == newCol) {
                            continue;
                        }
                        
                        // 如果找到了可能的源位置
                        if (m_previousBoard[oldRow][oldCol] == newValue || 
                            m_previousBoard[oldRow][oldCol] * 2 == newValue) {
                            // 检查这个源位置是否已经被记录为其他移动的源
                            bool alreadyMoved = false;
                            for (const TileMovement &move : m_tileMovements) {
                                if (move.fromRow == oldRow && move.fromCol == oldCol) {
                                    alreadyMoved = true;
                                    break;
                                }
                            }
                            
                            if (!alreadyMoved) {
                                // 记录移动
                                TileMovement move;
                                move.fromRow = oldRow;
                                move.fromCol = oldCol;
                                move.toRow = newRow;
                                move.toCol = newCol;
                                m_tileMovements.append(move);
                                
                                // 如果值翻倍了，记录为合并
                                if (m_previousBoard[oldRow][oldCol] * 2 == newValue) {
                                    m_mergedTiles.append(qMakePair(newRow, newCol));
                                }
                                
                                foundSource = true;
                            }
                        }
                    }
                }
                
                // 如果没找到源位置，认为是新方块
                if (!foundSource) {
                    m_newTiles.append(qMakePair(newRow, newCol));
                }
            }
            // 如果这个位置原来有值，但值变了，可能是合并的结果
            else if (m_previousBoard[newRow][newCol] != newValue) {
                // 如果值翻倍了，记录为合并
                if (m_previousBoard[newRow][newCol] * 2 == newValue) {
                    m_mergedTiles.append(qMakePair(newRow, newCol));
                }
            }
        }
    }
}

void MainWindow::updateScore(int score)
{
    m_scoreLabel->setText(QString("Score: %1").arg(score));
}

void MainWindow::handleGameOver()
{
    QMessageBox::information(this, "Game Over", QString("Game Over! Your score: %1").arg(m_game->score()));
}

void MainWindow::updateTileAppearance(QLabel *label, int value)
{
    if (value == 0) {
        label->setText("");
    } else {
        label->setText(QString::number(value));
    }
    
    label->setStyleSheet(getTileStyleSheet(value));
}

QString MainWindow::getTileStyleSheet(int value)
{
    return QString("background-color: %1; color: %2; border-radius: 10px; font-weight: bold;")
            .arg(getTileColor(value))
            .arg(getTextColor(value));
}

QString MainWindow::getTileColor(int value)
{
    switch (value) {
    case 0: return "#CDC1B4";
    case 2: return "#EEE4DA";
    case 4: return "#EDE0C8";
    case 8: return "#F2B179";
    case 16: return "#F59563";
    case 32: return "#F67C5F";
    case 64: return "#F65E3B";
    case 128: return "#EDCF72";
    case 256: return "#EDCC61";
    case 512: return "#EDC850";
    case 1024: return "#EDC53F";
    case 2048: return "#EDC22E";
    default: return "#3C3A32";
    }
}

QString MainWindow::getTextColor(int value)
{
    return (value <= 4) ? "#776E65" : "#F9F6F2";
}

// 开始所有动画
void MainWindow::startAnimations()
{
    // 如果没有需要动画的方块，直接返回
    if (m_tileMovements.isEmpty() && m_mergedTiles.isEmpty() && m_newTiles.isEmpty()) {
        return;
    }
    
    // 清空之前的动画组
    m_animationGroup->clear();
    
    // 标记动画开始运行
    m_animationRunning = true;
    
    // 首先执行移动动画
    for (const TileMovement &move : m_tileMovements) {
        animateTileMovement(move.fromRow, move.fromCol, move.toRow, move.toCol);
    }
    
    // 然后执行合并动画
    for (const QPair<int, int> &pos : m_mergedTiles) {
        animateTileMerge(pos.first, pos.second);
    }
    
    // 最后执行新方块出现的动画
    for (const QPair<int, int> &pos : m_newTiles) {
        animateNewTile(pos.first, pos.second);
    }
    
    // 开始动画
    m_animationGroup->start();
}

// 方块移动动画
void MainWindow::animateTileMovement(int fromRow, int fromCol, int toRow, int toCol)
{
    // 创建一个临时标签用于动画
    QLabel *tempLabel = new QLabel(this);
    tempLabel->setAlignment(Qt::AlignCenter);
    tempLabel->setFont(QFont("Arial", 20, QFont::Bold));
    
    // 设置临时标签的初始位置和大小
    QRect fromRect = m_tiles[fromRow][fromCol]->geometry();
    QRect toRect = m_tiles[toRow][toCol]->geometry();
    tempLabel->setGeometry(fromRect);
    
    // 设置临时标签的样式和文本
    int value = m_previousBoard[fromRow][fromCol];
    updateTileAppearance(tempLabel, value);
    
    // 将临时标签添加到中央部件
    tempLabel->setParent(m_centralWidget);
    tempLabel->show();
    
    // 创建位置动画
    QPropertyAnimation *animation = new QPropertyAnimation(tempLabel, "geometry");
    animation->setDuration(200); // 200毫秒的动画时间
    animation->setStartValue(fromRect);
    animation->setEndValue(toRect);
    animation->setEasingCurve(QEasingCurve::OutQuad); // 使用平滑的缓动曲线
    
    // 动画结束后删除临时标签
    connect(animation, &QPropertyAnimation::finished, tempLabel, &QLabel::deleteLater);
    
    // 将动画添加到动画组
    m_animationGroup->addAnimation(animation);
    
    // 隐藏原始方块
    m_tiles[fromRow][fromCol]->setText("");
    m_tiles[fromRow][fromCol]->setStyleSheet(getTileStyleSheet(0));
}

// 方块合并动画
void MainWindow::animateTileMerge(int row, int col)
{
    // 创建一个缩放动画
    QPropertyAnimation *scaleAnimation = new QPropertyAnimation(m_tiles[row][col], "geometry");
    QRect originalRect = m_tiles[row][col]->geometry();
    QRect largerRect = originalRect;
    largerRect.setLeft(originalRect.left() - 5);
    largerRect.setTop(originalRect.top() - 5);
    largerRect.setRight(originalRect.right() + 5);
    largerRect.setBottom(originalRect.bottom() + 5);
    
    scaleAnimation->setDuration(150); // 150毫秒的动画时间
    scaleAnimation->setStartValue(originalRect);
    scaleAnimation->setEndValue(largerRect);
    scaleAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    // 创建第二个动画，恢复原始大小
    QPropertyAnimation *scaleBackAnimation = new QPropertyAnimation(m_tiles[row][col], "geometry");
    scaleBackAnimation->setDuration(150); // 150毫秒的动画时间
    scaleBackAnimation->setStartValue(largerRect);
    scaleBackAnimation->setEndValue(originalRect);
    scaleBackAnimation->setEasingCurve(QEasingCurve::InQuad);
    
    // 创建顺序动画组
    QSequentialAnimationGroup *mergeAnimation = new QSequentialAnimationGroup();
    mergeAnimation->addAnimation(scaleAnimation);
    mergeAnimation->addAnimation(scaleBackAnimation);
    
    // 将合并动画添加到主动画组
    m_animationGroup->addAnimation(mergeAnimation);
    
    // 更新方块外观
    int value = m_game->tileAt(row, col);
    updateTileAppearance(m_tiles[row][col], value);
}

// 新方块出现动画
void MainWindow::animateNewTile(int row, int col)
{
    // 创建一个不透明度动画
    QLabel *tile = m_tiles[row][col];
    tile->setStyleSheet(getTileStyleSheet(0)); // 先设置为空方块样式
    
    // 创建一个临时标签用于动画
    QLabel *tempLabel = new QLabel(this);
    tempLabel->setAlignment(Qt::AlignCenter);
    tempLabel->setFont(QFont("Arial", 20, QFont::Bold));
    
    // 设置临时标签的位置和大小
    QRect originalRect = tile->geometry();
    
    // 计算缩放起始矩形（从中心点开始，初始大小为0）
    QRect startRect = originalRect;
    int centerX = originalRect.x() + originalRect.width() / 2;
    int centerY = originalRect.y() + originalRect.height() / 2;
    startRect.setLeft(centerX);
    startRect.setTop(centerY);
    startRect.setRight(centerX);
    startRect.setBottom(centerY);
    
    tempLabel->setGeometry(startRect);
    
    // 设置临时标签的样式和文本
    int value = m_game->tileAt(row, col);
    updateTileAppearance(tempLabel, value);
    
    // 设置初始透明度
    QString styleSheet = tempLabel->styleSheet();
    styleSheet.replace("background-color:", "background-color: rgba(238, 228, 218, 0);");
    styleSheet.replace("color:", "color: rgba");
    tempLabel->setStyleSheet(styleSheet);
    
    // 将临时标签添加到中央部件
    tempLabel->setParent(m_centralWidget);
    tempLabel->show();
    
    // 创建缩放动画
    QPropertyAnimation *scaleAnimation = new QPropertyAnimation(tempLabel, "geometry");
    scaleAnimation->setDuration(200); // 200毫秒的动画时间
    scaleAnimation->setStartValue(startRect);
    scaleAnimation->setEndValue(originalRect);
    scaleAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    // 创建不透明度动画（通过样式表变化实现）
    QPropertyAnimation *opacityAnimation = new QPropertyAnimation(tempLabel, "styleSheet");
    opacityAnimation->setDuration(200); // 200毫秒的动画时间
    opacityAnimation->setStartValue(styleSheet);
    
    // 创建结束时的样式表（完全不透明）
    QString endStyleSheet = tempLabel->styleSheet();
    endStyleSheet.replace("rgba(238, 228, 218, 0)", "rgba(238, 228, 218, 1)");
    endStyleSheet.replace("color: rgba", "color:");
    opacityAnimation->setEndValue(endStyleSheet);
    opacityAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    // 创建并行动画组，同时执行缩放和不透明度动画
    QParallelAnimationGroup *newTileAnimation = new QParallelAnimationGroup();
    newTileAnimation->addAnimation(scaleAnimation);
    newTileAnimation->addAnimation(opacityAnimation);
    
    // 动画结束后删除临时标签，并显示实际方块
    connect(newTileAnimation, &QParallelAnimationGroup::finished, this, [this, tempLabel, tile, value]() {
        tempLabel->deleteLater();
        updateTileAppearance(tile, value);
    });
    
    // 将新方块动画添加到主动画组
    m_animationGroup->addAnimation(newTileAnimation);
}
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // 如果动画正在运行，阻止所有键盘事件
    if (m_animationRunning && event->type() == QEvent::KeyPress) {
        return true; // 阻止事件传递
    }
    
    // 捕获键盘事件
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        
        // 处理方向键
        switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
            keyPressEvent(keyEvent);
            return true; // 事件已处理
        default:
            break;
        }
    }
    
    // 其他事件交给默认处理
    return QMainWindow::eventFilter(watched, event);
}
