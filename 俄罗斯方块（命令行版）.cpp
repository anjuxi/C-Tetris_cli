#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <stdbool.h>

// ---------- 游戏配置（您随意修改）----------
#define BOARD_WIDTH  15      // 宽度（格）。
#define BOARD_HEIGHT 30      // 高度（格）。
#define BLOCK_SIZE   2       // 每格字符宽度（用于■）。
#define INIT_SPEED   300     // 初始下落间隔（毫秒）。
#define LOCK_DELAY   500     // 触底后的锁定缓冲时间（毫秒），留出微调余地。
#define HIGH_SCORE_FILE "tetris_highscore.txt"  // 最高分存档文件名，保存在运行目录。

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

const int COLORS[7] = {11, 14, 13, 10, 12, 9, 6};// 方块颜色，按 I、O、T、S、Z、L、J 排列。

typedef struct {
    int board[BOARD_HEIGHT][BOARD_WIDTH];// 0 是空地，其余存的是颜色值。
    int currentType, currentRot, currentX, currentY;
    int nextType, holdType;// holdType 为 -1 表示暂存区空着。
    int score, level, speed, lines, highScore;
    bool gameOver, pause, canHold, quit;
    int dropTimer, lockTimer;

    int bag[7], bagIndex;// 7-bag 的牌堆，一轮里七种各一张。

    // 增量绘制要拿上一帧的状态做比较，下面这些就是干这个的。
    int lastScore, lastLevel, lastSpeed, lastNextType, lastHoldType, lastLines, lastDrawScore;
    int oldX, oldY, oldRot, oldType, oldGhostY;
    bool titleDrawn, nextLabelDrawn, holdLabelDrawn, ctrlDrawn, wasPaused;
} Game;

static bool g_silent = false;// 自检时置真，绘制函数直接跳过。
static bool g_highScoreDirty = false;// 这局有没有破纪录。

void setupConsole(void);
bool readKey(int *out);// 拿到真实按键返回 true，窗口缩放之类的事件会被丢掉。
int  waitForKey(void);
void setColor(int color);
void gotoxy(int x, int y);
void hideCursor(void);
void showCursor(void);
void drawCell(int x, int y, int color, const char* str);
int  loadHighScore(void);
void saveHighScore(int score);
void showStartScreen(void);
void showEndScreen(Game *game);
void drawBorder(void);
void drawFixedBoard(Game *game);
void drawPieceCells(int type, int rot, int x, int y, int color);
void drawGhostPiece(Game *game);
void drawCurrentPiece(Game *game);
void eraseOldPiece(Game *game);
void syncPieceRecord(Game *game);
void refreshPiece(Game *game);
int  getGhostY(Game *game);
void drawPreview(int type, int px, int py);
void drawInfo(Game *game);
void fillBag(Game *game);
int  takeFromBag(Game *game);
void initGame(Game *game);
bool checkCollision(Game *game, int x, int y, int type, int rot);
bool tryMove(Game *game, int dx, int dy);
bool tryRotate(Game *game);
void lockPiece(Game *game);
void clearLines(Game *game);
void spawnNewPiece(Game *game);
void holdPiece(Game *game);
void hardDrop(Game *game);
void processInput(Game *game);
void gameLoop(Game *game);
int  runSelfTest(void);

int main(int argc, char *argv[]) {

    if (argc > 1 && strcmp(argv[1], "--test") == 0)// 只跑自检，不进游戏。
        return runSelfTest();

    srand((unsigned)time(NULL));// 种子只播一次，重开才有新序列。
    setupConsole();

    Game game;
    game.highScore = loadHighScore();

    showStartScreen();
    while (1) {
        initGame(&game);
        hideCursor();
        system("cls");
        drawBorder();
        drawFixedBoard(&game);
        drawGhostPiece(&game);
        drawCurrentPiece(&game);
        syncPieceRecord(&game);
        drawInfo(&game);

        gameLoop(&game);

        if (game.score > game.highScore) {// 破纪录才写盘。
            game.highScore = game.score;
            saveHighScore(game.highScore);
            g_highScoreDirty = true;
        }

        showEndScreen(&game);
        if (game.quit) break;// 按过 X 就直接走，别再看结算页。

        int key = waitForKey();
        if (key != 'r' && key != 'R') break;
    }

    showCursor();
    system("cls");
    return 0;
}

