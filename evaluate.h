#ifndef EVALUATE_H
#define EVALUATE_H

#include "myhead.h"
#include "forbidden.h"



//不同落子下的得分的宏
#define WIN 1000000
#define LIVE4 100000
#define BROKEN4 1000 //冲四
#define DEAD4 0
#define LIVE3 1000
#define BROKEN3 100 //冲三
#define DEAD3 0 //死三
#define LIVE2 100
#define BROKEN2 10 //冲二
#define DEAD2 0//死二

typedef struct{//存坐标和分数，用于辅助判断需要考虑搜索哪些点
    Point p;
    int score;//记录下在该点的得分，用于排序
}Move;

#define DISTANCE 2 //检查该点处是否有邻居的正方形“半径”范围
extern Move moves[SIZE*SIZE];//存储AI准备落的候选点的数组

#define INF 1000000 //无穷大，用于初始化

typedef struct {
    unsigned long long key; // Zobrist Hash 校验码
    int depth;              // 搜索深度 (这个结果是搜了几层得到的)
    int score;              // 分数
    int flag;               // 值的类型 (准确值/上界/下界)
} TTEntry;//置换表的一项

// 标记类型
#define HASH_EXACT 0 // 精确值
#define HASH_ALPHA 1 // 上界 (<= value)
#define HASH_BETA  2 // 下界 (>= value)

// 假设哈希表大小为 2^20 (约100万条)，需占用约 16MB 内存
// 必须是 2 的幂次方，以便用 & 运算替代 %
#define TT_SIZE 1048576

//Zobrist 随机数数组 [行][列][颜色(0空,1黑,2白)]
extern unsigned long long zArray[SIZE][SIZE][3];

//当前棋盘的实时 Hash 值
extern unsigned long long currentHash;

//置换表数组
extern TTEntry tTable[TT_SIZE];

//超时标记 (用于 minimax 中断)
extern int isTimeOut;


//初始化 Zobrist 哈希表 (需要在 main 开头调用)
void initZobrist(void);

int getPointScore(int x,int y,int myColor);
Point getBestMove(int myColor);
int isForbiddenPoint(int x, int y);
int lineScore(int x,int y, LineInfo lineinfo);
int evaluateOnecolor(int x,int y,int Color);
int isBoardEmpty();
int hasNeighbor(int x,int y,int distance);
int generateMoves(Move moves[],int myColor);
int compareMoves(const void*a,const void*b);
int evaluateBoard(int myColor);
void makeMove(Point p,int Color);
void unmakeMove(Point p);
int minimax(int depth, int alpha, int beta, int isMax, int myColor);
#endif