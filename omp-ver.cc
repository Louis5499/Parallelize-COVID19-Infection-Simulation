#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;

typedef struct {
    int iter;
    int node;
    int init_infected_node;
    int map_width;
    int map_height;
    double max_moving_radius;
    double max_infection_radius;
    double alpha_constant;
    double beta_constant;
} Parameter;
Parameter param;

const double DeadProbability[22] = {0.0, 0.001, 0.002, 0.004, 0.008, 0.014, 0.022, 0.032, 0.022, 0.014, 0.008, 0.004, 0.002, 0.001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
const double RecoveryRate[22] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.31, 0.43, 0.57, 0.73, 0.91, 1.0};

#define INFECTION_THRESHOLD 0.7
#define NODE_MAX_STEP 5
#define NODE_MAX_VELOCITY 4

#define NODE_STATE_SUSCEPTIBLE 0
#define NODE_STATE_INFECTIOUS 1
#define NODE_STATE_RECOVERED 2
#define NODE_STATE_DEAD 3

vector<vector<vector<int>>> map_list(720, vector<vector<int>>(720, vector<int>(5000))); // [Important] Be aware of insufficient memory space. Please scale up node number (5000) in a block if required.
vector<vector<int>> map_list_count(720, vector<int>(720, 0));

int blocksInWidth;
int blocksInHeight;
int blockFactor;

typedef struct _node Node;

int* calcBlockPosOfNode(int widthPos, int heightPos, int *returnPosArr) {
    returnPosArr[0] = widthPos / blockFactor; // For block width idx
    returnPosArr[1] = heightPos / blockFactor; // For block height idx
    return returnPosArr;
}

void removeElementFromVector(int blockI, int blockJ, int removeVal) {
    vector<int> &intendVec = map_list[blockI][blockJ];
    auto it = std::find(intendVec.begin(), intendVec.end(), removeVal);
    if(it != intendVec.end()) intendVec.erase(it);
    map_list_count[blockI][blockJ]--;
}

void addElementToVector(int blockI, int blockJ, int newVal) {
    vector<int> &intendVec = map_list[blockI][blockJ];
    int curIdx = map_list_count[blockI][blockJ];
    intendVec[curIdx] = newVal;
    map_list_count[blockI][blockJ]++;
}

// Node 的行為模式：
// 考量到 Node 移動特性，每個人對特定方向移動是有特定時間的 (age)
// Age 在等於 0 的時候會產生一個新的數值，代表這個人在走幾次 Iteration 才會改變方向
typedef struct _node {
    static int Node_Index_Incrementor;

    int index;                  // Node Index is the same as array index of node_list.
    double curPos[2];           // 2D Map Position
    int curBlockPos[2];

    // Node 本身狀況參數
    double velocity[2];
    int step;
    short state, nextState;
    short age;

    _node() {
        index = _node::Node_Index_Incrementor++;

        // Setup Initial Position
        // 產生最大小數位數為三位的位置
        // 這裡非常有可能兩個 Node 會擠在同一個位置上
        curPos[0] = rand() % (param.map_width * 1000) / 1000.0;
        curPos[1] = rand() % (param.map_height * 1000) / 1000.0;

        // 產生一個 Step 和 Velocity
        step = rand() % NODE_MAX_STEP + 1;
        velocity[0] = rand() % (NODE_MAX_VELOCITY * 2 * 1000) / 1000.0 - NODE_MAX_VELOCITY;
        velocity[1] = rand() % (NODE_MAX_VELOCITY * 2 * 1000) / 1000.0 - NODE_MAX_VELOCITY;

        // State
        state = NODE_STATE_SUSCEPTIBLE;
        age = 0;

        // cout << "[Node::constructor]: Node " << this->index << " Created with (" << curPos[0] << ", " << curPos[1] << ", " << velocity[0] << ", " << velocity[1] << ", " << age << ") State = " << state << endl;
    }

    // Node 的移動以及 Step 機制
    void move() {
        if (state == NODE_STATE_DEAD) return;
        
        // Update State
        state = nextState;

        // 移動
        // [TODO]: 這裡只寫了個簡單的判斷式，會導致 Node 可能永遠走不到靠著邊界
        int tmpX = curPos[0] + velocity[0], tmpY = curPos[1] + velocity[1];
        if (tmpX >= 0 && tmpX <= param.map_width) curPos[0] = tmpX;
        if (tmpY >= 0 && tmpY <= param.map_height) curPos[1] = tmpY;

        // Confirm whether the node has jump to another block
        int newBlockIdxs[2] = {0};
        calcBlockPosOfNode(curPos[0], curPos[1], newBlockIdxs);
        if (newBlockIdxs[0] != curBlockPos[0] || newBlockIdxs[1] != curBlockPos[1]) {
            // 1. Remove from original block map
            removeElementFromVector(curBlockPos[1], curBlockPos[0], index); // index == (Node_list 中此 Node 的 Array Index)
            // 2. Add to new block map
            addElementToVector(newBlockIdxs[1], newBlockIdxs[0], index); // index == (Node_list 中此 Node 的 Array Index)
            // 3. Renew self data
            curBlockPos[0] = newBlockIdxs[0];
            curBlockPos[1] = newBlockIdxs[1];
        }

        // Update Age
        step -= 1;

        // Check if need to update
        if (step == 0) {
            step = rand() % NODE_MAX_STEP + 1;
            velocity[0] = rand() % (NODE_MAX_VELOCITY * 2 * 1000) / 1000.0 - NODE_MAX_VELOCITY;
            velocity[1] = rand() % (NODE_MAX_VELOCITY * 2 * 1000) / 1000.0 - NODE_MAX_VELOCITY;
        }
    }

    // 根據 Neighbor 的狀況去計算是否 Change State
    // 這裡更新的東西是 next state，等到 move 的時候再一次更新
    void stateTransfer(double rate) {
        if (state == NODE_STATE_RECOVERED || state == NODE_STATE_DEAD) return;
        else if (state == NODE_STATE_SUSCEPTIBLE) {
            // 這個 State 需要 Check 感染機率
            if (rate >= INFECTION_THRESHOLD) {
                nextState = NODE_STATE_INFECTIOUS;
                age = 0;
            }
        } else if (state == NODE_STATE_INFECTIOUS) {
            // Infectious can turn into recoverd or dead
            // 直接 Random 出一個 rate
            double deadRate = rand() % 100 / 100.0;
            double recoveryRate = rand() % 100 / 100.0;
            if (deadRate < DeadProbability[age]) nextState = NODE_STATE_DEAD;
            else if (recoveryRate < RecoveryRate[age]) nextState = NODE_STATE_RECOVERED;
            age++;
        }
    }

} Node;
int Node::Node_Index_Incrementor = 0;