void setupConsole(void) {
    SetConsoleOutputCP(CP_UTF8);// 源文件是 UTF-8，控制台不切过来就是乱码。
    SetConsoleTitleW(L"俄罗斯方块");

    int windowWidth = BOARD_WIDTH * BLOCK_SIZE + 45;// 右边留出面板的位置。
    int windowHeight = BOARD_HEIGHT + 3;

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD maxBuf = { (SHORT)windowWidth, (SHORT)(windowHeight + 20) };
    SetConsoleScreenBufferSize(hOut, maxBuf);// 缓冲区先撑大，不然窗口调不高。

    SMALL_RECT rect = { 0, 0, (SHORT)(windowWidth - 1), (SHORT)(windowHeight - 1) };
    if (!SetConsoleWindowInfo(hOut, TRUE, &rect)) {// 有些环境不让改窗口，退回 mode 命令试试。
        char modeCmd[100];
        sprintf(modeCmd, "mode con cols=%d lines=%d", windowWidth, windowHeight);
        system(modeCmd);
    }

    COORD buf = { (SHORT)windowWidth, (SHORT)windowHeight };
    SetConsoleScreenBufferSize(hOut, buf);// 缓冲区和窗口一样高，画面就不会因为写到窗口外往上滚。

    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));// 清掉启动前残留的按键。
}

// ---------- 输入处理 ----------
bool readKey(int *out) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD count = 0;
    if (!GetNumberOfConsoleInputEvents(hIn, &count) || count == 0) return false;

    INPUT_RECORD rec;
    DWORD read = 0;
    if (!ReadConsoleInput(hIn, &rec, 1, &read) || read == 0) return false;

    if (rec.EventType != KEY_EVENT) return false;// 窗口缩放、鼠标之类的不是按键，不要。
    if (!rec.Event.KeyEvent.bKeyDown) return false;// 抬键也跳过，只认按下。

    WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
    char ch = rec.Event.KeyEvent.uChar.AsciiChar;
    if (ch == 0) *out = 0x100 + vk;// 方向键没有字符，用虚拟键码加个偏移区分开。
    else *out = (unsigned char)ch;
    return true;
}

