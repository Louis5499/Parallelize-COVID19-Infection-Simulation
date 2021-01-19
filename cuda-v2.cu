#include <iostream>
#include <iomanip>
#include <cassert>
#include <curand.h>
#include <omp.h>
#include <cmath>
#include <time.h>

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

// State Definition
#define NODE_STATE_SUSCEPTIBLE 0
#define NODE_STATE_INFECTIOUS 1
#define NODE_STATE_RECOVERED 2
#define NODE_STATE_DEAD 3

// Definition
#define NODE_MAX_VELOCITY 4

// Constant Definition
__constant__ float DeadProbability[22] = {0.0, 0.001, 0.002, 0.004, 0.008, 0.014, 0.022, 0.032, 0.022, 0.014, 0.008, 0.004, 0.002, 0.001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
__constant__ float RecoveryRate[22] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.31, 0.43, 0.57, 0.73, 0.91, 1.0};

// Kernel
__global__ void computeDist(float * ddist, float * dmapX, float * dmapY, int nodeCount, float alpha, float beta) {
    // 用輪盤式的 Access 看會不會好一點
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= nodeCount || y >= nodeCount) return;

    // 計算 Dist
    float dist = sqrtf(powf(dmapX[x] - dmapX[y], 2) + powf(dmapY[x] - dmapY[y], 2));
    
    // Calculate Infactious Rate
    ddist[x*nodeCount+y] = alpha * expf(-1 * dist * beta);
}

// Probability 算數
inline double infectionRate(double dist) {
    // 如果拿 alpha = 0.99(最大值) beta = 2.5 distance = 3 --> 接近 0
    return param.alpha_constant * exp(-1 * dist * param.beta_constant);
}

double calculateTime(timespec * start, timespec * end) {
  timespec temp;
  if ((end->tv_nsec - start->tv_nsec) < 0) {
       temp.tv_sec = end->tv_sec-start->tv_sec-1;
       temp.tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
   } else {
       temp.tv_sec = end->tv_sec - start->tv_sec;
       temp.tv_nsec = end->tv_nsec - start->tv_nsec;
   }
   return temp.tv_sec + (double) temp.tv_nsec / 1000000000.0;
}

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

    timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    // 這邊考量到 Coleased Memory Access 的問題，把 X, Y 的位置分開儲存
    // 這裏可以寫兩個版本，一個是由 CPU 同時處理 Rand 的計算，另一是使用 curand
    // 把資料直接產生在 GPU 上
    float * mapX = reinterpret_cast<float *>(new float[param.node]);
    float * mapY = reinterpret_cast<float *>(new float[param.node]);
    float * dmapX, * dmapY;
    cudaMalloc(&dmapX, param.node * sizeof(float));
    cudaMalloc(&dmapY, param.node * sizeof(float));
    
    // State
    // dState 是放在 GPU 上的，理論上並不需要傳回來，但為了把資料輸出視覺化呈現
    // 可以在每一輪結束時把資料 Transfer 回來
    // unsigned short * dState;
    short * state = reinterpret_cast<short *>(new short[param.node]);
    short * nextState = reinterpret_cast<short *>(new short[param.node]);
    for (int i = 0; i < param.node; i++) state[i] = nextState[i] = NODE_STATE_SUSCEPTIBLE;

    int tmp, counter = param.init_infected_node;
    while (counter > 0) {
        tmp = rand() % param.node;
        if (state[tmp] != NODE_STATE_INFECTIOUS) {
            state[tmp] = nextState[tmp] = NODE_STATE_INFECTIOUS;
            counter--;
            // cout << "[Map::generate_node]: Node " << node_list[tmp].index << " Set Infected." << endl;
        }
    }

    // 我覺得 GPU 單純拿來計算 Probability 就好，同一時間 CPU 負責計算 Move 的結果
    float * prob = reinterpret_cast<float *>(new float[param.node]);

    // Initial Rand Position
    // 這裡試過了，千萬不要放 Openmp 的 code 來跑，效果有夠慢
    for (int i = 0; i < param.node; i++) {
        mapX[i] = rand() % (param.map_width * 1000) / 1000.0;
        mapY[i] = rand() % (param.map_height * 1000) / 1000.0;
    }

    // Create Distance Matrix
    float * dist = reinterpret_cast<float *>(new float[param.node * param.node]);
    float * ddist;
    cudaMalloc(&ddist, param.node * param.node * sizeof(float));

    // Algorithm Main Part
    // GPU 每一有以下幾個 Step
    // 1. GPU Calculate Distance
    // 2. Calculate Probability
    // CPU 有以下幾個工作
    // 1. MOVE
    // 2. Change State
    const dim3 blocks(ceil(param.node/32.0), ceil(param.node/32.0));
    const dim3 threadBlock(32, 32);
    float moving[2];
    for (int iter = 0; iter < param.iter; iter++) {
        // 1. Async 搬移資料到 GPU 去
        // [TODO]: 不知道為什麼什麼都沒有過去
        cudaMemcpyAsync(dmapX, mapX, param.node*sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpyAsync(dmapY, mapY, param.node*sizeof(float), cudaMemcpyHostToDevice);

        // 2. Submit Kernel Function
        computeDist<<< blocks, threadBlock >>>(ddist, dmapX, dmapY, param.node, param.alpha_constant, param.beta_constant);

        // 3. CPU 同時計算 Move
        // [TODO] 這邊可以 Parallel 化
        for (int i = 0; i < param.node; i++) {
            moving[0] = mapX[i] += rand() % (NODE_MAX_VELOCITY * 2 * 1000) / 1000.0 - NODE_MAX_VELOCITY;
            moving[1] = mapY[i] += rand() % (NODE_MAX_VELOCITY * 2 * 1000) / 1000.0 - NODE_MAX_VELOCITY;
            if (moving[0] >= 0 && moving[0] <= param.map_width) mapX[i] = moving[0];
            if (moving[1] >= 0 && moving[1] <= param.map_height) mapY[i] = moving[1];
            state[i] = nextState[i];
        }

        // 4. 把資料搬回來
        cudaMemcpy(dist, ddist, param.node * param.node *sizeof(float), cudaMemcpyDeviceToHost);

        // 從 Dist 拿回來後，開始計算
        // [TODO]: 這邊可以用 openmp 來處理
#pragma omp parallel num_threads(16)
{
        #pragma omp for
        for (int n = 0; n < param.node; n++) {
            if (state[n] == NODE_STATE_RECOVERED || state[n] == NODE_STATE_DEAD) continue;
            float prob = 1;
            for (int k = 0; k < param.node; k++) {
                if (n == k || state[k] != NODE_STATE_INFECTIOUS) continue;

                if (dist[n*param.node + k] >= 0.1) prob *= dist[n*param.node + k];
            }
            if (prob == 1) prob = 0;

            // 5. State 計算
            if (prob >= 0.1) nextState[n] = NODE_STATE_INFECTIOUS;
        }
}
        cout << "Iteration: " << iter << " has completed." << endl;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    std::cout << "[0]Output Time: " << calculateTime(&start, &end) << "(sec)" << std::endl;
    // Free Cuda Resource
    cudaFree(dmapX);
    cudaFree(dmapY);
    cudaFree(ddist);

    // Free CPU Resources
    delete [] mapX;
    delete [] mapY;
    delete [] state;
    delete [] prob;
    delete [] dist;

    return 0;
}