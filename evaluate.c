#include "myhead.h"
#include "evaluate.h"
#include "forbidden.h"

Point getBestMove(int myColor){//参数是己方的棋子颜色
    Point BestMove={-1,-1};//用一个点来存返回值（落子处）
    int PointScore=-1;//记录点的得分
    int maxScore=-100000;//初始化一个最大得分值，用于统计目前扫描遇到的最大得分，会随着扫描更新
    int sameScoreCount=1;//使用蓄水池抽样随机算法

    if(isBoardEmpty()){
        return (Point){SIZE/2,SIZE/2}; //如果AI开局先，直接下天元
    }

    for(int row=0;row<SIZE;row++){//遍历行
        for(int col=0;col<SIZE;col++){//遍历列
            if(arrayForInnerBoardLayout[row][col]==EMPTY){//对未落子处求分数
                if(myColor==BLACK&&isForbiddenPoint(col,row)==1){//如果是黑子，先判断禁手，不符合直接跳过
                    continue;
                }   
                if((PointScore=getPointScore(col,row,myColor))>maxScore){
                    BestMove.x=col;
                    BestMove.y=row;
                    maxScore=PointScore;
                    sameScoreCount=1;//重置计数器，这是第一个最高分点
                }else if(PointScore==maxScore){
                    sameScoreCount++;
                    if(rand()%sameScoreCount==0){//如果得分重复，随机化落子位置
                        BestMove.x=col;
                        BestMove.y=row;
                    }
                }
            }
        }
    }
    return BestMove;
}

int getPointScore(int x,int y,int myColor){//输入一个点的坐标x:横坐标列（col）；y：纵坐标行（row）。返回该点得分
    //确定对手颜色
    int enemyColor=(myColor==BLACK)? WHITE:BLACK;
    int attackScore=evaluateOnecolor(x,y,myColor);//计算自己下这个点的得分
    int defenseScore=evaluateOnecolor(x,y,enemyColor);//计算对手下这个点能得的分 

    if(attackScore>=WIN || defenseScore>=WIN) return WIN; //在这儿落子，要么自己赢，要么对手赢，所以必须下这里
    if(attackScore>=LIVE4) return LIVE4; //有活四先下活四

    return attackScore+defenseScore;
}

int lineScore(int x,int y, LineInfo lineinfo){//计算一条线上的得分
    int result;
    if(lineinfo.count==5){
        return WIN;
    }else if(lineinfo.count==4){
        if(lineinfo.blocked==LIVE){
            return LIVE4;
        }else if(lineinfo.blocked==SEMI_LIVE){
            return BROKEN4;
        }else{
            return DEAD4;
        }
    }else if(lineinfo.count==3){
        if(lineinfo.blocked==LIVE){
            return LIVE3;
        }else if(lineinfo.blocked==SEMI_LIVE){
            return BROKEN3;
        }else{
            return DEAD3;
        }
    }else if(lineinfo.count==2){
        if(lineinfo.blocked==LIVE){
            return LIVE2;
        }else if(lineinfo.blocked==SEMI_LIVE){
            return BROKEN2;
        }else{
            return DEAD2;
        }
    }else{
        return 0;//单子或死棋
    }
}

//用于评估在(x,y)处下一个Color颜色的子得到的四个方向的总分
int evaluateOnecolor(int x,int y,int Color){
    int totalScore=0;
    LineInfo lineinfo=countLine(x,y,Color,HOR);//先计算横向信息
    totalScore+=lineScore(x,y,lineinfo);//计算横向分数

    lineinfo=countLine(x,y,Color,VER);//计算竖向信息
    totalScore+=lineScore(x,y,lineinfo);

    lineinfo=countLine(x,y,Color,DIAG_M);
    totalScore+=lineScore(x,y,lineinfo);

    lineinfo=countLine(x,y,Color,DIAG_S);
    totalScore+=lineScore(x,y,lineinfo);

    return totalScore;
}

int isBoardEmpty(){
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            if(arrayForInnerBoardLayout[i][j]!=EMPTY){
                return 0;
            }
        }
    }
    return 1;
}