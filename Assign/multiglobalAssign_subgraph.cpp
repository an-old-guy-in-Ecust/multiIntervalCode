#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <climits>
#include <assert.h>
#include <time.h>
#include <vector>
#include <queue>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <unordered_set>
#include <numeric>
#include <set>
#include <array>
#include <chrono>
#include <boost/dynamic_bitset.hpp>
#include <functional>
#include "../include/tqdm.h"
using namespace std;
chrono::duration<double> updatetime(0), hashMergeTime(0);
typedef vector<int> vec;
void readGraph(string file, vec *&G, int N)
{
    cout << "readGraph" << endl;
    ifstream hash_File(file.c_str()); // c_str()为了兼容C语言，C语言中无string，hash_file文件流读取文件file
    string currLine;
    while (getline(hash_File, currLine))
    {
        // 更新G邻居集合，第i行代表第i个节点的所有邻居节点的order
        stringstream linestream(currLine);
        string ids;
        getline(linestream, ids, ' ');
        int idx = stoi(ids);
        string ns;
        getline(linestream, ns, ' ');
        int nid = stoi(ns);
        if (idx >= N || nid >= N)
        {
            continue;
        }
        G[idx].push_back(nid);
        G[nid].push_back(idx);
    }
    for (int i = 0; i < N; i++)
    {
        sort(G[i].begin(), G[i].end());
        vec::iterator uniqEnd = unique(G[i].begin(), G[i].end());
        G[i].erase(uniqEnd, G[i].end());
    }
    cout << "done" << endl;
}

inline int binarySearch(int _key, const vec &_list, int _left, int _right)
{
    int _mid;
    while (_left < _right)
    {
        _mid = (_right + _left) / 2;
        if (_list[_mid] < _key)
            _left = _mid + 1;
        else
            _right = _mid;
    }
    return _left;
}

