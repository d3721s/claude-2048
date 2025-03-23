#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_game(new Game2048(this))
{
    setupUi();
    
    // 连接信号和槽
    connect(m_game, &Game2048::boardChanged, this, &MainWindow::updateBoard);
    connect(m_game, &Game2048::scoreChanged, this, &MainWindow::updateScore);
    connect(m_game, &Game2048::gameOver, this, &MainWindow::handleGameOver);
    connect(m_newGameButton, &QPushButton::clicked, m_game, &Game2048::newGame);
    
    // 初始化游戏界面
    updateBoard();
    updateScore(0);
    
    // 确保窗口获得焦点
    this->setFocus();
    this->activateWindow();
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
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_game->isGameOver()) {
        return;
    }
    
    switch (event->key()) {
    case Qt::Key_Up:
        m_game->move(Game2048::Direction::Up);
        break;
    case Qt::Key_Down:
        m_game->move(Game2048::Direction::Down);
        break;
    case Qt::Key_Left:
        m_game->move(Game2048::Direction::Left);
        break;
    case Qt::Key_Right:
        m_game->move(Game2048::Direction::Right);
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::updateBoard()
{
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int value = m_game->tileAt(row, col);
            updateTileAppearance(m_tiles[row][col], value);
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