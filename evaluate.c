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
// 全局：记录上次 AI 决策用时（秒）
double lastAIDuration = 0.0;


// 如果没有威胁点则返回{-1,-1)，否则返回封堵威胁的点
Point handle_opponent_fours(int myColor) {
    Point res = {-1, -1};
    if (currentPoint.x < 0 || currentPoint.y < 0) return res;
    int enemyColor = (myColor == BLACK) ? WHITE : BLACK;
    if (arrayForInnerBoardLayout[currentPoint.y][currentPoint.x] != enemyColor) return res;

    int lr = currentPoint.y, lc = currentPoint.x;
    int found = 0;
    // 检查任何方向的四个点 (包括冲四/活四)
    for (int d = 0; d < 4; d++) {
        LineInfo li = countLine(lc, lr, enemyColor, d);
        if (li.count >= 4) { found = 1; break; }
    }
    if (!found) return res;

    // 如果AI能直接赢则下该点
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (arrayForInnerBoardLayout[r][c] != EMPTY) continue;
            if (myColor == BLACK && isForbiddenPoint(c, r)) continue;
            if (!hasNeighbor(c, r, DISTANCE)) continue;
            if (evaluateOnecolor(c, r, myColor) >= WIN) return (Point){c, r};
        }
    }

    // 收集上一步棋(对手)的封堵点
    Move cand[SIZE*SIZE]; int cc = 0;
    int rowS = (lr-4<0)?0:lr-4; int rowE = (lr+4>SIZE-1)?SIZE-1:lr+4;
    int colS = (lc-4<0)?0:lc-4; int colE = (lc+4>SIZE-1)?SIZE-1:lc+4;
    for (int r = rowS; r <= rowE; r++){
        for (int c = colS; c <= colE; c++){
            if (arrayForInnerBoardLayout[r][c] != EMPTY) continue;
            int def = evaluateOnecolor(c, r, enemyColor);
            if (def >= LIVE3) {
                cand[cc].p.x = c; cand[cc].p.y = r; cand[cc].score = def + WIN/2; cc++;
            }
        }
    }
    if (cc==0) return res;
    if (cc>1) qsort(cand, cc, sizeof(Move), compareMoves);
    return cand[0].p;
}

Point handle_opponent_broken4(int myColor){
    return handle_opponent_fours(myColor);
}

Point handle_opponent_live3(int myColor){
    Point res = {-1,-1};
    if (currentPoint.x < 0 || currentPoint.y < 0) return res;
    int enemyColor = (myColor == BLACK) ? WHITE : BLACK;
    if (arrayForInnerBoardLayout[currentPoint.y][currentPoint.x] != enemyColor) return res;

    int lr = currentPoint.y, lc = currentPoint.x;
    int live3_found = 0, live4_found = 0;
    for (int d=0; d<4; d++){
        LineInfo li = countLine(lc, lr, enemyColor, d);
        if (li.count == 3 && li.blocked == LIVE) live3_found = 1;
        if (li.count >= 4) live4_found = 1;
    }
    if (!live3_found) return res;

    // 如果我方有直接必胜或能造活四，优先反击
    for (int r = 0; r < SIZE; r++){
        for (int c = 0; c < SIZE; c++){
            if (arrayForInnerBoardLayout[r][c] != EMPTY) continue;
            if (myColor == BLACK && isForbiddenPoint(c, r)) continue;
            if (!hasNeighbor(c, r, DISTANCE)) continue;
            int myAttack = evaluateOnecolor(c, r, myColor);
            if (myAttack >= WIN) return (Point){c, r};
            if (myAttack >= LIVE4) return (Point){c, r};
        }
    }

    // 否则必须堵活三，收集封堵点并返回最佳
    Move cand[SIZE*SIZE]; int cc=0;
    int rowS = (lr-4<0)?0:lr-4; int rowE = (lr+4>SIZE-1)?SIZE-1:lr+4;
    int colS = (lc-4<0)?0:lc-4; int colE = (lc+4>SIZE-1)?SIZE-1:lc+4;
    for (int r=rowS;r<=rowE;r++){
        for (int c=colS;c<=colE;c++){
            if (arrayForInnerBoardLayout[r][c] != EMPTY) continue;
            int def = evaluateOnecolor(c, r, enemyColor);
            if (def >= LIVE3){ cand[cc].p.x=c; cand[cc].p.y=r; cand[cc].score=def+WIN/2; cc++; }
        }
    }
    if (cc==0) return res;
    if (cc>1) qsort(cand, cc, sizeof(Move), compareMoves);
    return cand[0].p;
}

