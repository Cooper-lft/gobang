#include "include/myhead.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//棋盘使用的是UTF-8编码，每一个中文字符占用3个字节。

//空棋盘模板 
char arrayForEmptyBoard[SIZE][SIZE*CHARSIZE+1] = 
{
		"┏┯┯┯┯┯┯┯┯┯┯┯┯┯┓",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┠┼┼┼┼┼┼┼┼┼┼┼┼┼┨",
		"┗┷┷┷┷┷┷┷┷┷┷┷┷┷┛"
};
//此数组存储用于显示的棋盘 
char arrayForDisplayBoard[SIZE][SIZE*CHARSIZE+1];
 
char play1Pic[]="●";//黑棋子;
char play1CurrentPic[]="▲"; 

char play2Pic[]="◎";//白棋子;
char play2CurrentPic[]="△";

//此数组用于记录当前的棋盘的格局 
//innerboard的数组元素为1表示黑色棋子，2表示白色棋子，0表示没有棋子
int arrayForInnerBoardLayout[SIZE][SIZE];

int main()
{
    int workstate;
    workstate=selectmode();//选择模式
    

    arrayForInnerBoardLayout[0][0]=1;    //在棋盘的左上角落一个黑色棋子
    innerLayoutToDisplayArray();  //将心中的棋盘转成用于显示的棋盘
    displayBoard();          //显示棋盘
    getchar();   
/*
    arrayForInnerBoardLayout[5][9]=2; //innerboard的数组元素为1表示黑色棋子，2表示白色棋子，0表示没有棋子
    innerLayoutToDisplayArray();
    displayBoard();
    getchar();

    arrayForInnerBoardLayout[3][4]=2;
    innerLayoutToDisplayArray();
    displayBoard();
    getchar();

    arrayForInnerBoardLayout[6][1]=1;
    innerLayoutToDisplayArray();
    displayBoard();
    getchar();

    arrayForInnerBoardLayout[9][4]=2;
    innerLayoutToDisplayArray();
    displayBoard();
    getchar();
*/
 
    return 0;
}

//初始化一个空棋盘格局 
void initRecordBorard(void){
	//通过双重循环，将arrayForInnerBoardLayout清0
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            arrayForDisplayBoard[i][j]=0;
        }       
    }
}

//将arrayForInnerBoardLayout中记录的棋子位置，转化到arrayForDisplayBoard中
void innerLayoutToDisplayArray(void){
	//第一步：将arrayForEmptyBoard中记录的空棋盘，复制到arrayForDisplayBoard中
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE*CHARSIZE+1;j++){
            arrayForDisplayBoard[i][j]=arrayForEmptyBoard[i][j];
        }
        arrayForDisplayBoard[i][SIZE*CHARSIZE+1]='\0'; //给每行棋盘数组末尾加\0表示结束。
    }


	//第二步：扫描arrayForInnerBoardLayout，当遇到非0的元素，将●或者◎复制到arrayForDisplayBoard的相应位置上
	//注意：arrayForDisplayBoard所记录的字符是中文字符，每个字符占3个字节(?)。●和◎也是中文字符，每个也占3个字节。
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            if(arrayForInnerBoardLayout[i][j]!=0){
                if(arrayForInnerBoardLayout[i][j]==1){
                    arrayForDisplayBoard[i][CHARSIZE*j]=play1Pic[0];
                    arrayForDisplayBoard[i][CHARSIZE*j+1]=play1Pic[1];
                    if(CHARSIZE==3){
                        arrayForDisplayBoard[i][CHARSIZE*j+2]=play1Pic[2];
                    }
                }else if(arrayForInnerBoardLayout[i][j]==2){
                    arrayForDisplayBoard[i][CHARSIZE*j]=play2Pic[0];
                    arrayForDisplayBoard[i][CHARSIZE*j+1]=play2Pic[1];
                    if(CHARSIZE==3){
                        arrayForDisplayBoard[i][CHARSIZE*j+2]=play2Pic[2];
                    }
                } 
            }
        }
    }   


 
}