int waitForKey(void) {
    int key = 0;
    while (!readKey(&key)) Sleep(20);
    return key;
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void gotoxy(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor(void) {
    CONSOLE_CURSOR_INFO cursor_info = {100, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void showCursor(void) {
    CONSOLE_CURSOR_INFO cursor_info = {100, TRUE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void drawCell(int x, int y, int color, const char* str) {
    if (g_silent) return;
    gotoxy(x, y);
    setColor(color);
    printf("%s", str);
}

// ---------- 最高分存档 ----------
int loadHighScore(void) {
    FILE *fp = fopen(HIGH_SCORE_FILE, "r");
    int score = 0;
    if (fp == NULL) return 0;
    if (fscanf(fp, "%d", &score) != 1) score = 0;// 文件被改坏了也当没记录。
    fclose(fp);
    return score;
}

void saveHighScore(int score) {
    FILE *fp = fopen(HIGH_SCORE_FILE, "w");
    if (fp == NULL) return;// 没写权限就算了，不影响玩。
    fprintf(fp, "%d\n", score);
    fclose(fp);
}

void showStartScreen(void) {
    system("cls");

    setColor(14);
    printf("\n\n");
    printf("    ██████╗  ██████╗ ███████╗███████╗██╗ █████╗ \n");
    printf("    ██╔══██╗██╔═══██╗██╔════╝██╔════╝██║██╔══██╗\n");
    printf("    ██████╔╝██║   ██║███████╗█████╗  ██║███████║\n");
    printf("    ██╔══██╗██║   ██║╚════██║██╔══╝  ██║██╔══██║\n");
    printf("    ██║  ██║╚██████╔╝███████║██║     ██║██║  ██║\n");
    printf("    ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝\n\n");

    setColor(11);
    printf("          命令行版 - 经典俄罗斯方块\n\n");

    setColor(7);
    printf("游戏规则:\n");
    printf("1. 方向键/WASD 控制方块移动、旋转，空格直接落底\n");
    printf("2. C 键暂存方块，P 键暂停，X 键退出\n");
    printf("3. 消除行获得分数，每消 10 行升一级，等级越高速度越快\n");
    printf("4. 灰色轮廓是落点提示，可以提前判断位置\n\n");
    printf("当前游戏区域: %d x %d\n\n", BOARD_WIDTH, BOARD_HEIGHT);

    setColor(10);
    printf("按任意键开始...");
    waitForKey();
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

void drawPieceCells(int type, int rot, int x, int y, int color) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[type][rot][i][j] == 0) continue;
            int py = y + i + 1;// 盘面行号要加 1，上面空着一条边框。
            if (py < 1 || py > BOARD_HEIGHT) continue;// 刚生成时可能有一截在屏幕外。
            drawCell((x + j) * BLOCK_SIZE + 1, py, color, "■");
        }
    }
}

void drawGhostPiece(Game *game) {
    int ghostY = getGhostY(game);
    if (ghostY == game->currentY) return;// 已经贴底，幽灵和本体重合，不用画。
    drawPieceCells(game->currentType, game->currentRot, game->currentX, ghostY, 8);
}

void drawCurrentPiece(Game *game) {
    drawPieceCells(game->currentType, game->currentRot,
                   game->currentX, game->currentY, COLORS[game->currentType]);
}

void eraseOldPiece(Game *game) {
    // 擦除一定要用旧坐标，新坐标刚写进去就擦会把方块本身抹掉。
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[game->oldType][game->oldRot][i][j] == 0) continue;
            int py = game->oldY + i + 1;
            if (py < 1 || py > BOARD_HEIGHT) continue;
            drawCell((game->oldX + j) * BLOCK_SIZE + 1, py, 8, "·");
        }
    }
    // 幽灵下面是空地，直接补背景点就完事。
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[game->oldType][game->oldRot][i][j] == 0) continue;
            int py = game->oldGhostY + i + 1;
            if (py < 1 || py > BOARD_HEIGHT) continue;
            drawCell((game->oldX + j) * BLOCK_SIZE + 1, py, 8, "·");
        }
    }
}

void syncPieceRecord(Game *game) {
    game->oldX = game->currentX;
    game->oldY = game->currentY;
    game->oldRot = game->currentRot;
    game->oldType = game->currentType;
    game->oldGhostY = getGhostY(game);
}

void refreshPiece(Game *game) {
    eraseOldPiece(game);
    drawGhostPiece(game);
    drawCurrentPiece(game);// 本体后画，和幽灵重叠时压在上面。
    syncPieceRecord(game);
}

int getGhostY(Game *game) {
    int y = game->currentY;
    while (!checkCollision(game, game->currentX, y + 1, game->currentType, game->currentRot))
        y++;
    return y;
}

