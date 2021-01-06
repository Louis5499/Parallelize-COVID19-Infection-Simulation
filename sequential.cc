#include <iostream>
#include <iomanip>
#include <stdlib.h>

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

#define NODE_MAX_AGE 5
#define NODE_MAX_VELOCITY 8

// Node 的行為模式：
// 考量到 Node 移動特性，每個人對特定方向移動是有特定時間的 (age)
// Age 在等於 0 的時候會產生一個新的數值，代表這個人在走幾次 Iteration 才會改變方向
typedef struct _node {
    static int Node_Index_Incrementor;

    int index;                  // Node Index
    double curPos[2];           // 2D Map Position

    // Node 本身狀況參數
    double velocity[2];
    int age;

    _node() {
        this->index = _node::Node_Index_Incrementor++;

        // Setup Initial Position
        // 產生最大小數位數為三位的位置
        // 這裡非常有可能兩個 Node 會擠在同一個位置上
        curPos[0] = rand() % (param.map_width * 1000) / 1000.0;
        curPos[1] = rand() % (param.map_height * 1000) / 1000.0;

        // 產生一個 Age 和 Velocity
        age = rand() % NODE_MAX_AGE + 1;
        velocity[0] = rand() % (NODE_MAX_VELOCITY * 1000) / 1000.0;
        velocity[1] = rand() % (NODE_MAX_VELOCITY * 1000) / 1000.0;

        cout << "[Node::constructor]: Node " << this->index << " Created with (" << curPos[0] << ", " << curPos[1] << ", " << velocity[0] << ", " << velocity[1] << ", " << age << ")" << endl;
    }
} Node;
int Node::Node_Index_Incrementor = 1;

typedef struct _map {
    // Public Member
    Node * node_list;

    _map() {
        cout << "[Map::constructor]: Map Initialized with " << param.map_width << "X" << param.map_height << endl;
    };

    void generate_node() {
        // init node_list
        this->node_list = new Node[param.node];

        // 把資料產生出來
        cout << "[Map::generate_node]: Node List Initialized with #Node = " << param.node << " #Infectious = " << param.init_infected_node << endl;
    }

    void random_walk(int iter = 1) {
        for (int i = 0; i < iter; i++) {
            // Random Work，把每個點的位置都隨便亂移
            // [TODO]: 這裡搞不好可以優化些什麼
        }
    }

    ~_map() {
        delete [] this->node_list;
    };
} Map;

void extractParam(Parameter * param, char *argv[]);

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