#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <stdbool.h>

// ---------- 游戏配置（您随意修改）----------
#define BOARD_WIDTH  15      // 宽度（格）
#define BOARD_HEIGHT 30      // 高度（格）
#define BLOCK_SIZE   2       // 每格字符宽度（用于■）
#define INIT_SPEED   300     // 初始下落间隔（毫秒）

// ---------- 方块形状 ----------
const int SHAPES[7][4][4][4] = {
    // I
    {{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
     {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
     {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
     {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}},
    // O
    {{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}}},
    // T
    {{{0,0,0,0},{0,1,0,0},{1,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,0,0},{0,1,1,0},{0,1,0,0}},
     {{0,0,0,0},{0,0,0,0},{1,1,1,0},{0,1,0,0}},
     {{0,0,0,0},{0,1,0,0},{1,1,0,0},{0,1,0,0}}},
    // S
    {{{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,0,0},{0,1,1,0},{0,0,1,0}},
     {{0,0,0,0},{0,0,0,0},{0,1,1,0},{1,1,0,0}},
     {{0,0,0,0},{1,0,0,0},{1,1,0,0},{0,1,0,0}}},
    // Z
    {{{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,0,1,0},{0,1,1,0},{0,1,0,0}},
     {{0,0,0,0},{0,0,0,0},{1,1,0,0},{0,1,1,0}},
     {{0,0,0,0},{0,1,0,0},{1,1,0,0},{1,0,0,0}}},
    // L
    {{{0,0,0,0},{0,0,1,0},{1,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,0,0},{0,1,0,0},{0,1,1,0}},
     {{0,0,0,0},{0,0,0,0},{1,1,1,0},{1,0,0,0}},
     {{0,0,0,0},{1,1,0,0},{0,1,0,0},{0,1,0,0}}},
    // J
    {{{0,0,0,0},{1,0,0,0},{1,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,0,0},{0,1,0,0}},
     {{0,0,0,0},{0,0,0,0},{1,1,1,0},{0,0,1,0}},
     {{0,0,0,0},{0,1,0,0},{0,1,0,0},{1,1,0,0}}}
};

const int COLORS[7] = {11, 14, 13, 10, 12, 9, 6};//方块颜色

typedef struct {//游戏结构体，用来保存游戏状态。
    int board[BOARD_HEIGHT][BOARD_WIDTH];//游戏盘面。 
    int currentType, currentRot, currentX, currentY;//当前方块类型，当前方块旋转角度，当前方块x坐标，当前方块y坐标
    int nextType;//下一个方块类型
    int score, level, speed;//得分，等级，速度
    bool gameOver, pause;//游戏是否结束，是否暂停
    
    // ----- 增量绘制辅助变量 -----
    int lastScore, lastLevel, lastSpeed, lastNextType; // 记录上次绘制值
    int oldX, oldY, oldRot, oldType;                  // 记录当前方块的旧位置（用于清除）
} Game;


void setColor(int color);//设置颜色。
void showStartScreen();//显示开始界面。
void gotoxy(int x, int y);//移动光标。
void hideCursor();//隐藏光标。
void showCursor();//显示光标。
void drawCell(int x, int y, int color, const char* str);//绘制单个格子。
void drawFixedBoard(Game *game);//绘制固定方块。
void clearCurrentPiece(Game *game);//清除当前方块。
void drawCurrentPiece(Game *game);//绘制当前方块。
void drawBorder();//绘制边框。
void drawInfo(Game *game);//绘制右侧信息面板。
void initGame(Game *game);//初始化游戏。
bool checkCollision(Game *game, int x, int y, int type, int rot);//碰撞检测。
void lockPiece(Game *game);//固定方块。
void clearLines(Game *game);//消除满行。
void spawnNewPiece(Game *game);//生成新方块。
void hardDrop(Game *game);//硬降。
void processInput(Game *game);//处理输入。
void gameLoop(Game *game);//游戏循环。
void showEndScreen(Game *game);//显示结束界面。

int main() {

    // 动态窗口大小，根据游戏盘面大小和边框宽度计算。
    int windowWidth  = BOARD_WIDTH * BLOCK_SIZE + 45;//窗口宽度。BOARD_WIDTH是游戏盘面宽度，BLOCK_SIZE是每个方块的宽度，45是边框宽度。
    int windowHeight = BOARD_HEIGHT + 20;//窗口高度。BOARD_HEIGHT是游戏盘面高度，20是边框高度。
    char modeCmd[100];//定义一个字符串，这个字符串用来构建设置窗口大小命令。
    sprintf(modeCmd, "mode con cols=%d lines=%d", windowWidth, windowHeight);//构建命令。
    system(modeCmd);//执行命令。
    system("title 俄罗斯方块");//设置窗口标题。
    
    showStartScreen();//显示开始界面。本质就是一堆printf。
    Game game;//定义一个游戏结构体变量。
    
    while (1) {//无限循环，直到游戏结束。
        initGame(&game);//初始化游戏。
        hideCursor();//隐藏光标。
        system("cls");//清空屏幕。
        // 一次性绘制所有静态元素
        drawBorder();//绘制边框。
        drawFixedBoard(&game);//绘制固定方块。
        drawCurrentPiece(&game);//绘制当前方块。
        drawInfo(&game);  // 内部会处理静态文字的首次绘制
        
        gameLoop(&game);//游戏循环。
        
        showEndScreen(&game);//显示结束界面。
        int key = _getch();
        if (key != 'r' && key != 'R') break;
    }
    
    showCursor();
    system("cls");
    return 0;
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void showStartScreen() {//显示开始界面。
    system("cls");//清空屏幕。

    setColor(14);//设置颜色。
    printf("\n\n");//打印空行。
    printf("    ██████╗  ██████╗ ███████╗███████╗██╗ █████╗ \n");//打印标题。
    printf("    ██╔══██╗██╔═══██╗██╔════╝██╔════╝██║██╔══██╗\n");//打印标题。
    printf("    ██████╔╝██║   ██║███████╗█████╗  ██║███████║\n");//打印标题。
    printf("    ██╔══██╗██║   ██║╚════██║██╔══╝  ██║██╔══██║\n");//打印标题。
    printf("    ██║  ██║╚██████╔╝███████║██║     ██║██║  ██║\n");//打印标题。
    printf("    ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝\n\n");//打印标题。
    
    setColor(11);//设置颜色。
    printf("          命令行版 - 经典俄罗斯方块\n\n");//打印标题。
    
    setColor(7);//设置颜色。
    printf("游戏规则:\n");//打印标题。
    printf("1. 方向键/WASD控制方块移动、旋转\n");//打印标题。
    printf("2. 空格键直接落底\n");//打印标题。
    printf("3. 消除行获得分数，等级越高速度越快\n\n");//打印标题。
    printf("当前游戏区域: %d x %d\n\n", BOARD_WIDTH, BOARD_HEIGHT);//打印标题。
    
    setColor(10);//设置颜色。
    printf("按任意键开始...");//打印标题。
    _getch();//等待用户按任意键。
}

// ---------- 控制台工具 ----------
void gotoxy(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursor_info = {100, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void showCursor() {
    CONSOLE_CURSOR_INFO cursor_info = {100, TRUE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}



// ---------- 绘制单个格子 ----------
void drawCell(int x, int y, int color, const char* str) {
    gotoxy(x, y);
    setColor(color);
    printf("%s", str);
}

// ---------- 绘制固定方块（整个 board）----------
void drawFixedBoard(Game *game) {
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            int x = col * BLOCK_SIZE + 1;
            int y = row + 1;
            if (game->board[row][col] != 0)
                drawCell(x, y, game->board[row][col], "■");
            else
                drawCell(x, y, 8, "·");
        }
    }
}

// ---------- 清除当前方块的旧位置（用背景点覆盖）----------
void clearCurrentPiece(Game *game) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[game->oldType][game->oldRot][i][j]) {
                int x = (game->oldX + j) * BLOCK_SIZE + 1;
                int y = game->oldY + i + 1;
                if (y > 0 && y <= BOARD_HEIGHT)
                    drawCell(x, y, 8, "·");
            }
        }
    }
}