// ---------- 绘制边框（只画一次）----------
void drawBorder(void) {
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

void drawPreview(int type, int px, int py) {
    // 预览区先拿空格擦一遍，比补背景点看着干净。
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            drawCell(px + j * BLOCK_SIZE, py + i, 8, "  ");

    if (type < 0) return;// 暂存区空着，擦完就行。
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (SHAPES[type][0][i][j])
                drawCell(px + j * BLOCK_SIZE, py + i, COLORS[type], "■");
}

// ---------- 右侧信息面板 ----------
// 面板每帧都调用，但真正重绘的只有变化的字段，静态文字带个标志位只画一次。
void drawInfo(Game *game) {
    int infoX = BOARD_WIDTH * BLOCK_SIZE + 5;
    int baseY = 2;

    if (!game->titleDrawn) {
        setColor(14);
        gotoxy(infoX, baseY); printf("俄罗斯方块");
        game->titleDrawn = true;
    }

    if (game->score != game->lastScore) {
        setColor(7);
        gotoxy(infoX, baseY + 2); printf("得分: %d    ", game->score);
        game->lastScore = game->score;
    }
    if (game->level != game->lastLevel) {
        setColor(7);
        gotoxy(infoX, baseY + 3); printf("等级: %d    ", game->level);
        game->lastLevel = game->level;
    }
    if (game->speed != game->lastSpeed) {
        setColor(7);
        gotoxy(infoX, baseY + 4); printf("速度: %d ms ", game->speed);
        game->lastSpeed = game->speed;
    }
    if (game->lines != game->lastLines) {
        setColor(7);
        gotoxy(infoX, baseY + 5); printf("行数: %d    ", game->lines);
        game->lastLines = game->lines;
    }

    if (!game->nextLabelDrawn) {
        setColor(7);
        gotoxy(infoX, baseY + 7); printf("下一个:");
        game->nextLabelDrawn = true;
    }
    if (game->nextType != game->lastNextType) {
        drawPreview(game->nextType, infoX, baseY + 8);
        game->lastNextType = game->nextType;
    }

    if (!game->holdLabelDrawn) {
        setColor(7);
        gotoxy(infoX, baseY + 13); printf("暂存:");
        game->holdLabelDrawn = true;
    }
    if (game->holdType != game->lastHoldType) {
        drawPreview(game->holdType, infoX, baseY + 14);
        game->lastHoldType = game->holdType;
    }

    if (game->highScore != game->lastDrawScore) {
        setColor(14);
        gotoxy(infoX, baseY + 19); printf("最高分: %d    ", game->highScore);
        game->lastDrawScore = game->highScore;
    }

    if (game->pause != game->wasPaused) {
        setColor(12);
        gotoxy(infoX, baseY + 21);
        if (game->pause) printf("  暂停中  ");
        else             printf("          ");
        game->wasPaused = game->pause;
    }

    if (!game->ctrlDrawn) {
        int ctrlY = baseY + 23;
        setColor(11);
        gotoxy(infoX, ctrlY);     printf("按键说明:");
        gotoxy(infoX, ctrlY + 1); printf("← → 移动   ↑ 旋转");
        gotoxy(infoX, ctrlY + 2); printf("↓ 软降     空格 硬降");
        gotoxy(infoX, ctrlY + 3); printf("C 暂存     P 暂停");
        gotoxy(infoX, ctrlY + 4); printf("X 退出");
        game->ctrlDrawn = true;
    }
}

// ---------- 7-bag 随机 ----------
void fillBag(Game *game) {
    for (int i = 0; i < 7; i++) game->bag[i] = i;
    for (int i = 6; i > 0; i--) {// Fisher-Yates，从后往前换。
        int j = rand() % (i + 1);
        int tmp = game->bag[i];
        game->bag[i] = game->bag[j];
        game->bag[j] = tmp;
    }
    game->bagIndex = 0;
}

int takeFromBag(Game *game) {
    if (game->bagIndex >= 7) fillBag(game);// 取完一轮再洗。
    return game->bag[game->bagIndex++];
}

// ---------- 初始化 ----------
void initGame(Game *game) {
    for (int i = 0; i < BOARD_HEIGHT; i++)
        for (int j = 0; j < BOARD_WIDTH; j++)
            game->board[i][j] = 0;

    game->bagIndex = 7;// 置成越界值，逼第一次取牌时洗一轮。
    game->currentType = takeFromBag(game);
    game->nextType = takeFromBag(game);
    game->holdType = -1;
    game->currentRot = 0;
    game->currentX = BOARD_WIDTH / 2 - 2;
    game->currentY = 0;
    game->score = 0;
    game->level = 1;
    game->lines = 0;
    game->speed = INIT_SPEED;
    game->gameOver = false;
    game->pause = false;
    game->canHold = true;
    game->quit = false;
    game->dropTimer = 0;
    game->lockTimer = 0;

    // 这些不重置的话，重开一局面板就只剩数字，标题和说明不会再画。
    game->lastScore = -1;
    game->lastLevel = -1;
    game->lastSpeed = -1;
    game->lastNextType = -99;
    game->lastHoldType = -99;
    game->lastLines = -1;
    game->lastDrawScore = -1;
    game->titleDrawn = false;
    game->nextLabelDrawn = false;
    game->holdLabelDrawn = false;
    game->ctrlDrawn = false;
    game->wasPaused = false;
    g_highScoreDirty = false;

    game->oldX = game->currentX;
    game->oldY = game->currentY;
    game->oldRot = game->currentRot;
    game->oldType = game->currentType;
    game->oldGhostY = game->currentY;
}

// ---------- 碰撞检测 ----------
bool checkCollision(Game *game, int x, int y, int type, int rot) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[type][rot][i][j] == 0) continue;
            int bx = x + j;
            int by = y + i;
            if (bx < 0 || bx >= BOARD_WIDTH || by >= BOARD_HEIGHT) return true;
            if (by >= 0 && game->board[by][bx] != 0) return true;
        }
    }
    return false;
}