inline void commonNeighbor(vec &vec1, vec &vec2, int &cn, int id1, int ed1, int id2, int ed2)
{
    // 求vec1与vec2的交集cap，以及cap的个数cn
    while (id1 != ed1 && id2 != ed2)
    {
        if (vec1[id1] == vec2[id2])
        {
            cn++;
            id1++;
            id2++;
        }
        else if (vec1[id1] < vec2[id2])
            id1++;
        else
            id2++;
    }
}
// 定义pair为键时的哈希函数
template <typename T>
inline void hash_combine(std::size_t &seed, const T &val)
{
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
// auxiliary generic functions to create a hash value using a seed
template <typename T>
inline void hash_val(std::size_t &seed, const T &val)
{
    hash_combine(seed, val);
}
template <typename T, typename... Types>
inline void hash_val(std::size_t &seed, const T &val, const Types &...args)
{
    hash_combine(seed, val);
    hash_val(seed, args...);
}

template <typename... Types>
inline std::size_t hash_val(const Types &...args)
{
    std::size_t seed = 0;
    hash_val(seed, args...);
    return seed;
}

struct pair_hash
{
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2> &p) const
    {
        return hash_val(p.first, p.second);
    }
};
void updateBad(vec *&G, unordered_map<std::pair<int, int>, int, pair_hash> &turn, vec *&Bad, int *ids, int N, double epsilon, unsigned &updated, unsigned &updatedTotal, int &round, int *degrees)
{
    // 每次order结束后根据点好不好的定义更新Bad集合
    //  epsilon: lambda小于epsilon，就为好
    //  updated: 本轮有多少被更新了，如果更新的少就结束order
    cout << "update begin" << endl;
    tqdm bar;
    unsigned Badtotal = 0;
    for (int i = 0; i < N; i++)
    {
        Badtotal += Bad[i].size();
    }
    cout << "BTotal: " << Badtotal << endl;
    bar.progress(1, N);
    ofstream ratiofile;
    string ratio = "../ratio/ratio_" + to_string(round);
    ratiofile.open(ratio + ".csv");
    for (int u = 0; u < N; u++)
    {
        vec idsNeighborU;
        int s1 = degrees[u];
        if (!s1)
        {
            Bad[u].erase(Bad[u].begin(), Bad[u].end());
            continue;
        }
        for (int i = 0; i < s1; i++)
        {
            idsNeighborU.push_back(ids[G[u][i]]);
        }
        sort(idsNeighborU.begin(), idsNeighborU.end());
        for (int p = 0; p < Bad[u].size(); p++)
        {
            int v = Bad[u][p], s2 = degrees[v];
            if (!s2)
            {
                auto iter1 = remove(Bad[u].begin(), Bad[u].end(), v);
                p--;
                Bad[u].erase(iter1, Bad[u].end());
                continue;
            }
            vec idsNeighborV;
            for (int i = 0; i < s2; i++)
            {
                idsNeighborV.push_back(ids[G[v][i]]);
            }
            // intersection
            sort(idsNeighborV.begin(), idsNeighborV.end());
            vec inter;
            int p1 = 0, p2 = 0, k = 1, Min, Max;
            if (idsNeighborU[p1] < idsNeighborV[p2])
                p1 = binarySearch(idsNeighborV[p2], idsNeighborU, p1, s1);
            else if (idsNeighborU[p1] > idsNeighborV[p2])
                p2 = binarySearch(idsNeighborU[p1], idsNeighborV, p2, s2);
            if (idsNeighborU[s1 - 1] > idsNeighborV[s2 - 1])
                s1 = binarySearch(idsNeighborV[s2 - 1] + 1, idsNeighborU, p1, s1);
            else if (idsNeighborU[s1 - 1] < idsNeighborV[s2 - 1])
                s2 = binarySearch(idsNeighborU[s1 - 1] + 1, idsNeighborV, p2, s2);
            // reduced ratio指标
            k = s1 - p1 + s2 - p2 + 1;
            while (p1 < s1 && p2 < s2)
            {
                if (idsNeighborU[p1] < idsNeighborV[p2])
                {
                    p1++;
                }
                else if (idsNeighborU[p1] > idsNeighborV[p2])
                {
                    p2++;
                }
                else
                {
                    // 连续性定义法
                    // if (!inter.empty() && inter.back() < idsNeighborU[p1] - 1)
                    // {
                    //     k++;
                    // }
                    // else if (inter.empty())
                    // {
                    //     Min = idsNeighborU[p1];
                    // }

                    inter.push_back(idsNeighborU[p1]);
                    p1++;
                    p2++;
                }
            }
            double lambda;
            // 连续性指标
            // if (inter.empty())
            // {
            //     lambda = 0;
            // }
            // else
            // {
            //     Max = inter.back();
            //     double S = Max - Min + 1;
            //     lambda = k / S;
            // }
            // reduced ratio性指标
            int s = inter.size();
            lambda = (double)k / (2 * s + 1);
            ratiofile << lambda << endl;
            if (lambda <= epsilon)
            {
                // 满足好的定义，从Bad中删除(u,v)和(v,u)
                std::pair<int, int> P(u, v);
                turn[P] = round;
                auto iter1 = remove(Bad[u].begin(), Bad[u].end(), v), iter2 = remove(Bad[v].begin(), Bad[v].end(), u);
                Bad[u].erase(iter1, Bad[u].end());
                p--;
                updated++;
                if (iter2 != Bad[v].end())
                {
                    Bad[v].erase(iter2, Bad[v].end());
                    updated++;
                }
            }
        }
        bar.progress(u + 1, N);
    }
    updatedTotal += updated;
    cout << endl
         << updatedTotal << endl;
}

void updateMinMaxFast(vec *&G, vec *&Bad, int node, int h, int *&hashesMin, int *&hashesMax, unordered_map<int, unsigned> *&hashesNums, queue<int> &candidates, unordered_set<int> &candidatesUniq)
{
    for (const auto &n : G[node])
    {
        if (h < hashesMin[n])
            hashesMin[n] = h;
        if (h > hashesMax[n])
            hashesMax[n] = h;
        if (candidatesUniq.find(n) == candidatesUniq.end())
        {
            candidates.push(n);
            candidatesUniq.insert(n);
        }
    }
    for (const auto &n : G[node])
    {
        for (const auto &n2 : Bad[n])
        {
            if (h < hashesMin[n2] || h > hashesMax[n2])
            {
                unordered_map<int, unsigned> &hn2 = hashesNums[n2];
                if (hn2.find(h) != hn2.end())
                    hn2[h]++;
                else
                    hn2[h] = 1;
            }
        }
    }
}

template <typename T>
void sort_indexes(T *&v, int *&ids, int N)
{
    int *idx = new int[N];
    iota(idx, idx + N, 0); // fill idx with 0,1,2...,N
    stable_sort(idx, idx + N, [v](int i1, int i2)
                { return v[i1] < v[i2]; }); // 归并排序 v[i]=j等价于idx[j]=i

    T *newV = new T[N];
    for (int i = 0; i < N; i++)
    {
        ids[idx[i]] = i;     // ids==v
        newV[i] = v[idx[i]]; // newV = 0,1,2,3,...,N
    }
    delete[] v;
    v = newV;
}