//显示棋盘格局 
void displayBoard(void){
	int i;
	//第一步：清屏
	system("clear");   //清屏  
	//第二步：将arrayForDisplayBoard输出到屏幕上
    for(int i=SIZE,k=0;i>0;i--,k++){
        printf("%2d ",i);
        for(int j=0;j<SIZE;j++){
            printf("%c%c%c ",arrayForDisplayBoard[k][CHARSIZE*j],arrayForDisplayBoard[k][CHARSIZE*j+1],arrayForDisplayBoard[k][CHARSIZE*j+2]);
        }
        printf("\n");
    }
	//第三步：输出最下面的一行字母A B .... 
    printf("   ");
    for(int i=0;i<SIZE;i++){
        printf("%c ",'A'+i);

    }
    printf("\n");
    //显示作者名字
    printf("作者：李丰廷\n");
} 

//选择下棋模式
int selectmode(){
    int workmode;
    while(1){
        initRecordBorard();    //初始化一个空棋盘
        innerLayoutToDisplayArray();
        displayBoard();//显示棋盘
        printf("请选择模式：人人对战(请输入1)/人机对战(请输入0)\n");
        //读取并验证输入是否正确
        if(scanf("%d",&workmode)==1 && (workmode==0||workmode==1) ){
            break;
        }else{
            getchar();//吸收错误输入的空格
            int c;
            while((c=getchar())!='\n'&&c!=EOF){
                ;
            }
            printf("\n输入错误,请按任意建后重新输入\n");
            getchar();
        }
    }
    return workmode;
}

int checkWin(int x,int y,int state){//x,y分别是列和行的索引，从上到下，从左到右均从0开始
    if(isHorFive(x,y,state)==1 || isVerFive(x,y,state)==1 || isDiagMasFive(x,y,state)==1|| isDiagSubFive(x,y,state)==1){
        return 1;
    }else{
        return 0;
    }
}

int isHorFive(int x,int y, int state){//横向是否有五个长连,state即是黑子BLACK还是白子WHITE
    int count=1;
    for(int i=x+1;i>=0&&i<SIZE&&arrayForInnerBoardLayout[y][i]==state;i++){
        count++;
    }//向右数同种落子个数
    for(int i=x-1;i>=0&&i<SIZE&&arrayForInnerBoardLayout[y][i]==state;i--){
        count++;
    }//向左数
    if(count>=5){
        return 1;//连成至少5珠
    }else{
        return 0;
    }
}

int isVerFive(int x,int y,int state){//竖向是否有5连子
    int count=1;
    for(int j=y+1;j>=0&&j<SIZE&&arrayForInnerBoardLayout[j][x]==state;j++){
        count++;
    }//向下数
    for(int j=y-1;j>=0&&j<SIZE&&arrayForInnerBoardLayout[j][x]==state;j--){
        count++;
    }//向上数
    if(count>=5){
        return 1;
    }else{
        return 0;
    }
}

int isDiagMasFive(int x,int y,int state){//主对角线是否连成5颗
    int count=1;
    for(int i=x+1,j=y+1;i<SIZE&&j<SIZE&&arrayForInnerBoardLayout[j][i]==state;i++,j++){
        count++;
    }//向右下数
    for(int i=x-1,j=y-1;i>=0&&j>=0&&arrayForInnerBoardLayout[j][i]==state;i--,j--){
        count++;
    }//向左上数
    if(count>=5){
        return 1;
    }else{
        return 0;
    }
}

int isDiagSubFive(int x,int y,int state){
    int count=1;
    for(int i=x-1,j=y+1;i>=0&&j<SIZE&&arrayForInnerBoardLayout[j][i]==state;i--,j++){
        count++;
    }//向左下数
    for(int i=x+1,j=y-1;i<SIZE&&j>=0&&arrayForInnerBoardLayout[j][i]==state;i++,j--){
        count++;
    }//向右上数
    if(count>=5){
        return 1;
    }else{
        return 0;
    }
}