// ---------- 绘制当前方块（新位置）----------
void drawCurrentPiece(Game *game) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[game->currentType][game->currentRot][i][j]) {
                int x = (game->currentX + j) * BLOCK_SIZE + 1;
                int y = game->currentY + i + 1;
                if (y > 0 && y <= BOARD_HEIGHT)
                    drawCell(x, y, COLORS[game->currentType], "■");
            }
        }
    }
}

// ---------- 绘制边框（只画一次）----------
void drawBorder() {
    setColor(7);
    gotoxy(0, 0);
    for (int i = 0; i < (BOARD_WIDTH * BLOCK_SIZE + 2); i++) printf("▄");
    gotoxy(0, BOARD_HEIGHT + 1);
    for (int i = 0; i < (BOARD_WIDTH * BLOCK_SIZE + 2); i++) printf("▀");
    for (int i = 1; i <= BOARD_HEIGHT; i++) {
        gotoxy(0, i); printf("█");
        gotoxy(BOARD_WIDTH * BLOCK_SIZE + 1, i); printf("█");
    }
}

// ---------- 右侧信息面板（增量更新）----------
void drawInfo(Game *game) {
    int infoX = BOARD_WIDTH * BLOCK_SIZE + 5;
    int baseY = 2;

    // 标题（只画一次）
    static bool titleDrawn = false;
    if (!titleDrawn) {
        setColor(14);
        gotoxy(infoX, baseY); printf("俄罗斯方块");
        titleDrawn = true;
    }

    // 得分：仅变化时重绘
    if (game->score != game->lastScore) {
        setColor(7);
        gotoxy(infoX, baseY + 2); printf("得分: %d    ", game->score);
        game->lastScore = game->score;
    }
    // 等级：仅变化时重绘
    if (game->level != game->lastLevel) {
        setColor(7);
        gotoxy(infoX, baseY + 3); printf("等级: %d    ", game->level);
        game->lastLevel = game->level;
    }
    // 速度：仅变化时重绘
    if (game->speed != game->lastSpeed) {
        setColor(7);
        gotoxy(infoX, baseY + 4); printf("速度: %d ms ", game->speed);
        game->lastSpeed = game->speed;
    }

    // “下一个:” 静态文字只画一次
    static bool nextLabelDrawn = false;
    if (!nextLabelDrawn) {
        setColor(7);
        gotoxy(infoX, baseY + 6); printf("下一个:");
        nextLabelDrawn = true;
    }

    // 下一个方块预览（仅当类型变化时重绘）
    if (game->nextType != game->lastNextType) {
        int previewY = baseY + 8;
        // 清除旧的预览区
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int x = infoX + j * 2;
                int y = previewY + i;
                drawCell(x, y, 8, "·");
            }
        }
        // 绘制新的预览
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int x = infoX + j * 2;
                int y = previewY + i;
                if (SHAPES[game->nextType][0][i][j])
                    drawCell(x, y, COLORS[game->nextType], "■");
            }
        }
        game->lastNextType = game->nextType;
    }

    // 控制说明（只画一次）
    static bool ctrlDrawn = false;
    if (!ctrlDrawn) {
        int ctrlY = baseY + 8 + 6;
        setColor(11);
        gotoxy(infoX, ctrlY);     printf("← → : 移动");
        gotoxy(infoX, ctrlY + 1); printf("↑   : 旋转");
        gotoxy(infoX, ctrlY + 2); printf("↓   : 加速");
        gotoxy(infoX, ctrlY + 3); printf("空格 : 硬降");
        gotoxy(infoX, ctrlY + 4); printf("P   : 暂停");
        gotoxy(infoX, ctrlY + 5); printf("X   : 退出");
        ctrlDrawn = true;
    }

    // 暂停提示（动态）
    static bool wasPaused = false;
    if (game->pause != wasPaused) {
        int ctrlY = baseY + 8 + 6;
        if (game->pause) {
            setColor(12);
            gotoxy(infoX, ctrlY + 7); printf("  暂停中  ");
        } else {
            gotoxy(infoX, ctrlY + 7); printf("          ");
        }
        wasPaused = game->pause;
    }
}

