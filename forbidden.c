#include "myhead.h"
#include "forbidden.h"

// 宏：判断坐标是否在棋盘内
#define IS_VALID(x, y) ((x) >= 0 && (x) < SIZE && (y) >= 0 && (y) < SIZE)

// 获取棋子状态，越界视为墙壁（即当作白棋/对手处理）
static int getStone(int x, int y) {
    if (!IS_VALID(x, y)) return WHITE; 
    return arrayForInnerBoardLayout[y][x];
}

// 将以 (x,y) 为中心，方向 (dx,dy) 上的前后4格提取到 buffer 中
// buffer[4] 永远是中心点 (x,y)
static void getLine(int x, int y, int dx, int dy, int buffer[9]) {
    for (int i = -4; i <= 4; i++) {
        buffer[i + 4] = getStone(x + i * dx, y + i * dy);
    }
}


// 基础形状判断(在一维数组上判断)


// 判断是否成五 (11111)
static int hasFive(int line[9]) {
    for (int i = 0; i <= 4; i++) { // 窗口滑动
        int count = 0;
        for (int k = 0; k < 5; k++) {
            if (line[i+k] == BLACK) count++;
        }
        if (count == 5) return 1;
    }
    return 0;
}

// 判断是否长连 (111111...)
static int hasOverline(int line[9]) {
    for (int i = 0; i <= 3; i++) {
        int count = 0;
        for (int k = 0; k < 6; k++) { // 检查6个
            if (line[i+k] == BLACK) count++;
        }
        if (count == 6) return 1;
    }
    return 0;
}

// 判断是否是“标准活四” (011110)
// 这是判断活三的基础：如果加一颗子能变成这样，那就是活三
static int checkLiveFourShape(int line[9]) {
    for (int i = 0; i <= 3; i++) {
        // 模式： EMPTY + BLACK*4 + EMPTY
        if (line[i] == EMPTY &&
            line[i+1] == BLACK &&
            line[i+2] == BLACK &&
            line[i+3] == BLACK &&
            line[i+4] == BLACK &&
            line[i+5] == EMPTY) {
            return 1;
        }
    }
    return 0;
}

// 虚拟落子

// 判断是否是“四” (包括冲四和活四)
// 定义：在这一行再加一颗黑子，能连成五 (且不是长连)
static int checkFour(int line[9]) {
    // 遍历 buffer 中的空位，尝试填入黑子
    for (int i = 0; i < 9; i++) {
        if (line[i] == EMPTY) {
            line[i] = BLACK; // 虚拟落子
            
            // 如果落子后形成5连，且没有形成长连，那就是“四”
            if (hasFive(line) && !hasOverline(line)) {
                line[i] = EMPTY; // 还原
                return 1;
            }
            line[i] = EMPTY; // 还原
        }
    }
    return 0;
}

// 判断是否是“活三”
// 定义：在这一行再加一颗黑子，能形成“标准活四”(011110)
// 注意：禁手规则里，必须形成活四才算活三。如果形成冲四不算活三。
static int checkLiveThree(int line[9]) {
    for (int i = 0; i < 9; i++) {
        if (line[i] == EMPTY) {
            line[i] = BLACK; // 虚拟落子
            
            // 核心逻辑：加一子能变活四，原状即为活三
            if (checkLiveFourShape(line)) {
                line[i] = EMPTY; // 还原
                return 1;
            }
            line[i] = EMPTY; // 还原
        }
    }
    return 0;
}

// 主入口：判断禁手
int isForbiddenPoint(int x, int y) {
    // 【关键修复】：保存原始状态
    // 不管本来是 EMPTY 还是 BLACK (刚下的)，我们都先记下来
    int originalState = arrayForInnerBoardLayout[y][x];

    // 如果该点是白子（对手），那肯定不是禁手（或者逻辑错误），直接返回
    if (originalState == WHITE) return 0;

    // 1. 强制设为黑子进行检查
    arrayForInnerBoardLayout[y][x] = BLACK;

    int dxs[] = {1, 0, 1, 1};
    int dys[] = {0, 1, 1, -1};

    int fiveCount = 0;     // 五连个数
    int overlineCount = 0; // 长连个数
    int fourCount = 0;     // 四的个数 (活四+冲四)
    int threeCount = 0;    // 活三的个数

    int lineBuffer[9]; // 用于缓存一条线 (前后各4格，共9格)

    // 遍历四个方向
    for (int i = 0; i < 4; i++) {
        // 提取当前方向的线到 buffer
        getLine(x, y, dxs[i], dys[i], lineBuffer);

        // A. 检查成五
        if (hasFive(lineBuffer)) {
            // 注意：要区分是正好5个还是长连
            if (hasOverline(lineBuffer)) {
                overlineCount++;
            } else {
                fiveCount++;
            }
        } 
        // B. 如果没成五，才去检查三和四 (因为成五优先级最高)
        else {
            if (checkFour(lineBuffer)) fourCount++;
            if (checkLiveThree(lineBuffer)) threeCount++;
        }
    }

    // 2. 还原棋盘状态
    arrayForInnerBoardLayout[y][x] = originalState;

    // 3. 判定禁手优先级

    // 优先级 1：成五 (Win) - 如果落子能成五，禁手失效，黑棋直接获胜
    if (fiveCount > 0) return 0;

    // 优先级 2：长连 (Forbidden)
    if (overlineCount > 0) return 1;

    // 优先级 3：四四禁手 (Double Four) - 两个或以上“四”
    if (fourCount >= 2) return 1;

    // 优先级 4：三三禁手 (Double Live Three) - 两个或以上“活三”
    if (threeCount >= 2) return 1;

    // 安全点
    return 0;
}