bool tryMove(Game *game, int dx, int dy) {
    if (checkCollision(game, game->currentX + dx, game->currentY + dy,
                       game->currentType, game->currentRot))
        return false;
    game->currentX += dx;
    game->currentY += dy;
    refreshPiece(game);
    return true;
}

bool tryRotate(Game *game) {
    int newRot = (game->currentRot + 1) % 4;
    int kicks[5] = {0, -1, 1, -2, 2};// 贴着墙转不动就往两边让一让，最多两格。
    for (int k = 0; k < 5; k++) {
        int nx = game->currentX + kicks[k];
        if (checkCollision(game, nx, game->currentY, game->currentType, newRot)) continue;
        game->currentX = nx;
        game->currentRot = newRot;
        refreshPiece(game);
        return true;
    }
    return false;
}

// ---------- 固定方块 ----------
void lockPiece(Game *game) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (SHAPES[game->currentType][game->currentRot][i][j] == 0) continue;
            int y = game->currentY + i;
            int x = game->currentX + j;
            if (y < 0 || y >= BOARD_HEIGHT || x < 0 || x >= BOARD_WIDTH) continue;
            game->board[y][x] = COLORS[game->currentType];
            drawCell(x * BLOCK_SIZE + 1, y + 1, COLORS[game->currentType], "■");
        }
    }
}

// ---------- 消除满行 ----------
void clearLines(Game *game) {
    int lines = 0;
    for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
        bool full = true;
        for (int col = 0; col < BOARD_WIDTH; col++)
            if (game->board[row][col] == 0) { full = false; break; }
        if (!full) continue;

        lines++;
        for (int r = row; r > 0; r--)// 上面的行整体落下来。
            for (int c = 0; c < BOARD_WIDTH; c++)
                game->board[r][c] = game->board[r - 1][c];
        for (int c = 0; c < BOARD_WIDTH; c++)
            game->board[0][c] = 0;
        row++;// 抵消上面的 row--，同一行还得再看一次。
    }

    if (lines == 0) return;

    game->lines += lines;
    int base = (lines == 1) ? 100 : (lines == 2) ? 300 : (lines == 3) ? 500 : 800;
    game->score += base * game->level;

    int newLevel = game->lines / 10 + 1;
    if (newLevel > game->level) {
        game->level = newLevel;
        int s = INIT_SPEED;
        for (int i = 1; i < newLevel; i++) s = s * 85 / 100;
        game->speed = (s < 50) ? 50 : s;// 再快就玩不了了，卡在 50 毫秒。
    }

    drawFixedBoard(game);// 板子动得多，整块重画一次省事。
}

// ---------- 生成新方块 ----------
void spawnNewPiece(Game *game) {
    game->currentType = game->nextType;
    game->nextType = takeFromBag(game);
    game->currentRot = 0;
    game->currentX = BOARD_WIDTH / 2 - 2;
    game->currentY = 0;
    game->canHold = true;
    game->dropTimer = 0;
    game->lockTimer = 0;

    syncPieceRecord(game);// 必须先同步，不然下一次擦除会把刚锁定的那些格子抹掉。
    if (checkCollision(game, game->currentX, game->currentY,
                       game->currentType, game->currentRot)) {
        game->gameOver = true;// 出生点被堵死。
        return;
    }
    drawGhostPiece(game);
    drawCurrentPiece(game);
}