// ---------- 初始化 ----------
void initGame(Game *game) {
    for (int i = 0; i < BOARD_HEIGHT; i++)
        for (int j = 0; j < BOARD_WIDTH; j++)
            game->board[i][j] = 0;
    
    srand((unsigned)time(NULL));
    game->currentType = rand() % 7;
    game->currentRot = 0;
    game->currentX = BOARD_WIDTH / 2 - 2;
    game->currentY = 0;
    game->nextType = rand() % 7;
    game->score = 0;
    game->level = 1;
    game->speed = INIT_SPEED;
    game->gameOver = false;
    game->pause = false;
    
    // 增量绘制辅助变量初始化
    game->lastScore = -1;
    game->lastLevel = -1;
    game->lastSpeed = -1;
    game->lastNextType = -1;
    game->oldX = game->currentX;
    game->oldY = game->currentY;
    game->oldRot = game->currentRot;
    game->oldType = game->currentType;
}

// ---------- 碰撞检测 ----------
bool checkCollision(Game *game, int x, int y, int type, int rot) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[type][rot][i][j]) {
                int bx = x + j;
                int by = y + i;
                if (bx < 0 || bx >= BOARD_WIDTH || by >= BOARD_HEIGHT) return true;
                if (by >= 0 && game->board[by][bx] != 0) return true;
            }
        }
    }
    return false;
}

