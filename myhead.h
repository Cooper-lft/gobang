#ifndef MYHEAD_H
#define MYHEAD_H

//系统头文件
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define SIZE 15
#define CHARSIZE 3
#define PVP 1  //人人对战模式
#define PVE 0  //人机对战模式
#define PLAYERFIRST 1 //PVE中谁先落子的宏
#define AIFIRST 2

//定义方向枚举
typedef enum{
    HOR=0, //水平方向
    VER,   //竖直方向
    DIAG_M,//主对角线
    DIAG_S //副对角线
}Direction;//方向：水平，竖直，主对角线(mian)，副对角线(sub)

typedef struct {
    int x;
    int y;
}Point;//定义Point类型变量,便于返回一个点

#define LIVE 0//活
#define SEMI_LIVE 1//半活
#define DEAD 2//死

//先定义一条线上的结构体记录信息。成员：count:连子数;blocked:被封堵数（0=活，1=半活，2=死）
typedef struct{
    int count; //连子数
    int blocked; //被封堵数（0=活，1=半活，2=死）
}LineInfo;

//棋子状态：黑子和白子
#define BLACK 1
#define WHITE 2
#define EMPTY 0

//数组声明
extern int arrayForInnerBoardLayout[SIZE][SIZE];

extern Point currentPoint;


//函数声明
void initRecordBorard(void);

void innerLayoutToDisplayArray(Point currentPoint);
void displayBoard();

int selectmode();

int checkWin(int x,int y,int state);
int isHorFive(int x,int y, int state);
int isVerFive(int x,int y,int state);
int isDiagMasFive(int x,int y,int state);
int isDiagSubFive(int x,int y,int state);
LineInfo countLine(int x,int y,int state,Direction direction);
Point getPlayerMove(int Color);
#endif