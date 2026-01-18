#ifndef MYHEAD_H
#define MYHEAD_H

//定义方向枚举
typedef enum{
    HORIZONTAL=0, //水平方向
    VERTICAL,     //竖直方向
    DIAG_L,
    DIAG_R
}Direction;//方向：水平，竖直，主对角线，副对角线

typedef struct
{
    int count; //连子个数
    int blocked; //封子数(0=活，1=半活，2=死)
}LineInfo; //记录一条线上的信息

struct PointScore{
    int x;
    int y;
    int attackScore;
    int defenseScore;
};

//不同落子下的得分
#define WIN5 10000000000
#define LIVE4 10000
#define DEAD4 1000
#define LIVE3 1000
#define DEAD3 100
#define LIVE2 100
#define DEAD2 10

int selectmode();

#endif