typedef struct _map {
    // Public Member
    Node * node_list;

    _map() {
        cout << "[Map::constructor]: Map Initialized with " << param.map_width << "X" << param.map_height << endl;
    };

    void generate_node() {
        // Calculating how many blocks in the map
        blocksInWidth = param.map_width/blockFactor;
        blocksInHeight = param.map_height/blockFactor;

        // init node_list
        // 把資料產生出來
        node_list = new Node[param.node];

        // 隨機散佈 Infectious 的 Node
        int tmp, counter = param.init_infected_node;
        while (counter > 0) {
            tmp = rand() % param.node;
            if (node_list[tmp].state != NODE_STATE_INFECTIOUS) {
                node_list[tmp].state = NODE_STATE_INFECTIOUS;
                counter--;
                // cout << "[Map::generate_node]: Node " << node_list[tmp].index << " Set Infected." << endl;
            }
        }

        // Distribute node to corresponding block
        for (int i=0; i<param.node; i++) {
            int blockIdx[2] = {0};
            calcBlockPosOfNode(node_list[i].curPos[0], node_list[i].curPos[1], blockIdx);
            node_list[i].curBlockPos[0] = blockIdx[0];
            node_list[i].curBlockPos[1] = blockIdx[1];
            addElementToVector(blockIdx[1], blockIdx[0], i);
        }
        // cout << "[Map::generate_node]: Node List Initialized with #Node = " << param.node << " #Infectious = " << param.init_infected_node << endl;
    }

    void random_walk(int iter = 1) {
        for (int i = 0; i < iter; i++) {
            // Random Work，把每個點的位置都隨便亂移
            // [TODO]: 這裡搞不好可以優化些什麼
            for (int j = 0; j < param.node; j++) {
                node_list[j].move();
            }
        }
    }

    ~_map() {
        delete [] this->node_list;
    };
} Map;

void extractParam(Parameter * param, char *argv[]);
inline double distance(Node &p1, Node &p2);
inline double infectionRate(double dist);
inline bool isExceedBoundary(int i, int j);