// ---------- 固定方块（并绘制固定部分）----------
void lockPiece(Game *game) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[game->currentType][game->currentRot][i][j]) {
                int y = game->currentY + i;
                int x = game->currentX + j;
                if (y >= 0 && y < BOARD_HEIGHT && x >= 0 && x < BOARD_WIDTH) {
                    game->board[y][x] = COLORS[game->currentType];
                    // 立即绘制这个被固定的格子
                    int px = x * BLOCK_SIZE + 1;
                    int py = y + 1;
                    drawCell(px, py, COLORS[game->currentType], "■");
                }
            }
        }
    }
}

// ---------- 消除满行（并重绘受影响的区域）----------
void clearLines(Game *game) {
    int lines = 0;
    for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
        bool full = true;
        for (int col = 0; col < BOARD_WIDTH; col++)
            if (game->board[row][col] == 0) { full = false; break; }
        if (full) {
            lines++;
            // 上方向下移动
            for (int r = row; r > 0; r--)
                for (int c = 0; c < BOARD_WIDTH; c++)
                    game->board[r][c] = game->board[r - 1][c];
            for (int c = 0; c < BOARD_WIDTH; c++)
                game->board[0][c] = 0;
            row++; // 重新检查同一行
        }
    }
    // 消行后整个板子变化较大，直接重绘所有固定方块
    if (lines > 0) {
        drawFixedBoard(game);
    }
    
    if (lines == 1) game->score += 100;
    else if (lines == 2) game->score += 300;
    else if (lines == 3) game->score += 500;
    else if (lines == 4) game->score += 800;
    
    int newLevel = game->score / 500 + 1;
    if (newLevel > game->level) {
        game->level = newLevel;
        game->speed = INIT_SPEED / (newLevel * 0.5 + 0.5);
        if (game->speed < 50) game->speed = 50;
    }
}

// ---------- 生成新方块 ----------
void spawnNewPiece(Game *game) {
    game->currentType = game->nextType;
    game->currentRot = 0;
    game->currentX = BOARD_WIDTH / 2 - 2;
    game->currentY = 0;
    game->nextType = rand() % 7;
    game->oldX = game->currentX;
    game->oldY = game->currentY;
    game->oldRot = game->currentRot;
    game->oldType = game->currentType;
    
    if (checkCollision(game, game->currentX, game->currentY,
                       game->currentType, game->currentRot))
        game->gameOver = true;
    else
        drawCurrentPiece(game); // 直接绘制新方块
}

// ---------- 硬降 ----------
void hardDrop(Game *game) {
    // 清除旧位置
    clearCurrentPiece(game);
    
    while (!checkCollision(game, game->currentX, game->currentY + 1,
                           game->currentType, game->currentRot))
        game->currentY++;
    
    // 更新记录的位置（用于下次清除）
    game->oldX = game->currentX;
    game->oldY = game->currentY;
    game->oldRot = game->currentRot;
    game->oldType = game->currentType;
    
    lockPiece(game);
    clearLines(game);
    spawnNewPiece(game);
}