Point getBestMove(int myColor) {
    Point bestMove = {-1, -1};
    int enemyColor = (myColor == BLACK) ? WHITE : BLACK;

    //如果开局AI先行，直接下天元
    if (isBoardEmpty()) {
        bestMove.x = 7; bestMove.y = 7;
        return bestMove;
    }

     // 初始化时间
    startTime = clock();
    isTimeOut = 0;
    double timeLimit = 8.0; // 8秒限制

    // 准备候选缓冲区（用于在发现紧急威胁时直接使用）
    Move candidateMoves[SIZE*SIZE];
    int candidateCount = 0;
    int useCandidates = 0; // 1 表示后续迭代加深只使用 candidateMoves

    // 1) 先判断我方有没有一步必胜点 (先手必赢)
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            if (arrayForInnerBoardLayout[row][col] != EMPTY) continue;
            if (myColor == BLACK && isForbiddenPoint(col, row)) continue;
            if (!hasNeighbor(col, row, DISTANCE)) continue;

            int attackScore = evaluateOnecolor(col, row, myColor);
            if (attackScore >= WIN) {
                bestMove.x = col;
                bestMove.y = row;
                return bestMove; // 直接赢
            }
        }
    }

    // 2) 用封装函数优先处理对手最近一手的威胁（若有则直接返回应对点）
    Point threatPoint;
    threatPoint = handle_opponent_fours(myColor);
    if (threatPoint.x != -1) return threatPoint;
    threatPoint = handle_opponent_broken4(myColor);
    if (threatPoint.x != -1) return threatPoint;
    threatPoint = handle_opponent_live3(myColor);
    if (threatPoint.x != -1) return threatPoint;

    // 生成第一层候选点
    Move rootMoves[SIZE*SIZE];
    int rootCount = 0;
    if (useCandidates) {
        // 使用前面收集到的紧急候选点
        for (int i = 0; i < candidateCount; i++) rootMoves[rootCount++] = candidateMoves[i];
    } else {
        // 否则按原有逻辑生成候选点
        rootCount = generateMoves(rootMoves, myColor);
    }

    // 迭代加深
    for (int depth = 2; depth <= 20; depth += 2) {
        Point currentBestMove = {-1, -1};
        int currentBestScore = -INF;
        int alpha = -INF; 
        int beta = INF;

        for (int i = 0; i < rootCount; i++) {
            makeMove(rootMoves[i].p, myColor);
            // 调用 minimax
            int score = minimax(depth - 1, alpha, beta, 0, myColor);
            unmakeMove(rootMoves[i].p);

            // 检查是否超时
            if ((double)(clock() - startTime) / CLOCKS_PER_SEC >= timeLimit) {
                isTimeOut = 1;
                break; // 跳出循环
            }

            if (score > currentBestScore) {
                currentBestScore = score;
                currentBestMove = rootMoves[i].p;
            }
            if (currentBestScore > alpha) alpha = currentBestScore;
        }
        // 如果在这一层搜索过程中超时了，丢弃这一层结果并回退
        if (isTimeOut) {
            break; 
        } else {
            // 如果没超时，更新最佳移动
            bestMove = currentBestMove;
            // 若已找到必胜点则可以提前停止
            if (currentBestScore >= WIN - 20) break; 
        }
    }

    // 打印本次AI决策消耗的总时长（只打印最终落子的用时）
    double elapsed = (double)(clock() - startTime) / CLOCKS_PER_SEC;
    // 不在此处打印，保存到全局以便外层在清屏后输出
    lastAIDuration = elapsed;

    return bestMove;
}


