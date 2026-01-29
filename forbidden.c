#include "myhead.h"
#include "forbidden.h"

// 辅助宏：判断坐标是否在棋盘内
#define IS_VALID(x, y) ((x) >= 0 && (x) < SIZE && (y) >= 0 && (y) < SIZE)

// 辅助函数：获取指定位置的棋子颜色，越界视为白棋（也就是对手/墙壁）
// 因为对于黑棋来说，墙壁和白棋的效果是一样的，都是阻挡
static int getStone(int x, int y) {
    if (!IS_VALID(x, y)) return WHITE; 
    return arrayForInnerBoardLayout[y][x];
}

// 核心分析函数：分析某个方向上形成的棋型
// 参数：x, y 落子点; dx, dy 方向向量
// 返回值：
// 0 - 什么都不是
// 1 - 活三 (Live Three)
// 2 - 四 (冲四或活四) (Four)
// 3 - 五连 (Five)
// 4 - 长连 (Overline)
static int analyzeLine(int x, int y, int dx, int dy) {
    // 临时数组存储该方向上的棋型，中心点在索引 4
    // 范围取前后各4格，共9格： O O O O X O O O O
    int line[9]; 
    
    // 1. 提取线条信息
    for (int i = -4; i <= 4; i++) {
        int cx = x + i * dx;
        int cy = y + i * dy;
        if (i == 0) {
            line[4 + i] = BLACK; // 假设当前点落了黑子
        } else {
            line[4 + i] = getStone(cx, cy);
        }
    }

    // 2. 扫描连珠情况 (Count Consecutive)
    // 只需要看包含中心点(索引4)的连珠
    int maxLen = 0;
    int currentLen = 0;

    
    // 判断长连(6个及以上)
    int left = 4, right = 4;
    while (left > 0 && line[left - 1] == BLACK) left--;
    while (right < 8 && line[right + 1] == BLACK) right++;
    int len = right - left + 1;
    if (len >= 6) return 4; // 长连
    if (len == 5) return 3; // 五连

    // 接下来判断四和活三
    // 为了准确判断跳四和跳三，我们分析中心点左右的结构
    
    // 我们的目标是识别以下模式 (B=Black, E=Empty, W=White/Wall)
    // 活三： E B B B E (标准) 或 E B E B B E (跳三)
    // 四：   B B B B (活四/冲四) 或 B E B B B (跳四) 等
    
    // 我们枚举所有经过中心点的 5 格或 6 格窗口来检测
    
    int fourCount = 0;
    int threeCount = 0; // 活三计数

    // A. 检测“四” (Four)
    // 特征：5个格子里有4个黑子，或者6个格子里有4个黑子且中间空1个
    
    // 扫描所有可能的5格窗口 (索引 0-4 到 4-8)
    for(int i = 0; i <= 4; i++) {
        // 窗口范围 line[i] 到 line[i+4]
        // 必须包含中心点 index 4
        if (i > 4 || i + 4 < 4) continue;
        
        int bCount = 0;
        int eCount = 0;
        for(int k=0; k<5; k++) {
            if(line[i+k] == BLACK) bCount++;
            else if(line[i+k] == EMPTY) eCount++;
        }
        
        if(bCount == 4 && eCount == 1) return 2; // 这是一个四 (可能是冲四也可能是活四，禁手规则里统称四)
        if(bCount == 5) return 3; // 五连
    }

    // B. 检测“活三” (Live Three)
    // 活三必须是“本方再走一着可以形成活四”。
    // 典型形状： 0 1 1 1 0 (标准) 或 0 1 0 1 1 0 (跳三)
    // 关键点：两端必须是空的，且被封堵后不能成四
    
    // 模式1：标准活三 0 1 1 1 0
    if (line[3]==BLACK && line[5]==BLACK && line[2]==EMPTY && line[6]==EMPTY) {
        // 中心是4，且连续3个黑子，两头空
        // 还要确保不是 0 1 1 1 1 0 (那是四) -> 前面已经check过四了，这里安全
        return 1;
    }
    
    // 模式2：左跳活三 0 1 0 1 1 0  (中心在右边两个1里)
    if (line[3]==EMPTY && line[2]==BLACK && line[1]==EMPTY && line[5]==BLACK && line[6]==EMPTY) return 1;
    
    // 模式3：右跳活三 0 1 1 0 1 0 (中心在左边两个1里)
    if (line[5]==EMPTY && line[6]==BLACK && line[7]==EMPTY && line[3]==BLACK && line[2]==EMPTY) return 1;

    // 模式4：中心跳活三 0 1 1 0 1 0 (中心在 0 的位置? 不对，中心必须是刚下的子)
    // 如果刚下在中间空位： 1 (1) 1 -> 变成 1 1 1，这回归到模式1

    return 0;
}

int isForbiddenPoint(int x, int y){
    
    //如果这个点本身已经有子，不能下
    if (arrayForInnerBoardLayout[y][x] != EMPTY) return 0;

    int fiveCount = 0;
    int overlineCount = 0;
    int fourCount = 0;
    int threeCount = 0;

    // 四个方向：横、竖、撇、捺
    int dxs[] = {1, 0, 1, 1};
    int dys[] = {0, 1, 1, -1};

    for (int i = 0; i < 4; i++) {
        int result = analyzeLine(x, y, dxs[i], dys[i]);
        
        if (result == 3) fiveCount++;       // 五连
        else if (result == 4) overlineCount++; // 长连
        else if (result == 2) fourCount++;  // 四
        else if (result == 1) threeCount++; // 活三
    }

    // 禁手优先级判断规则：
    
    // 黑方五连与禁手同时形成，禁手失效，黑方胜
    if (fiveCount > 0) return 0; 

    // 长连禁手：形成的5个以上同色棋子
    if (overlineCount > 0) return 1;

    // 四四禁手：形成两个或两个以上的冲四或活四
    if (fourCount >= 2) return 1;

    // 三三禁手：形成两个或两个以上的活三
    if (threeCount >= 2) return 1;

    return 0; // 不是禁手
}