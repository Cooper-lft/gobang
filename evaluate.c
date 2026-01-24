#include "myhead.h"
#include "evaluate.h"
#include "forbidden.h"

// 全局 Zobrist 数组：[行][列][颜色(0空, 1黑, 2白)]
unsigned long long zArray[SIZE][SIZE][3];
unsigned long long currentHash = 0;
TTEntry tTable[TT_SIZE];

// 生成随机 64 位整数
unsigned long long rand64() {
    return (unsigned long long)rand() ^ 
           ((unsigned long long)rand() << 15) ^ 
           ((unsigned long long)rand() << 30) ^ 
           ((unsigned long long)rand() << 45) ^ 
           ((unsigned long long)rand() << 60);
}

// 初始化 Zobrist 数组 (在 main 函数最开始调用一次)
void initZobrist() {
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            zArray[i][j][BLACK] = rand64();
            zArray[i][j][WHITE] = rand64();
        }
    }
    currentHash = 0; // 初始空棋盘 Hash 为 0
    // 清空置换表
    memset(tTable, 0, sizeof(tTable)); 
}

// 全局变量，用于在递归深处通知超时
int isTimeOut = 0; 
clock_t startTime;


//参数是己方的棋子颜色
Point getBestMove(int myColor) {
    Point bestMove = {-1, -1};
    
    //如果开局AI先行，直接下天元
    if (isBoardEmpty()) {
        bestMove.x = 7; bestMove.y = 7;
        return bestMove;
    }

     // 初始化时间
    startTime = clock();
    isTimeOut = 0;
    double timeLimit = 8.0; // 8秒限制

    //生成第一层候选点(即Max层的子分支，AI可能要落的点)
    Move moves[SIZE*SIZE];
    int count = generateMoves(moves, myColor);
    
    // 迭代加深 (Iterative Deepening)
    // 从 2 层开始搜，搜完 2 层如果时间够，搜 4 层，然后 6 层...
    // 每次深度增加，都会利用 TT 表中的数据加速
    for (int depth = 2; depth <= 20; depth += 2) {
        
        Point currentBestMove = {-1, -1};
        int currentBestScore = -INF;
        
        // --- 根节点搜索 (复制一部分代码出来以便获取 Move) ---
        Move moves[SIZE*SIZE];
        int count = generateMoves(moves, myColor);
        
        int alpha = -INF; 
        int beta = INF;

        // 这里我们把 sort(moves) 加上
        // 并且！如果有上一层搜到的 bestMove (TT表中可能有)，应该把它排在第一个！
        // (PV-Move Ordering，暂且不写这么复杂，靠 qsort 也就够了)

        for (int i = 0; i < count; i++) {
            makeMove(moves[i].p, myColor);
            
            // 调用 minimax
            int score = minimax(depth - 1, alpha, beta, 0, myColor);
            
            unmakeMove(moves[i].p);

            // 【关键】检查是否超时
            if ((double)(clock() - startTime) / CLOCKS_PER_SEC >= timeLimit) {
                isTimeOut = 1;
                break; // 跳出循环
            }

            if (score > currentBestScore) {
                currentBestScore = score;
                currentBestMove = moves[i].p;
            }
            if (currentBestScore > alpha) alpha = currentBestScore;
        }

        // 如果在这一层搜索过程中超时了，这一层的结果是不完整的，不可信！
        // 我们必须丢弃这一层的结果，使用上一层 (depth-2) 找到的 bestMove
        if (isTimeOut) {
            printf("深度 %d 搜索超时，回退使用深度 %d 的结果\n", depth, depth - 2);
            break; 
        } else {
            // 如果没超时，更新最佳移动
            bestMove = currentBestMove;
            printf("深度 %d 完成，最佳点 (%d,%d) 分数 %d，耗时 %.2fs\n", 
                   depth, bestMove.y, bestMove.x, currentBestScore, 
                   (double)(clock() - startTime) / CLOCKS_PER_SEC);
            
            // 如果已经找到必胜路径，没必要再深搜了
            if (currentBestScore >= WIN - 20) break; 
        }
    }

    return bestMove;
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

int hasNeighbor(int x,int y,int distance){//判断(x,y)附近distance距离内是否有棋子
    //确定扫描的矩形范围，不要超过边界
    int rowStart=((y-distance)<0)? 0:y-distance;
    int rowEnd=((y+distance)>SIZE-1)? SIZE-1:y+distance;
    int colStart=((x-distance)<0)? 0:x-distance;
    int colEnd=((x+distance)>SIZE-1)? SIZE-1:x+distance;

    //遍历矩形范围内的棋子看是否有子（neighbor）
    for(int i=rowStart;i<=rowEnd;i++){
        for(int j=colStart;j<=colEnd;j++){
            if(arrayForInnerBoardLayout[i][j]!=EMPTY&&!(i==y&&j==x)){
                return 1;//有邻居
            }
        }
    }
    return 0;//扫描结束也没有返回，则无邻居
}

int generateMoves(Move moves[],int myColor){//返回值是有效候选点的个数，参数一个是候选点数组作为列表，一个是本次模拟的落子颜色
    int count=0;//记录候选点个数
    //扫描全盘
    for(int row=0;row<SIZE;row++){
        for(int col=0;col<SIZE;col++){
            //落子处必须没子才行：
            if(arrayForInnerBoardLayout[row][col]!=EMPTY){
                continue;
            }
            //如果是黑子，检查禁手，如果是禁手则跳过
            if(myColor==BLACK&&isForbiddenPoint(col,row)==1){
                continue;
            }
            //只搜索周围2x2的矩形内的点是否有棋子（distance==2），以提高效率
            if(!hasNeighbor(col,row,DISTANCE)){
                continue;
            }

            //走到这一步说明是一个值得搜索的点
            //记录坐标
            moves[count].p.x=col;
            moves[count].p.y=row;

            //用之前的获取分数函数对当前下这处的选择打分，便于排序
            moves[count].score=getPointScore(col,row,myColor);
            count++;
        }
    }
    //对所有候选点进行排序，从高到底，以便于剪枝,更快剪掉不必要的分支
    //这里使用快速排序
    if(count>1){
        qsort(moves,count,sizeof(Move),compareMoves);
    }
    return count;
}

//比较函数，用于qsort，对候选点作比较，返回值<0：a排在b前面（a得分高）（为了适配sort一般默认小的在前，所以这里为了分数高的在前，把b的分数减a的分数）
int compareMoves(const void*a,const void*b){
    const Move *moveA=(const Move *)a;
    const Move *moveB=(const Move *)b;
    return (moveB->score)-(moveA->score);//因为分数高的在前，所以用b-a
}

//全局评分函数，在搜索到根节点时评分
int evaluateBoard(int myColor){
    int myScore=0;
    int enemyScore=0;
    int enemyColor=(myColor==BLACK)? WHITE:BLACK;

    //遍历棋盘,计算自己的总得分和敌人总得分
    for(int row=0;row<SIZE;row++){
        for(int col=0;col<SIZE;col++){
            int state=arrayForInnerBoardLayout[row][col];
            if(state==myColor){
                myScore+=evaluateOnecolor(col,row,myColor);//这里会有重复计算分数的误差，比如活三中三颗棋子会计算三次
            }else if(state==enemyColor){
                enemyScore+=evaluateOnecolor(col,row,enemyColor);
            }
        }
    }
    return myScore-enemyScore;//评估局势
}

//模拟落子和撤销落子（悔棋）的函数：

//模拟落子
void makeMove(Point p,int Color){
    arrayForInnerBoardLayout[p.y][p.x]=Color;
    // 异或操作：对应的位置“加上”这个颜色的子
    currentHash ^= zArray[p.y][p.x][Color]; 
}
//撤销落子（悔棋）
void unmakeMove(Point p){
    // 必须先取出颜色，因为置为 EMPTY 后就不知道刚才是什么色了
    // 优化：调用者通常知道颜色，或者直接从棋盘读
    int color = arrayForInnerBoardLayout[p.y][p.x]; 

    arrayForInnerBoardLayout[p.y][p.x] = EMPTY;
    // 异或操作：对应的位置“减去”这个颜色的子 (异或两次等于还原)
    currentHash ^= zArray[p.y][p.x][color];
}

//递归极大极小值函数
//参数depth：剩余搜索深度
//参数alpha：是在调用该函数之前，对于Max层
//参数beta: Min层的当前(之后至少要<=beta),alpha和beta是历史上记录的Max层的下界和Min层的上界
//参数isMax: 1表示轮到AI了(Max层),0表示轮到对手(Min层)(标记当前是谁的回合)
//参数myColor: 为定值，始终表明AI的棋子颜色
//返回值: 返回在该层某个子树的根节点代表的落子通过minimax得到的最终局势评分
int minimax(int depth, int alpha, int beta, int isMax, int myColor){
    
    // 检查超时 (每隔几千次检查一次，避免频繁调用 clock 影响性能)
    static int checkCounter = 0;
    if (++checkCounter > 2000) {
        checkCounter = 0;
        if ((double)(clock() - startTime) / CLOCKS_PER_SEC >= 8.0) {
            isTimeOut = 1; // 标记超时
        }
    }
    if (isTimeOut) return 0; // 随便返回个值，反正会被上层丢弃

    
    if (isTimeOut) return 0; // 如果外层标记超时，直接返回，不再计算
    //查置换表
    // 计算下标：currentHash % TT_SIZE (用 & 运算更快)
    int ttIndex = currentHash & (TT_SIZE - 1);
    TTEntry *tt = &tTable[ttIndex];

    // 如果校验码匹配，且表里的深度 >= 当前要求的深度 (说明表里的结果更靠谱)
    if (tt->key == currentHash && tt->depth >= depth) {
        if (tt->flag == HASH_EXACT) {
            return tt->score; // 找到了精确值，直接返回，剪枝成功！
        }
        else if (tt->flag == HASH_ALPHA && tt->score <= alpha) {
            return alpha; // 以前搜过，这分支上限很低，肯定不如现在的 alpha 好
        }
        else if (tt->flag == HASH_BETA && tt->score >= beta) {
            return beta; // 以前搜过，这分支下限很高，肯定会被外面剪枝
        }
    }

    //收敛条件：
    //1.搜索过程中游戏结束
    int currentScore=evaluateBoard(myColor);
    if(currentScore>=WIN){
        return WIN+depth;
    }
    if(currentScore<= -WIN){
        return (-WIN)-depth;
    }

    //2.搜到底了
    if(depth==0){
        return currentScore;
    }

    //如果没有收敛，那么还可以生成候选点：
    Move moves[150];//临时存储候选点
    int enemyColor=(myColor==BLACK)? WHITE:BLACK;
    int count=generateMoves(moves,(isMax?myColor:enemyColor));

    //如果没有棋可以下：
    if(count==0) return 0;//平局

    // 用于记录存表标志
    int hashFlag = HASH_ALPHA; // 用于记录这次搜索结果的类型 (精确/上界/下界)
    int bestValue;

    //当前是Max层，即己方将要落子时：
    if(isMax){
        bestValue=-INF;//在该层遍历得到的局部最大值，要跟先前已经走过的分支的总的最大值alpha比较，且alpha会随时更新

        //遍历候选点，此时会用上剪枝算法
        for(int i=0;i<count;i++){
            makeMove(moves[i].p,myColor);//模拟己方落子
            //递归调用minimax函数来判断落在该点处预测到的全局分数并记录
            int value=minimax(depth-1,alpha,beta,0,myColor);//因为上一行模拟了AI落子，所以该对手落子了，所以isMax是0
            unmakeMove(moves[i].p);//撤销落子

            if (isTimeOut) return 0; // 递归回来如果超时了，立刻退出

            //更新当前Max层所模拟到的最大值全局得分
            if(value>bestValue) bestValue=value;
            //更新alpha 
            if(bestValue>alpha){
                alpha=bestValue;//剪枝：如果发现这个分支比上一层Min层允许的最大值（beta值）还要大，则Min层不会允许我走到这个分支，因此可以停止搜索了，剪掉
                hashFlag=HASH_EXACT;// 如果找到了比当前 alpha 更好的值，说明在这个窗口内它是精确的
            }
            if(beta<=alpha) {
                hashFlag = HASH_BETA; // 触发剪枝,这是一个下界(Lower Bound)，真实值 >= beta
                break;
            }
        }
    }else{//当前是Min层，对方要落子
        bestValue=INF;
        hashFlag = HASH_BETA; // 默认：如果没有低于 beta，说明这是个下界(Lower Bound)

        for(int i=0;i<count;i++){
            //模拟对手落子
            makeMove(moves[i].p,enemyColor);
            //递归调用，下一步棋该AI，isMax为1
            int value=minimax(depth-1,alpha,beta,1,myColor);
            //撤销落子
            unmakeMove(moves[i].p);

            //超时检查
            if (isTimeOut) return 0;

            //更新局部最小值
            if(value<bestValue){ 
                bestValue=value;
            //更新beta(该分支已遍历的子分支中的局部最小值小于该分支之前得到的最小值beta时)
            }
            if(bestValue<beta){
                beta=bestValue;
            //剪枝：如果对手发现这个分支的可能取的得分(因为是Min层，所以可能取值一定是取局部最小值)比先前已知晓的保底最大值alpha还低,说明Max层绝不会选这条分支，就不用算了
                hashFlag = HASH_EXACT; // 在当前 alpha-beta 窗口内，这是精确值
            }
            if(beta<=alpha){ 
                hashFlag = HASH_ALPHA; // 这是一个上界(Upper Bound)，真实值 <= alpha
                break;
            }
        }
    }

    // 5. 存置换表 (Store TT)
    // 如果没有超时，才把结果存进去；否则存的是半成品，有错误
    if (!isTimeOut) {
        // 替换策略：始终替换，或者深度更深时替换 (这里用简单的总是替换策略，或者 depth >= tt->depth)
        // 实际上最好是: if (tt->key == 0 || depth >= tt->depth)
        tt->key = currentHash;
        tt->depth = depth;
        tt->score = bestValue;
        tt->flag = hashFlag;
    }
    return bestValue;
}