// ---------- 暂存 ----------
void holdPiece(Game *game) {
    if (!game->canHold) return;// 一块只能换一次。

    eraseOldPiece(game);
    int current = game->currentType;

    if (game->holdType < 0) {// 暂存区还空着，直接放进去，从下一块接着玩。
        game->holdType = current;
        game->currentType = game->nextType;
        game->nextType = takeFromBag(game);
    } else {// 有存货就交换。
        game->currentType = game->holdType;
        game->holdType = current;
    }

    game->currentRot = 0;
    game->currentX = BOARD_WIDTH / 2 - 2;
    game->currentY = 0;
    game->canHold = false;
    game->dropTimer = 0;
    game->lockTimer = 0;

    syncPieceRecord(game);
    if (checkCollision(game, game->currentX, game->currentY,
                       game->currentType, game->currentRot)) {
        game->gameOver = true;
        return;
    }
    drawGhostPiece(game);
    drawCurrentPiece(game);
}

// ---------- 硬降 ----------
void hardDrop(Game *game) {
    eraseOldPiece(game);

    int startY = game->currentY;
    while (!checkCollision(game, game->currentX, game->currentY + 1,
                           game->currentType, game->currentRot))
        game->currentY++;
    game->score += (game->currentY - startY) * 2;// 落得越远分越多，鼓励快速操作。

    lockPiece(game);
    clearLines(game);
    spawnNewPiece(game);
}

// ---------- 键盘输入 ----------
void processInput(Game *game) {
    int key = 0;
    while (readKey(&key)) {// 缓冲区里有多少按键就处理多少，连按不丢。
        if (key >= 0x100) {// 方向键。
            if (game->pause) continue;
            switch (key - 0x100) {
                case VK_LEFT:  if (tryMove(game, -1, 0)) game->lockTimer = 0; break;
                case VK_RIGHT: if (tryMove(game, 1, 0)) game->lockTimer = 0; break;
                case VK_UP:    if (tryRotate(game)) game->lockTimer = 0; break;
                case VK_DOWN:// 软降，落一格一分。
                    if (tryMove(game, 0, 1)) {
                        game->score += 1;
                        game->dropTimer = 0;
                        game->lockTimer = 0;
                    }
                    break;
            }
            continue;
        }

        switch (key) {
            case 'a': case 'A': if (!game->pause && tryMove(game, -1, 0)) game->lockTimer = 0; break;
            case 'd': case 'D': if (!game->pause && tryMove(game, 1, 0)) game->lockTimer = 0; break;
            case 'w': case 'W': if (!game->pause && tryRotate(game)) game->lockTimer = 0; break;
            case 's': case 'S':
                if (!game->pause && tryMove(game, 0, 1)) {
                    game->score += 1;
                    game->dropTimer = 0;
                    game->lockTimer = 0;
                }
                break;
            case ' ':
                if (!game->pause) hardDrop(game);
                break;
            case 'c': case 'C':
                if (!game->pause) holdPiece(game);
                break;
            case 'p': case 'P':
                game->pause = !game->pause;
                break;
            case 'x': case 'X':// 直接退出，不弹结算。
                game->quit = true;
                game->gameOver = true;
                break;
        }
    }
}

