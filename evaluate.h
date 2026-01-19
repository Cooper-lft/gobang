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


int getPointScore(int x,int y,int myColor);
Point getBestMove(int myColor);
int isForbiddenPoint(int x, int y);
int lineScore(int x,int y, LineInfo lineinfo);
int evaluateOnecolor(int x,int y,int Color);
int isBoardEmpty();
#endif