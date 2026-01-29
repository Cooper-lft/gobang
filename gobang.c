#include "myhead.h"
#include "evaluate.h"
#include "forbidden.h"
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
Point currentPoint={-1,-1};//记录当前下的子的位置


int main()
{
    srand((unsigned int)time(NULL));//获取随机数种子
    initZobrist();//初始化置换表
    initRecordBorard();//初始化棋盘
    int isWin=0;
    int workstate;
    workstate=selectmode();//选择模式
    int currentPlayerColor = BLACK;//先手一定是黑棋

    
    if(workstate==PVE){
        int whoFirst;
        while(1){
            printf("谁先落子？(玩家(输入1) or AI(输入2)): \n");
            //获取当前玩家的落子
            if(scanf("%d",&whoFirst)==1&&whoFirst<=2&&whoFirst>=1){
                break;
            }else{
                if(whoFirst!=EOF){
                printf("输入有误，请重新选择谁先落子");
                while(getchar()!='\n'){
                    ;
                }
                }else{
                    exit(0);
                }
            }
        }
        while (1)//PVE主程序
        {
            //显示棋盘
            innerLayoutToDisplayArray(currentPoint);
            displayBoard();
            
            switch (whoFirst){//看是人先还是AI先
            case PLAYERFIRST://玩家先，为黑子
                while(1){
                    currentPoint=getPlayerMove(currentPlayerColor);//获取玩家落子
                    arrayForInnerBoardLayout[currentPoint.y][currentPoint.x]=currentPlayerColor;
                    
                    innerLayoutToDisplayArray(currentPoint); //将当前落子更新到显示数组
                    displayBoard();//显示棋盘
                    isWin=checkWin(currentPoint.x,currentPoint.y,currentPlayerColor);
                    if(isWin==1&&isForbiddenPoint(currentPoint.x,currentPoint.y)!=1){//检查是否获胜且不是禁手
                        printf("黑方获胜\n");
                        return 0;
                    }else if(isForbiddenPoint(currentPoint.x,currentPoint.y)==1){
                        printf("黑方落子(%c,%d)为禁手，白方获胜\n",currentPoint.x+'A',SIZE-currentPoint.y);
                        return 0;
                    }else{//没赢，轮到AI，是白子
                        currentPlayerColor=WHITE;
                    }

                    currentPoint=getBestMove(currentPlayerColor);//获取AI落子
                    // 如果getBestMove在空盘或异常情况下返回(-1,-1)，落子到天元（中央）
                    if(currentPoint.x==-1 || currentPoint.y==-1){
                        currentPoint.x = SIZE/2;
                        currentPoint.y = SIZE/2;
                    }
                    arrayForInnerBoardLayout[currentPoint.y][currentPoint.x]=currentPlayerColor;
                    innerLayoutToDisplayArray(currentPoint);
                    displayBoard();
                    // 显示AI最后落子的棋盘坐标（列字母, 行数字，与棋盘显示一致）
                    printf("AI落子：(%d,%c)\n",  SIZE - currentPoint.y,currentPoint.x + 'A');
                    printf("AI用时：%.2fs\n", lastAIDuration);
                    fflush(stdout);
                    isWin=checkWin(currentPoint.x,currentPoint.y,currentPlayerColor);
                    if(isWin==1){
                        printf("白方获胜\n");
                        return 0;
                    }else{
                        currentPlayerColor=BLACK;
                    }
                }
                break;
            
            case AIFIRST:
                while(1){
                    currentPoint=getBestMove(currentPlayerColor);
                    // 保护性处理：若AI在开局返回无效点，落子天元
                    if(currentPoint.x==-1 || currentPoint.y==-1){
                        currentPoint.x = SIZE/2;
                        currentPoint.y = SIZE/2;
                    }
                    arrayForInnerBoardLayout[currentPoint.y][currentPoint.x]=currentPlayerColor;
                    innerLayoutToDisplayArray(currentPoint);
                    displayBoard();
                    // 显示AI最后落子的棋盘坐标（列字母, 行数字，与棋盘显示一致）
                    printf("AI落子：(%d,%c)\n",  SIZE - currentPoint.y,currentPoint.x + 'A');
                    printf("AI用时：%.2fs\n", lastAIDuration);
                    fflush(stdout);
                    isWin=checkWin(currentPoint.x,currentPoint.y,currentPlayerColor);
                    if(isWin==1){
                        printf("黑方获胜\n");
                        return 0;
                    }else{
                        currentPlayerColor=WHITE;
                    }

                    currentPoint=getPlayerMove(currentPlayerColor);
                    arrayForInnerBoardLayout[currentPoint.y][currentPoint.x]=currentPlayerColor;
                    innerLayoutToDisplayArray(currentPoint);
                    displayBoard();
                    isWin=checkWin(currentPoint.x,currentPoint.y,currentPlayerColor);
                    if(isWin==1){
                        printf("白方获胜\n");
                        return 0;
                    }else{
                        currentPlayerColor=BLACK;
                    }
                }
                break;
            }
        }
        
    }else if(workstate==PVP){//PVP主程序
        while (1)
        {
            //显示棋盘
            innerLayoutToDisplayArray(currentPoint);
            displayBoard();
            
            currentPoint=getPlayerMove(currentPlayerColor);//获取玩家落子
            arrayForInnerBoardLayout[currentPoint.y][currentPoint.x]=currentPlayerColor;
            innerLayoutToDisplayArray(currentPoint);
            displayBoard();
            
            isWin=checkWin(currentPoint.x,currentPoint.y,currentPlayerColor);
            if(isWin==1){
                printf("%s方获胜\n",((currentPlayerColor==BLACK)?"黑":"白"));
                return 0;
            }else{
                currentPlayerColor=(currentPlayerColor==BLACK)? WHITE:BLACK;
            }
        }
        
    }

/*
    arrayForInnerBoardLayout[0][0]=1;    //在棋盘的左上角落一个黑色棋子
    innerLayoutToDisplayArray();  //将心中的棋盘转成用于显示的棋盘
    displayBoard();          //显示棋盘
    getchar();   

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
            arrayForInnerBoardLayout[i][j]=0;
        }       
    }
}

//将arrayForInnerBoardLayout中记录的棋子位置，转化到arrayForDisplayBoard中
void innerLayoutToDisplayArray(Point currentPoint){
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
                    if(currentPoint.x==j&&currentPoint.y==i){
                        arrayForDisplayBoard[i][CHARSIZE*j]=play1CurrentPic[0];
                        arrayForDisplayBoard[i][CHARSIZE*j+1]=play1CurrentPic[1];
                        arrayForDisplayBoard[i][CHARSIZE*j+2]=play1CurrentPic[2];
                    }else{
                        arrayForDisplayBoard[i][CHARSIZE*j]=play1Pic[0];
                        arrayForDisplayBoard[i][CHARSIZE*j+1]=play1Pic[1];
                        if(CHARSIZE==3){
                            arrayForDisplayBoard[i][CHARSIZE*j+2]=play1Pic[2];
                        }
                    }
                }else if(arrayForInnerBoardLayout[i][j]==2){
                    if(currentPoint.x==j&&currentPoint.y==i){
                        arrayForDisplayBoard[i][CHARSIZE*j]=play2CurrentPic[0];
                        arrayForDisplayBoard[i][CHARSIZE*j+1]=play2CurrentPic[1];
                        arrayForDisplayBoard[i][CHARSIZE*j+2]=play2CurrentPic[2];
                    }else{    
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
}


//显示棋盘格局 
void displayBoard(){
	int i;
	//第一步：清屏
	system("clear");   //清屏  
	//第二步：将arrayForDisplayBoard输出到屏幕上
    for(int i=SIZE,k=0;i>0;i--,k++){//i用于打印行数，k是内部棋盘的行数，j是内部棋盘的列数
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
    printf("Work of 李丰廷\n");
} 

//选择下棋模式
int selectmode(){
    int workmode;
    while(1){
        initRecordBorard();    //初始化一个空棋盘
        innerLayoutToDisplayArray(currentPoint);
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

int checkWin(int x,int y,int state){//检查是否获胜：x,y分别是列和行的索引，从上到下，从左到右均从0开始
    if(isHorFive(x,y,state)==1 || isVerFive(x,y,state)==1 || isDiagMasFive(x,y,state)==1|| isDiagSubFive(x,y,state)==1){
        return 1;
    }else{
        return 0;
    }
}

int isHorFive(int x,int y, int state){//横向是否有五个长连,state即是黑子BLACK还是白子WHITE
    LineInfo lineinfo=countLine(x,y,state,HOR);
    if (lineinfo.count>=5)return 1;
    else return 0;
}

int isVerFive(int x,int y,int state){//竖向是否有5连子
    LineInfo lineinfo=countLine(x,y,state,VER);
    if (lineinfo.count>=5)return 1;
    else return 0;
}

int isDiagMasFive(int x,int y,int state){//主对角线是否连成5颗
    LineInfo lineinfo=countLine(x,y,state,DIAG_M);
    if (lineinfo.count>=5)return 1;
    else return 0;
}


int isDiagSubFive(int x,int y,int state){
    LineInfo lineinfo=countLine(x,y,state,DIAG_S);
    if (lineinfo.count>=5)return 1;
    else return 0;
}

LineInfo countLine(int x,int y,int state,Direction direction){ //解耦，通过direction来判断计数方向
    LineInfo lineinfo;
    lineinfo.count=1;
    lineinfo.blocked=0;
    int dx,dy;
    switch (direction)
    {
    case HOR:    dx=1;dy=0;break;
    case VER:    dx=0;dy=1;break;
    case DIAG_M: dx=1;dy=1;break;
    case DIAG_S: dx=-1;dy=1;break;
    }
    //先正向
    int i=x+dx,j=y+dy;
    for(;i>=0&&i<SIZE&&j>=0&&j<SIZE&&arrayForInnerBoardLayout[j][i]==state;i+=dx,j+=dy){
        lineinfo.count++;
    }//正向数同种落子个数
    if(i<0||i>=SIZE||j<0||j>=SIZE){//撞墙
        lineinfo.blocked++;
    }else if(arrayForInnerBoardLayout[j][i]!=EMPTY){//有不同颜色的棋子堵着
        lineinfo.blocked++;
    }

    //再反向
    i=x-dx,j=y-dy;
    for(;i>=0&&i<SIZE&&j>=0&&j<SIZE&&arrayForInnerBoardLayout[j][i]==state;i-=dx,j-=dy){
        lineinfo.count++;
    }//反向数
    if(i<0||i>=SIZE||j<0||j>=SIZE){//撞墙
        lineinfo.blocked++;
    }else if(arrayForInnerBoardLayout[j][i]!=EMPTY){//有不同颜色的棋子堵着
        lineinfo.blocked++;
    }

    return lineinfo;
}

Point getPlayerMove(int Color){//获取玩家落子信息，第一个参数是下棋模式，第二个参数是玩家的颜色
    Point p;

    printf("请%s落子(输入行 列):",((Color==BLACK)?"黑方":"白方"));//提示输入

    //之后读取输入，且要确保输入的坐标正确
    while(1){
        int row;
        char col;//因为显示的棋盘的列方向是字母A...
        int scanResult=scanf("%d %c",&row,&col);
        //检测到EOF，直接退出程序
        if (scanResult == EOF) {
            printf("\n检测到退出信号,游戏结束。\n");
            exit(0); // 直接终止程序
        }

        //输入格式有误
        if (scanResult != 2) {
            printf("输入格式错误(例如: 8 H)，请重新输入: ");
            
            // 清理输入缓冲区 (跳过错误数据，直到换行符)
            int c;
            while ((c = getchar()) != '\n' && c != EOF); 
            continue; 
        }

        //格式正确但数值错误
        if (row < 1 || row > 15 || col < 'A' || col > 'O') {
            printf("坐标越界(行1-15, 列A-O)，请重新输入: ");
            continue;
        }
        //转换坐标
        p.y=-row+SIZE;
        p.x=col-'A';
        
        //输入无误但要检查是否已有子
        if(arrayForInnerBoardLayout[p.y][p.x]!=EMPTY){//检验要落的点是否有棋子
            printf("该点处已有棋子，请重新选择\n");
            continue;
        }
        break;
    }

    return p;
}