// ---------- 主循环 ----------
void gameLoop(Game *game) {
    ULONGLONG last = GetTickCount64();// 按真实时间算下落，数 Sleep 次数会越玩越偏。

    while (!game->gameOver) {
        ULONGLONG now = GetTickCount64();
        int dt = (int)(now - last);
        last = now;
        if (dt > 100) dt = 100;// 切出去再切回来 dt 会很大，截断一下免得方块跳一大截。

        processInput(game);

        if (!game->pause) {
            bool landed = checkCollision(game, game->currentX, game->currentY + 1,
                                         game->currentType, game->currentRot);

            if (landed) {// 落地之后先等一会儿再锁，留点微调时间。
                game->lockTimer += dt;
                if (game->lockTimer >= LOCK_DELAY) {
                    game->lockTimer = 0;
                    lockPiece(game);
                    clearLines(game);
                    spawnNewPiece(game);
                }
            } else {
                game->lockTimer = 0;
                game->dropTimer += dt;
                while (game->dropTimer >= game->speed) {// 用 while，掉帧时能补上落下的格数。
                    game->dropTimer -= game->speed;
                    if (checkCollision(game, game->currentX, game->currentY + 1,
                                       game->currentType, game->currentRot))
                        break;
                    eraseOldPiece(game);
                    game->currentY++;
                    drawGhostPiece(game);
                    drawCurrentPiece(game);
                    syncPieceRecord(game);
                }
            }
        }

        drawInfo(game);
        Sleep(10);
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
    printf("                         到达等级: %d\n", game->level);
    printf("                         消除行数: %d\n", game->lines);
    printf("                         历史最高: %d\n\n", game->highScore);
    if (g_highScoreDirty) {
        setColor(10);
        printf("                         新纪录！\n\n");
    }
    setColor(7);
    printf("                按 R 重新开始，其他键退出...");
}

// ---------- 逻辑自检 ----------
// 用固定种子跑一遍关键逻辑，改动之后先跑这个，省得手动试半天。
int runSelfTest(void) {
    g_silent = true;// 自检不画界面，不然控制台会被刷满。
    int fails = 0;
    Game g;

    srand(12345);

    // 7-bag：连抽 70 个，每种应该正好 10 次。
    initGame(&g);
    g.bagIndex = 7;// initGame 已经拿走两个，重开一轮再统计。
    int count[7] = {0};
    for (int i = 0; i < 70; i++) count[takeFromBag(&g)]++;
    for (int i = 0; i < 7; i++) {
        if (count[i] != 10) {
            printf("7-bag 不对，第 %d 种出了 %d 次！\n", i, count[i]);
            fails++;
        }
    }

    // 边界碰撞。
    initGame(&g);
    if (!checkCollision(&g, -1, 5, 0, 0)) { printf("左边界没拦住！\n"); fails++; }
    if (!checkCollision(&g, BOARD_WIDTH, 5, 0, 0)) { printf("右边界没拦住！\n"); fails++; }
    if (!checkCollision(&g, 0, BOARD_HEIGHT, 0, 0)) { printf("底部没拦住！\n"); fails++; }

    // 消行：填满最底下一行，应该清掉、加分、行数加一。
    initGame(&g);
    for (int c = 0; c < BOARD_WIDTH; c++) g.board[BOARD_HEIGHT - 1][c] = COLORS[0];
    int scoreBefore = g.score;
    clearLines(&g);
    if (g.board[BOARD_HEIGHT - 1][0] != 0) { printf("满行没清掉！\n"); fails++; }
    if (g.score <= scoreBefore) { printf("消行没加分！\n"); fails++; }
    if (g.lines != 1) { printf("消行数算错了！\n"); fails++; }

    // 幽灵块：方块的落点提示应该一路落到底，再往下就走不动了。
    initGame(&g);
    g.currentType = 0;
    g.currentRot = 0;
    g.currentX = 3;
    g.currentY = 0;
    int ghostY = getGhostY(&g);
    if (ghostY <= 0) { printf("幽灵块没往下落！\n"); fails++; }
    if (!checkCollision(&g, g.currentX, ghostY + 1, g.currentType, g.currentRot)) {
        printf("幽灵块没停到底部！\n");
        fails++;
    }

    // 墙踢：I 形竖着贴右墙，转横排会超出去，应该自动左让一格。
    initGame(&g);
    g.currentType = 0;
    g.currentRot = 1;
    g.currentX = BOARD_WIDTH - 3;
    g.currentY = 5;
    syncPieceRecord(&g);
    if (!tryRotate(&g)) { printf("贴墙转不动，墙踢没生效！\n"); fails++; }
    if (checkCollision(&g, g.currentX, g.currentY, g.currentType, g.currentRot)) {
        printf("墙踢之后位置不对！\n");
        fails++;
    }

    // 暂存：换过去之后手里应该拿到下一块，而且这一块不能再换。
    initGame(&g);
    int firstType = g.currentType;
    holdPiece(&g);
    if (g.holdType != firstType) { printf("暂存没把方块收进去！\n"); fails++; }
    if (g.canHold) { printf("暂存次数没限制住！\n"); fails++; }

    g_silent = false;
    if (fails == 0) printf("✅ 自检全部通过！\n");
    else printf("自检有 %d 项没通过！\n", fails);
    return fails;
}