// ---------- 键盘输入 ----------
void processInput(Game *game) {
    if (_kbhit()) {
        int key = _getch();
        if (key == 224) {
            key = _getch();
            switch (key) {
                case 75: // ←
                    if (!checkCollision(game, game->currentX - 1, game->currentY,
                                        game->currentType, game->currentRot)) {
                        clearCurrentPiece(game);
                        game->currentX--;
                        drawCurrentPiece(game);
                        // 更新旧位置记录
                        game->oldX = game->currentX;
                        game->oldY = game->currentY;
                        game->oldRot = game->currentRot;
                        game->oldType = game->currentType;
                    }
                    break;
                case 77: // →
                    if (!checkCollision(game, game->currentX + 1, game->currentY,
                                        game->currentType, game->currentRot)) {
                        clearCurrentPiece(game);
                        game->currentX++;
                        drawCurrentPiece(game);
                        game->oldX = game->currentX;
                        game->oldY = game->currentY;
                        game->oldRot = game->currentRot;
                        game->oldType = game->currentType;
                    }
                    break;
                case 72: // ↑ 旋转
                    {
                        int newRot = (game->currentRot + 1) % 4;
                        if (!checkCollision(game, game->currentX, game->currentY,
                                            game->currentType, newRot)) {
                            clearCurrentPiece(game);
                            game->currentRot = newRot;
                            drawCurrentPiece(game);
                            game->oldX = game->currentX;
                            game->oldY = game->currentY;
                            game->oldRot = game->currentRot;
                            game->oldType = game->currentType;
                        }
                    }
                    break;
                case 80: // ↓ 加速
                    if (!checkCollision(game, game->currentX, game->currentY + 1,
                                        game->currentType, game->currentRot)) {
                        clearCurrentPiece(game);
                        game->currentY++;
                        drawCurrentPiece(game);
                        game->oldX = game->currentX;
                        game->oldY = game->currentY;
                        game->oldRot = game->currentRot;
                        game->oldType = game->currentType;
                    }
                    break;
            }
        } else {
            switch (key) {
                case 'a': case 'A':
                    if (!checkCollision(game, game->currentX - 1, game->currentY,
                                        game->currentType, game->currentRot)) {
                        clearCurrentPiece(game);
                        game->currentX--;
                        drawCurrentPiece(game);
                        game->oldX = game->currentX;
                        game->oldY = game->currentY;
                        game->oldRot = game->currentRot;
                        game->oldType = game->currentType;
                    }
                    break;
                case 'd': case 'D':
                    if (!checkCollision(game, game->currentX + 1, game->currentY,
                                        game->currentType, game->currentRot)) {
                        clearCurrentPiece(game);
                        game->currentX++;
                        drawCurrentPiece(game);
                        game->oldX = game->currentX;
                        game->oldY = game->currentY;
                        game->oldRot = game->currentRot;
                        game->oldType = game->currentType;
                    }
                    break;
                case 'w': case 'W':
                    {
                        int newRot = (game->currentRot + 1) % 4;
                        if (!checkCollision(game, game->currentX, game->currentY,
                                            game->currentType, newRot)) {
                            clearCurrentPiece(game);
                            game->currentRot = newRot;
                            drawCurrentPiece(game);
                            game->oldX = game->currentX;
                            game->oldY = game->currentY;
                            game->oldRot = game->currentRot;
                            game->oldType = game->currentType;
                        }
                    }
                    break;
                case 's': case 'S':
                    if (!checkCollision(game, game->currentX, game->currentY + 1,
                                        game->currentType, game->currentRot)) {
                        clearCurrentPiece(game);
                        game->currentY++;
                        drawCurrentPiece(game);
                        game->oldX = game->currentX;
                        game->oldY = game->currentY;
                        game->oldRot = game->currentRot;
                        game->oldType = game->currentType;
                    }
                    break;
                case ' ':
                    hardDrop(game);
                    break;
                case 'p': case 'P':
                    game->pause = !game->pause;
                    break;
                case 'x': case 'X':
                    game->gameOver = true;
                    break;
            }
        }
    }
}

// ---------- 主循环 ----------
void gameLoop(Game *game) {
    int frameCounter = 0;
    while (!game->gameOver) {
        if (!game->pause) {
            processInput(game);
            
            frameCounter++;
            if (frameCounter >= (game->speed / 20)) {
                frameCounter = 0;
                if (!checkCollision(game, game->currentX, game->currentY + 1,
                                    game->currentType, game->currentRot)) {
                    // 自动下落：清除旧位置，移动，绘制新位置
                    clearCurrentPiece(game);
                    game->currentY++;
                    drawCurrentPiece(game);
                    game->oldX = game->currentX;
                    game->oldY = game->currentY;
                    game->oldRot = game->currentRot;
                    game->oldType = game->currentType;
                } else {
                    // 无法下落：固定，消行，生成新块
                    lockPiece(game);
                    clearLines(game);
                    spawnNewPiece(game);
                }
            }
            
            // 信息面板增量更新
            drawInfo(game);
        } else {
            drawInfo(game);
            processInput(game);
        }
        Sleep(20);
    }
}



// ---------- 结束界面 ----------
void showEndScreen(Game *game) {
    system("cls");
    setColor(12);
    printf("\n\n");
    printf("     ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ \n");
    printf("    ██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗\n");
    printf("    ██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝\n");
    printf("    ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗\n");
    printf("    ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝ ╚████╔╝ ███████╗██║  ██║\n");
    printf("     ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝\n\n");
    setColor(14);
    printf("                         最终得分: %d\n", game->score);
    printf("                         到达等级: %d\n\n", game->level);
    setColor(7);
    printf("                按 R 重新开始，其他键退出...");
}

// ---------- main ----------