void hashMergeDiffFast(vec *&G, vec *&Bad, int *except, int *&hashes, int *&hashesMin, int *&hashesMax, int maxDegreeNode, int N)
{
    /*
    G邻居向量
    except度小于1的点
    hashes是最终order的结果
    hashesMin是每个点rangecode的left
    hashesMax是每个点rangecode的right
    maxDegreeNode是指从哪个点开始order
    */
    unordered_map<int, unsigned> *hashesNums = new unordered_map<int, unsigned>[N](); // T(u,h): Bad(u)中有多少个点的label包含h
    boost::dynamic_bitset<> adjVec(N);                                                // 记录节点n邻居的二进制set
    boost::dynamic_bitset<> marked(N);
    marked.set(maxDegreeNode); // 第maxDegreeNode个bit设置为1，其余为0
    hashes[maxDegreeNode] = 1;
    int upper = 2;
    int lower = 0;
    long long R_count = 0;
    queue<int> candidates; // 点的邻居集合
    unordered_set<int> candidatesUniq;
    updateMinMaxFast(G, Bad, maxDegreeNode, 1, hashesMin, hashesMax, hashesNums, candidates, candidatesUniq); // 初始化maxDegreeNode的label为1。
    tqdm bar;
    bar.progress(1, N);
    int markedNum = 1;
    maxDegreeNode = 1;
    while (markedNum != N)
    {
        int n;
        if (!candidates.empty())
        {
            n = candidates.front();
            candidates.pop();
            candidatesUniq.erase(n);
            if (n == maxDegreeNode)
                maxDegreeNode++;
        }
        else
        {
            n = maxDegreeNode;
            maxDegreeNode++;
        }
        if (!marked.test(n))
        {
            if (except[n] == 0)
            {
                marked.set(n);
                markedNum++;
                bar.progress(markedNum, N);
                continue;
            }
            vec &gn = G[n];
            for (const auto &adj : gn)
                adjVec.set(adj); // adjVec 用来判断是否为节点n的邻居
            int prev = 0;
            int minus = 0;
            int hnum = upper - lower + 1;
            R_count += hnum;
            int *currNum = new int[hnum]; // P1
            fill_n(currNum, hnum, 0);
            unsigned triNum = 0; // CN
            for (const int adj : gn)
            {
                if (hashesMin[adj] != INT_MAX)
                {
                    unordered_map<int, unsigned> &hn = hashesNums[adj]; // hn->h(v,.)
                    for (int uh = lower; uh <= hashesMin[adj] - 1; uh++)
                    {
                        if (hn.find(uh) != hn.end())
                            prev += hn[uh]; // prev = p2(u, lower-1) = U(u,h)
                    }
                }
                // 计算p1
                for (const int adj2 : Bad[adj])
                {
                    if (adjVec.test(adj2))
                        triNum++;
                    else if (hashesMin[adj2] != INT_MAX)
                    {
                        currNum[hashesMin[adj2] - lower]++;
                        currNum[hashesMax[adj2] + 1 - lower]--;
                    }
                }
            }
            for (int i = 1; i < hnum; i++)
                currNum[i] = currNum[i - 1] + currNum[i]; // P1(u,h−1) + P1(u,h)
            int minScore = INT_MAX;
            int hstar = upper;
            for (int h = lower; h <= upper; h++)
            {
                int score = triNum + currNum[h - lower] - minus;
                for (const auto &adj : G[n])
                {
                    unordered_map<int, unsigned> &hn = hashesNums[adj];
                    if (hn.find(h) != hn.end())
                    {
                        if (h < hashesMin[adj])
                            minus = minus + hn[h]; // minus = L(u,h-1)
                        if (h > hashesMax[adj])
                            prev = prev + hn[h];
                    }
                }
                score = prev + score;
                if (h == lower || (h != upper && score <= minScore) || (h == upper && score < minScore))
                {
                    minScore = score;
                    hstar = h;
                }
            }
            hashes[n] = hstar;
            if (hstar == upper)
                upper++;
            else if (hstar == lower)
                lower--;
            updateMinMaxFast(G, Bad, n, hstar, hashesMin, hashesMax, hashesNums, candidates, candidatesUniq);
            delete[] currNum;
            adjVec.reset();
            marked.set(n);
            markedNum++;
            bar.progress(markedNum, N);
        }
    }
    double R = (long double)R_count / (long double)N;
    cout << "R_count = " << R_count << endl;
    cout << "R = " << R << endl;
    delete[] hashesNums;
}
// void initialBad(vec *&Bad, int N)
// {
//     vec local(N, 0);
//     iota(local.begin(), local.end(), 0);
//     for (int i = 0; i < N; i++)
//     {
//         Bad[i].insert(Bad[i].end(), local.begin(), local.end());
//         Bad[i].erase(Bad[i].begin() + i, Bad[i].begin() + i + 1); //(i,i)不属于Bad
//     }
// }
void readNEnode(string file, int *&partition, int N, char delim = '	')
{
    // 读取nodes.tsv文件
    ifstream hash_File(file.c_str());
    string currLine;
    while (getline(hash_File, currLine))
    {
        stringstream linestream(currLine);
        string ids1;
        getline(linestream, ids1, delim);
        int idx1 = stoi(ids1);
        if (idx1 >= N)
        {
            continue;
        }
        string p;
        getline(linestream, p, delim);
        int tr = stoi(p);
        partition[idx1] = tr;
    }
}
void readsubNEnode(string file, int *&partition, int N, unordered_map<int, int> &id2NEids, int part, int &index, char delim = '	')
{
    // 读取subgraph，并将原始点和子图点id一一对应
    ifstream hash_File(file.c_str());
    string currLine;
    while (getline(hash_File, currLine))
    {
        stringstream linestream(currLine);
        string ids1;
        getline(linestream, ids1, delim);
        int idx1 = stoi(ids1);
        string p;
        getline(linestream, p, delim);
        int tr = stoi(p);
        partition[idx1] = tr;
        if (tr == part)
        {
            id2NEids[idx1] = index;
            index++;
        }
    }
}
vec selectNEinitialNode(int *NEnode, int iter, int N, int maxDegreeNode, int *except)
{
    // 根据NEparition选取初始化节点
    cout << "selectnode begin" << endl;
    vec initialNode(iter + 1, 0);
    unsigned seed = time(0);
    srand(seed);
    for (int i = 0; i <= iter; i++)
    {
        int local = rand() % N;
        while (NEnode[local] != i || !except[local])
        {
            local = rand() % N;
        }
        initialNode[i] = local;
    }
    initialNode[NEnode[maxDegreeNode]] = maxDegreeNode;
    cout << "selectnode done" << endl;
    return initialNode;
}
vec selectTwoHopInitialNode(int iter, int N, int maxDegreeNode, int *except, vec *G)
{
    // 选择初始点，原则：初始点无法构成三角形
    vec result;
    vector<bool> includeNode(N, false);
    includeNode[maxDegreeNode] = true;
    cout << "1" << endl;
    int included = 1;
    for (int i = 0; i < N; i++)
    {
        if (!except[i])
        {
            includeNode[i] = true;
            included++;
        }
    }
    result.push_back(maxDegreeNode);
    unsigned int seed = time(0);
    srand(seed);
    while (result.size() <= iter)
    {
        int node = rand() % N;
        if (included == N)
        {
            result.push_back(node);
            continue;
        }
        while (includeNode[node])
        {
            node = rand() % N;
        }
        includeNode[node] = true;
        included++;
        result.push_back(node);
        for (int adj : G[node])
        {
            if (!includeNode[adj])
            {
                includeNode[adj] = true;
                included++;
            }
            for (int adjofadj : G[adj])
            {
                if (!includeNode[adjofadj])
                {
                    includeNode[adjofadj] = true;
                    included++;
                }
            }
        }
    }
    return result;
}
vec selectOneHopInitialNode(int iter, int N, int maxDegreeNode, int *except, vec *G)
{
    // 选择初始点，原则：初始点没有边相连
    vec result;
    vector<bool> includeNode(N, false);
    includeNode[maxDegreeNode] = 0;
    int included = 1;
    for (int i = 0; i < N; i++)
    {
        if (!except[i])
        {
            includeNode[i] = true;
            included++;
        }
    }
    result.push_back(maxDegreeNode);
    unsigned int seed = time(0);
    srand(seed);
    while (result.size() <= iter)
    {
        int node = rand() % N;
        if (included == N)
        {
            result.push_back(node);
            continue;
        }
        while (includeNode[node])
        {
            node = rand() % N;
        }
        includeNode[node] = true;
        included++;
        result.push_back(node);

        for (int adj : G[node])
        {
            if (!includeNode[adj])
            {
                includeNode[adj] = true;
                included++;
            }
        }
    }
    return result;
}
void makesubgraph(int *NE, vec *&G, int part, int N, string filename, unordered_map<int, int> id2NEid)
{
    ofstream subgraph;
    subgraph.open(filename);
    for (int i = 0; i < N; i++)
    {
        for (int j : G[i])
        {
            if (NE[i] == part && NE[j] == part)
            {
                subgraph << id2NEid[i] << " " << id2NEid[j] << endl;
            }
        }
    }
}
int main(int argc, char **argv)
{
    string directname = "../data/";
    string outdirect = "../hash_data/";
    string name = argv[1]; // 第一个参数 文件名（datafield）
    outdirect += name + "/";
    int N = stoi(argv[2]);  // 第二个参数 顶点数目
    bool R = stoi(argv[3]); // 第三个参数 0 or 1 0是range，1是two level
    string filename = directname + name + ".txt";
    cout << filename << endl;
    vec *OG = new vec[N];
    // G[i][j]表示顶点i对应的邻接顶点j
    readGraph(filename, OG, N);
    int round = 0, iter = stoi(argv[5]);
    unsigned updated = 0, updatedTotal = 0; // 每轮更新了多少点对
    double epsilon = stod(argv[4]);
    int part = stoi(argv[6]); // NE分割后第几块当作子图

    vec *Bad = new vec[N];
    // 构建子图
    string NEnodefilename = directname + name + ".txt" + ".edgepart." + to_string(iter + 1) + ".nodes.tsv";
    int *NE = new int[N];
    unordered_map<int, int> id2NEid;
    int index = 0;
    readsubNEnode(NEnodefilename, NE, N, id2NEid, part, index, '	');
    cout << "readNEnode finish" << endl;
    makesubgraph(NE, OG, part, N, directname + name + "_subgraph.txt", id2NEid);
    cout << "make subgraph done" << endl;
    // readNEnode(NEnodefilename, NE, N, '	');
    delete[] OG;
    cout << "the node of subgraph: " << index << endl;
    N = index;
    vec *G = new vec[N];
    filename = directname + name + "_subgraph.txt";
    readGraph(filename, G, N);
    unordered_map<std::pair<int, int>, int, pair_hash> turn; // 记录边在第几轮变成Good
    // cout << NEnodefilename << endl;
    for (int i = 0; i < N; i++)
    {
        *(Bad + i) = *(G + i);
    }
    // initialBad(Bad, N);
    int *degrees = new int[N];
    fill_n(degrees, N, 0);
    for (int i = 0; i < N; i++)
    {
        if (G[i].size() == 1)
        {
            degrees[G[i][0]] = degrees[G[i][0]] - 1;
            degrees[i] = 0;
        }
        else
            degrees[i] = degrees[i] + G[i].size();
    }
    int *except = new int[N]; // 顶点的度是否小于等于1，小于等于1就不考虑
    for (int i = 0; i < N; i++)
    {
        if (degrees[i] <= 1)
            except[i] = 0;
        else
        {
            int CN = 0;
            for (int j = 0; j != G[i].size(); j++)
            {
                int n = G[i][j];
                int id2 = binarySearch(n, G[n], 0, G[n].size());
                commonNeighbor(G[i], G[n], CN, j + 1, G[i].size(), id2, G[n].size());
            }
            except[i] = CN; // CN是i点和其所有邻居的公共点数目总和
        }
    }
    // 从最大度节点开始coding
    int maxDegreeNode = 0;
    int maxDegree = 0;
    for (int i = 0; i < N; i++)
    {
        if (G[i].size() > maxDegree)
        {
            maxDegree = G[i].size();
            maxDegreeNode = i;
        }
    }
    // vec initialNode = selectNEinitialNode(NE, iter, N, maxDegreeNode, except);
    vec initialNode = selectTwoHopInitialNode(iter, N, maxDegreeNode, except, G);
    cout << "end" << endl;
    // vec includeNode(N, 0);
    // // includeNode[i]=1表示不能作为初始点
    // for (int i : G[maxDegreeNode])
    // {
    //     includeNode[i] = 1;
    // }
    // includeNode[maxDegreeNode] = 1;

    delete[] NE;
    while ((updated || !round) && round <= iter)
    {
        round++;
        cout << "round " << round << " begin" << endl;
        updated = 0;
        // try{int *h=new int[N];}
        // catch(const bad_alloc& e)
        // {
        //     cout<<"Bad allocation"<<endl;
        // }
        int *hashes = new int[N];
        fill_n(hashes, N, INT_MAX);
        // unsigned seed = time(0);
        // srand(seed);
        // if (round != 1)
        // {
        //     while (includeNode[maxDegreeNode])
        //         maxDegreeNode = rand() % N;
        // }
        // for (int i : G[maxDegreeNode])
        // {
        //     includeNode[i] = 1;
        // }
        maxDegreeNode = initialNode[round - 1];
        if (R)
        {
            int *hashesMin = new int[N];
            int *hashesMax = new int[N];
            fill_n(hashesMin, N, INT_MAX);
            fill_n(hashesMax, N, INT_MIN);
            auto start_time = chrono::high_resolution_clock::now();
            hashMergeDiffFast(G, Bad, except, hashes, hashesMin, hashesMax, maxDegreeNode, N);
            auto end_time = chrono::high_resolution_clock::now();
            chrono::duration<double> diff = end_time - start_time;
            hashMergeTime += diff;
            cout << "MergeTime: " << diff.count() << " s" << endl;
            delete[] hashesMin;
            delete[] hashesMax;
        }
        else
        {
            auto start_time = chrono::high_resolution_clock::now();
            // hashMergeDiffIndexNBEx(G, except, hashes, maxDegreeNode, N);
            auto end_time = chrono::high_resolution_clock::now();
            chrono::duration<double> diff = end_time - start_time;
            cout << diff.count() << " s" << endl;
        }
        int *ids = new int[N];
        sort_indexes(hashes, ids, N);
        auto start_time = chrono::high_resolution_clock::now();
        updateBad(G, turn, Bad, ids, N, epsilon, updated, updatedTotal, round, degrees);
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> diff = end_time - start_time;
        updatetime += diff;
        cout << "UpdateTime: " << diff.count() << " s" << endl;
        cout << "updated done" << endl;
        string outname = name;
        if (R)
            outname += "_range_nb";
        else
            outname += "_index_nb";
        ofstream idfile;
        string b = outdirect + outname;
        idfile.open(b + "/" + outname + "_id_" + to_string(round) + ".csv");
        idfile << ":ID" << endl;
        for (int i = 0; i < N; i++)
            idfile << ids[i] << endl;
        cout << "id done" << endl;
        ofstream nodefile;
        nodefile.open(b + "/" + outname + "_node_" + to_string(round) + ".csv");
        nodefile << ":ID,h:int" << endl;
        cout << "node done" << endl;
        for (int i = 0; i < N; i++)
            nodefile << i << "," << hashes[i] << endl;
        delete[] ids;
        delete[] hashes;
    }
    int remain = 0;
    ofstream BadRemainfile, turnfile;
    string outname = name;
    if (R)
    {
        outname += "_range_nb";
    }
    else
    {
        outname += "_id_nb";
    }
    string b = outdirect + outname;
    BadRemainfile.open(b + "/" + outname + "_" + argv[4] + "_BadRemain.csv");
    for (int i = 0; i < N; i++)
    {
        for (auto j : Bad[i])
        {
            std::pair<int, int> p(i, j);
            BadRemainfile << i << "," << j << endl;
            turn[p] = 1;
            remain++;
        }
    }
    cout << "remain:" << remain << endl;
    cout << "Mergetotaltime: " << hashMergeTime.count() << 's' << endl;
    cout << "Updatetotaltime: " << updatetime.count() << 's' << endl;
    ofstream reporttime;
    reporttime.open(b + "/" + outname + "_" + argv[4] + "_result.txt");
    reporttime << "remain:" << remain << endl;
    reporttime << "Mergetotaltime: " << hashMergeTime.count() << 's' << endl;
    reporttime << "Updatetotaltime: " << updatetime.count() << 's' << endl;
    turnfile.open(b + "/" + outname + "_Good.csv");
    turnfile << "pair,turn_round" << endl;
    for (int i = 0; i < N; i++)
    {
        int s = G[i].size();
        for (int n = 0; n < s; n++)
        {
            int j = G[i][n];
            if (!degrees[j])
            {
                continue;
            }
            std::pair<int, int> p(i, j), q(j, i);
            int k = max(turn[p], turn[q]);
            turnfile << i << "," << j
                     << "," << k << endl;
        }
    }
    cout << "done" << endl;
    delete[] G;
    delete[] Bad;
}