int getPointScore(int x,int y,int myColor){//输入一个点的坐标x:横坐标列（col）；y：纵坐标行（row）。返回该点得分
    //确定对手颜色
    int enemyColor=(myColor==BLACK)? WHITE:BLACK;
    int attackScore=evaluateOnecolor(x,y,myColor);//计算自己下这个点的得分
    int defenseScore=evaluateOnecolor(x,y,enemyColor);//计算对手下这个点能得的分 

    if(attackScore>=WIN || defenseScore>=WIN) return WIN; //在这儿落子，要么自己赢，要么对手赢，所以必须下这里
    if(attackScore>=LIVE4) return LIVE4; //有活四先下活四

    return attackScore+defenseScore*1.2;//综合得分，防守权重稍微高一点
}

int lineScore(int x,int y, LineInfo lineinfo){//计算一条线上的得分,返回值为对应棋型的分数
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
    // 将四个方向的线型先分别评估，再合并判断复杂棋形（双活三、双四等）
    int dirScores[4];
    LineInfo info;
    int live3_count = 0;
    int live4_count = 0;
    int broken4_count = 0;
    int total = 0;

    // 横、竖、主对角、副对角
    info = countLine(x,y,Color,HOR);   //暂存一条线上的信息
    dirScores[0] = lineScore(x,y,info);
    
    info = countLine(x,y,Color,VER);   
    dirScores[1] = lineScore(x,y,info);
    
    info = countLine(x,y,Color,DIAG_M);
    dirScores[2] = lineScore(x,y,info);
    
    info = countLine(x,y,Color,DIAG_S);
    dirScores[3] = lineScore(x,y,info);

    for(int i=0;i<4;i++){
        int s = dirScores[i];
        if(s>=WIN) return WIN; // 任何方向直接成五/长连
        if(s==LIVE4) live4_count++;
        if(s==BROKEN4) broken4_count++;
        if(s==LIVE3) live3_count++;
        total += s;
    }

    
    // 双活四（存在任一活四已足够）
    if(live4_count>0) return LIVE4;
    // 双冲四/活四+冲四/两个冲四视为非常危险，提升为接近活四的权重
    if(broken4_count>=2) return LIVE4;
    if(broken4_count>=1 && live3_count>=1) return LIVE4;

    // 双活三（两处活三通常等价于必胜威胁），提升到活四级别以在搜索中优先处理
    if(live3_count>=2) return LIVE4;

    // 活三也要堵：单个活三视为需要优先防守的威胁，返回 LIVE3
    if(live3_count==1) return LIVE3;

    // 若无关键组合，返回四个方向分数之和（保留原有细分）
    return total;
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
    int enemyColor = (myColor==BLACK)? WHITE:BLACK;
    int visited[SIZE][SIZE] = {0};//标记已经加入候选列表的点，避免重复加入

    

    // 先扫描是否存在一步必胜（我方），若存在直接返回该步
    for(int row=0;row<SIZE;row++){
        for(int col=0;col<SIZE;col++){
            if(arrayForInnerBoardLayout[row][col]!=EMPTY) continue;
            int score = getPointScore(col,row,myColor);
            if(score >= WIN){
                moves[0].p.x = col;
                moves[0].p.y = row;
                moves[0].score = score;
                return 1; // 直接返回必胜点
            }
        }
    }

// 优先检测对手最近一次落子是否直接造成威胁（活三/活四/冲四）
    // 如果对手刚刚形成威胁，则只返回针对这些威胁的封堵点（强制防守）
    if (currentPoint.x >= 0 && currentPoint.y >= 0) {
        int lr = currentPoint.y;
        int lc = currentPoint.x;
        if (arrayForInnerBoardLayout[lr][lc] == enemyColor) {
            int threat = 0;
            for (int d = 0; d < 4; d++) {
                LineInfo li = countLine(lc, lr, enemyColor, d);
                if (li.count >= 4) threat = 1; // 活四/冲四/四连
                if (li.count == 3 && li.blocked == LIVE) threat = 1; // 活三
            }
            if (threat) {
                // 搜索最近区域内的封堵点（范围±4）并作为唯一候选返回
                int blk_count = 0;
                int rowS = (lr-4<0)?0:lr-4;
                int rowE = (lr+4>SIZE-1)?SIZE-1:lr+4;
                int colS = (lc-4<0)?0:lc-4;
                int colE = (lc+4>SIZE-1)?SIZE-1:lc+4;
                for (int r = rowS; r <= rowE; r++) {
                    for (int c = colS; c <= colE; c++) {
                        if (arrayForInnerBoardLayout[r][c] != EMPTY) continue;
                        int defScore = evaluateOnecolor(c, r, enemyColor);
                        if (defScore >= LIVE3) {
                            moves[blk_count].p.x = c;
                            moves[blk_count].p.y = r;
                            moves[blk_count].score = defScore + WIN; // 强制高优先级
                            blk_count++;
                            if (blk_count >= SIZE*SIZE) break;
                        }
                    }
                    if (blk_count >= SIZE*SIZE) break;
                }
                if (blk_count > 0) {
                    if (blk_count > 1) qsort(moves, blk_count, sizeof(Move), compareMoves);
                    return blk_count;
                }
            }
        }
    }

    // 扫描对手一步必胜的点（必须防守），优先加入
    for(int row=0;row<SIZE;row++){
        for(int col=0;col<SIZE;col++){
            if(arrayForInnerBoardLayout[row][col]!=EMPTY) continue;
            int defenseScore = getPointScore(col,row,enemyColor);
            if(defenseScore >= WIN){
                moves[count].p.x = col;
                moves[count].p.y = row;
                moves[count].score = defenseScore + WIN; // 极高优先级
                visited[row][col] = 1;
                count++;
            }
        }
    }

    // 若存在对手的活三（需要及时封堵），优先把这些封堵点放到候选列表前面
    // 单独提高活三封堵点的权重，确保它们在后续的候选生成和排序中靠前
    for(int row=0;row<SIZE;row++){
        for(int col=0;col<SIZE;col++){
            if(arrayForInnerBoardLayout[row][col]!=EMPTY) continue;
            if(visited[row][col]) continue;
            int defenseScore = evaluateOnecolor(col,row,enemyColor);
            if(defenseScore == LIVE3){
                moves[count].p.x = col;
                moves[count].p.y = row;
                // 给活三封堵点更高的临时权重，确保排在普通高危点前面
                moves[count].score = defenseScore + (WIN/5);
                visited[row][col] = 1;
                count++;
            }
        }
    }

    // 再收集需要优先防守的高危点（敌方活三/活四等），赋高分以排在前面
    for(int row=0;row<SIZE;row++){
        for(int col=0;col<SIZE;col++){
            if(arrayForInnerBoardLayout[row][col]!=EMPTY) continue;
            if(visited[row][col]) continue;
            int defenseScore = evaluateOnecolor(col,row,enemyColor);
            if(defenseScore >= LIVE3){
                // 若该点对手威胁较大，则先考虑
                moves[count].p.x = col;
                moves[count].p.y = row;
                moves[count].score = defenseScore + (WIN/10);
                visited[row][col] = 1;
                count++;
            }
        }
    }

    // 常规候选生成（邻域+禁手过滤），并跳过已加入的点
    for(int row=0;row<SIZE;row++){
        for(int col=0;col<SIZE;col++){
            if(arrayForInnerBoardLayout[row][col]!=EMPTY) continue;
            if(visited[row][col]) continue;

            // 如果是黑子，检查禁手，如果是禁手则跳过
            if(myColor==BLACK && isForbiddenPoint(col,row)==1) continue;

            // 只搜索周围 DISTANCE 范围内的点
            if(!hasNeighbor(col,row,DISTANCE)) continue;

            moves[count].p.x = col;
            moves[count].p.y = row;
            moves[count].score = getPointScore(col,row,myColor);
            count++;
        }
    }

    // 对候选点按分数排序（高->低）
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

    // 存置换表
    // 如果没有超时，才把结果存进去；否则存的是半成品，有错误
    if (!isTimeOut) {
        // 替换策略：始终替换
        tt->key = currentHash;
        tt->depth = depth;
        tt->score = bestValue;
        tt->flag = hashFlag;
    }
    return bestValue;
}