/*
    argv[1] --> Number of Iteration
    argv[2] --> Number of Nodes
    argv[3] --> Initial Number of Infectious Node
    argv[4] --> Map Width
    argv[5] --> Map Height
    argv[6] --> Max Moving Radius
    argv[7] --> Max Infection Radius
    argv[8] --> Infection Ratio Alpha for Probability of Infection
    argv[9] --> Infection Ratio Beta for Probability of Infection
*/
int main(int argc, char *argv[]) {

    assert(argc == 10);

    // Init Random Generator with a Seed
    srand(time(NULL));

    // Extract all Parameter to a Structure
    extractParam(&param, argv);

    // Init Map
    Map map;
    map.generate_node();

    // Algorithm Main Part
    for (int iter = 0; iter < param.iter; iter++) {
        // 總共要做這麼多輪
        for (int blockI = 0; blockI < blocksInWidth; blockI++) {
            for (int blockJ = 0; blockJ < blocksInHeight; blockJ++) {
                // TODO: Parallelize this phase, every thread responsible for a particular block
                for (int iOffset = -1; iOffset <= 1; iOffset++) {
                    for (int jOffset = -1; jOffset <= 1; jOffset++) {
                        const int actualBlockI = blockI + iOffset;
                        const int actualBlockJ = blockJ + jOffset;

                        if (isExceedBoundary(actualBlockI, actualBlockJ)) continue;

                        vector<int> inBlockNodeIdList = map_list[actualBlockI][actualBlockJ];
                        int blockNodeLen = map_list_count[actualBlockI][actualBlockJ];

                        for (int n1Idx = 0; n1Idx < blockNodeLen; n1Idx++) {
                            const int n1 = inBlockNodeIdList[n1Idx];
                            // Traverse Every Node & Check Neighbor
                            double total_infection_rate = 1;
                            if (map.node_list[n1].state == NODE_STATE_SUSCEPTIBLE) {
                                for (int n2Idx = 0; n2Idx < blockNodeLen; n2Idx++) {
                                    const int n2 = inBlockNodeIdList[n2Idx];
                                    if (n1 == n2 || map.node_list[n2].state != NODE_STATE_INFECTIOUS) continue;
                                    // Check Neighbor whether it's infected.
                                    double tmp_dist = distance(map.node_list[n1], map.node_list[n2]);
                                    if (tmp_dist <= param.max_infection_radius) {
                                        // Calculate Infection Rate of Each Node
                                        total_infection_rate *= infectionRate(tmp_dist);
                                    }
                                }
                                if (total_infection_rate == 1) total_infection_rate = 0;
                            } else {
                                total_infection_rate = 0;
                            }
                            // 確認是否需要換一個 State
                            map.node_list[n1].stateTransfer(total_infection_rate);
                        }
                    }
                } 
            }
        }
        map.random_walk();
        cout << "[Main]: Iteration " << iter << " Completed." << endl;
    }

    return 0;
}

void extractParam(Parameter * param, char *argv[]) {
    param->iter = atoi(argv[1]);
    param->node = atoi(argv[2]);
    param->init_infected_node = atoi(argv[3]);
    param->map_width = atoi(argv[4]);
    param->map_height = atoi(argv[5]);
    param->max_moving_radius = atof(argv[6]);
    param->max_infection_radius = atof(argv[7]);
    param->alpha_constant = atof(argv[8]);
    param->beta_constant = atof(argv[9]);

    // We initilize the blocking factor to infection radius. As a result, whenever we want to check how many people get infection under blocking version code, we only need to calculate adjacent blocks (9宮格) to check infection.
    blockFactor = param->max_infection_radius;

    // Self-Checking
    cout << "*---------------------------Input Parameter---------------------------*" << endl << endl;
    cout << left << setw(60) << "# Number of Iteration: " << param->iter << endl;
    cout << left << setw(60) << "# Number of Nodes: " << param->node << endl;
    cout << left << setw(60) << "# Initial Number of Infectious Node: " << param->init_infected_node << endl;
    cout << left << setw(60) << "# Map Width: " << param->map_width << endl;
    cout << left << setw(60) << "# Map Height: " << param->map_height << endl;
    cout << left << setw(60) << "# Max Moving Radius: " << param->max_moving_radius << endl;
    cout << left << setw(60) << "# Max Infection Radius: " << param->max_infection_radius << endl;
    cout << left << setw(60) << "# Infection Ratio Alpha for Probability of Infection: " << param->alpha_constant << endl;
    cout << left << setw(60) << "# Infection Ratio Beta for Probability of Infection: " << param->beta_constant << endl;
    cout << endl << "*---------------------------------------------------------------------*" << endl;
}

// Distance 算數
inline double distance(Node &p1, Node &p2) {
    return sqrt(pow(p1.curPos[0] - p2.curPos[0], 2) + pow(p1.curPos[1] - p2.curPos[1], 2));
}

// Probability 算數
inline double infectionRate(double dist) {
    // 如果拿 alpha = 0.99(最大值) beta = 2.5 distance = 3 --> 接近 0
    return param.alpha_constant * exp(-1 * dist * param.beta_constant);
}

// 確保目前計算的 Block 沒有超過邊界
inline bool isExceedBoundary(int i, int j) {
    if (i < 0 || i >= param.map_height) return true;
    else if (j < 0 || j >= param.map_width) return true;
    else return false;
}