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
#include "../include/tqdm.h"

using namespace std;

typedef vector<int> vec;

void readGraph(string file, vec *&G, int N)
{
    ifstream hash_File(file.c_str()); // c_str()为了兼容C语言，C语言中无string，hash_file文件流读取文件file
    string currLine;
    while (getline(hash_File, currLine))
    {
        //更新G邻居集合，第i行代表第i个节点的所有邻居节点的order
        stringstream linestream(currLine);
        string ids;
        getline(linestream, ids, ' ');
        int idx = stoi(ids);
        string ns;
        getline(linestream, ns, ' ');
        int nid = stoi(ns);
        G[idx].push_back(nid);
        G[nid].push_back(idx);
    }
    for (int i = 0; i < N; i++)
    {
        sort(G[i].begin(), G[i].end());
        vec::iterator uniqEnd = unique(G[i].begin(), G[i].end());
        G[i].erase(uniqEnd, G[i].end());
    }
}

template <typename T>
void sort_indexes(T *&v, int *&ids, int N)
{
    int *idx = new int[N];
    iota(idx, idx + N, 0); // fill idx with 0,1,2...,N
    stable_sort(idx, idx + N, [v](int i1, int i2)
                { return v[i1] < v[i2]; }); //归并排序

    T *newV = new T[N];
    for (int i = 0; i < N; i++)
    {
        ids[idx[i]] = i;
        newV[i] = v[idx[i]];
    }
    delete[] v;
    v = newV;
}
unsigned countContain(int h, int *&hashesMin, int *&hashesMax, vec &adjs)
{
    unsigned result = 0;
    for (const int n : adjs)
    {
        if (hashesMin[n] > h || h > hashesMax[n])
            result++;
    }
    return result;
}

//二分查找
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

void updateMinMaxFast(vec &neighbors, int h, int *&hashesMin, int *&hashesMax, unordered_map<int, unsigned> &hashesNums,
                      unordered_map<int, unsigned> &hashesContainNums, int upper, queue<int> &candidates, unordered_set<int> &candidatesUniq)
{
    for (int gh = h; gh <= upper; gh++)
        hashesContainNums[gh] = hashesContainNums[gh] + neighbors.size();
    for (const int n : neighbors)
    {
        int hm = hashesMin[n];
        int hM = hashesMax[n];
        if (hm == INT_MAX)
        {
            hashesMin[n] = h;
            hashesMax[n] = h;
            hashesNums[h]++;
        }
        else if (h < hm)
        {
            hashesMin[n] = h;
            hashesNums[h]++;
            for (int uh = h; uh < hm; uh++)
                hashesNums[uh]++;
        }
        else if (h > hM)
        {
            hashesMax[n] = h;
            hashesNums[h] = hashesNums[h] + 1;
            for (int uh = hM + 1; uh <= h; uh++)
                hashesNums[uh]++;
        }
        if (candidatesUniq.find(n) == candidatesUniq.end())
        {
            candidates.push(n);
            candidatesUniq.insert(n);
        }
    }
}

void hashMergeDiffFast(vec *&G, int *except, int *&hashes, int *&hashesMin, int *&hashesMax, int maxDegreeNode, int N)
{
    /*
    G邻居向量
    except度小于1的点
    hashes是最终order的结果
    hashesMin是每个点rangecode的left
    hashesMax是每个点rangecode的right
    maxDegreeNode是指从哪个点开始order
    */
    unordered_map<int, unsigned> hashesNums;        // T_1: 有多少点的RangeCode包含h
    unordered_map<int, unsigned> hashesContainNums; // T_2
    boost::dynamic_bitset<> marked(N);
    marked.set(maxDegreeNode); //第maxDegreeNode个bit设置为1，其余为0
    hashes[maxDegreeNode] = 1;
    hashesNums[1] = 0;
    hashesContainNums[1] = 0;
    int upper = 2;
    hashesNums[2] = 0;
    hashesContainNums[2] = hashesContainNums[1];
    int lower = 0;
    hashesNums[0] = 0;
    hashesContainNums[0] = 0;
    queue<int> candidates; //点的邻居集合
    unordered_set<int> candidatesUniq;
    updateMinMaxFast(G[maxDegreeNode], 1, hashesMin, hashesMax, hashesNums, hashesContainNums, upper, candidates, candidatesUniq); //初始化maxDegreeNode的label为1。
    tqdm bar;
    bar.progress(1, N);
    int markedNum = 1;
    maxDegreeNode = N - 2;
    while (markedNum != N)
    {
        int n;
        if (!candidates.empty())
        {
            n = candidates.front();
            candidates.pop();
            candidatesUniq.erase(n);
            if (n == maxDegreeNode)
                maxDegreeNode--;
        }
        else
            n = maxDegreeNode--;
        if (!marked.test(n))
        {
            if (except[n] == 1) //过滤度为1的节点
            {
                marked.set(n);
                markedNum++;
                bar.progress(markedNum, N);
                continue;
            }
            int minScore = INT_MAX;
            int hstar = upper;
            vec &gn = G[n];
            int gns = G[n].size();
            for (int h = lower; h <= upper; h++)
            {
                unsigned score = (hashesNums[h] + countContain(h, hashesMin, hashesMax, gn) - 1) * gns; // Part 1
                // Part 2计算
                for (const int adj : gn)
                {
                    int m1 = hashesMin[adj];
                    int M1 = hashesMax[adj];
                    if (m1 == INT_MAX)
                    {
                        if (h != lower)
                            score += hashesContainNums[h] - hashesContainNums[h - 1];
                    }
                    else if (h < m1)
                    {
                        if (h == lower)
                            score += hashesContainNums[m1 - 1];
                        else
                            score += hashesContainNums[m1 - 1] - hashesContainNums[h - 1];
                    }
                    else if (h > M1)
                        score += hashesContainNums[h] - hashesContainNums[M1];
                }
                if (score < minScore)
                {
                    minScore = score;
                    hstar = h;
                }
            }
            hashes[n] = hstar;
            cout<<hstar<<endl;
            if (hstar == upper)
            {
                upper++;
                hashesNums[upper] = 0;
                hashesContainNums[upper] = hashesContainNums[upper - 1];
            }
            else if (hstar == lower)
            {
                lower--;
                hashesNums[lower] = 0;
                hashesContainNums[lower] = 0;
            }
            updateMinMaxFast(G[n], hstar, hashesMin, hashesMax, hashesNums, hashesContainNums, upper, candidates, candidatesUniq);
            marked.set(n);
            markedNum++;
            bar.progress(markedNum, N);
        }
    }
}

inline void updateMinMaxIndex(vec &neighbors, int h, int upper, unordered_map<int, array<unsigned, 3>> &hashesNums, queue<int> &candidates,
                              unordered_set<int> &candidatesUniq, unordered_set<int> *&hashEntries, int *hashesMin, int *hashesMax)
{
    // hashEntries[n]表示有哪些点的order是n
    // hashNums：
    int dp = 0;
    for (const int &n : neighbors)
    {
        if (hashEntries[n].find(h) == hashEntries[n].end())
        {
            hashEntries[n].insert(h);
            dp++;
        }
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
    array<unsigned, 3> &hnh = hashesNums[h];
    hnh[0] += dp;
    hnh[2] += neighbors.size();
    for (int uh = h; uh <= upper; uh++)
        hnh[1] += dp;
}

void hashMergeDiffIndex(vec *&G, int *except, int *&hashes, int maxDegreeNode, int N)
{
    int *hashesMin = new int[N];
    int *hashesMax = new int[N];
    fill_n(hashesMin, N, INT_MAX);
    fill_n(hashesMax, N, INT_MIN);
    unordered_map<int, array<unsigned, 3>> hashesNums;
    unordered_set<int> *hashEntries = new unordered_set<int>[N];
    boost::dynamic_bitset<> marked(N);
    marked.set(maxDegreeNode);
    hashes[maxDegreeNode] = 1;
    hashesNums[1] = {0, 0, 0};
    int upper = 2;
    hashesNums[2] = {0, 0, 0};
    int lower = 0;
    hashesNums[0] = {0, 0, 0};
    queue<int> candidates;
    unordered_set<int> candidatesUniq;
    updateMinMaxIndex(G[maxDegreeNode], 1, upper, hashesNums, candidates, candidatesUniq, hashEntries, hashesMin, hashesMax);
    tqdm bar;
    bar.progress(1, N);
    int markedNum = 1;
    maxDegreeNode = N - 2;
    while (markedNum != N)
    {
        int n;
        if (!candidates.empty())
        {
            n = candidates.front();
            candidates.pop();
            candidatesUniq.erase(n);
            if (n == maxDegreeNode)
                maxDegreeNode--;
        }
        else
            n = maxDegreeNode--;
        if (!marked.test(n))
        {
            if (except[n] == 1)
            {
                marked.set(n);
                markedNum++;
                bar.progress(markedNum, N);
                continue;
            }
            unsigned minScore = INT_MAX;
            int hstar = upper;
            for (int h = lower; h <= upper; h++)
            {
                vec &gn = G[n];
                unsigned score = 0;
                int d = 0;
                for (const auto &adj : gn)
                {
                    if (hashEntries[adj].find(h) == hashEntries[adj].end())
                        d++;
                }
                for (const auto &adj : gn)
                {
                    if (hashEntries[adj].find(h) != hashEntries[adj].end())
                        score += hashesNums[h][0] + d;
                    else
                    {
                        int m1 = hashesMin[adj];
                        int M1 = hashesMax[adj];
                        unsigned m;
                        unsigned M;
                        if (m1 == INT_MAX || (h >= m1 && h <= M1))
                        {
                            if (h == lower)
                                m = 0;
                            else
                                m = hashesNums[h - 1][1];
                            M = hashesNums[h][1];
                        }
                        else if (h < m1)
                        {
                            if (h == lower)
                                m = 0;
                            else
                                m = hashesNums[h - 1][1];
                            M = hashesNums[m1 - 1][1];
                        }
                        else if (h > M1)
                        {
                            m = hashesNums[M1][1];
                            M = hashesNums[h][1];
                        }
                        score += (M - m + d - 1) + (hashesNums[h][2] + d - 1);
                    }
                }
                if (score <= minScore)
                {
                    minScore = score;
                    hstar = h;
                }
            }
            hashes[n] = hstar;
            if (hstar == upper)
            {
                unsigned prev = hashesNums[upper][1];
                upper++;
                hashesNums[upper] = {0, prev, 0};
            }
            else if (hstar == lower)
            {
                lower--;
                hashesNums[lower] = {0, 0, 0};
            }
            updateMinMaxIndex(G[n], hstar, upper, hashesNums, candidates, candidatesUniq, hashEntries, hashesMin, hashesMax);
            marked.set(n);
            markedNum++;
            bar.progress(markedNum, N);
        }
    }
    delete[] hashesMin;
    delete[] hashesMax;
    delete[] hashEntries;
}

inline int binarySearch(int _key, const vec &_list, int *&_hashes)
{
    int _left = 0;
    int _right = _list.size();
    int _mid;
    while (_left < _right)
    {
        _mid = (_right + _left) / 2;
        if (_hashes[_list[_mid]] < _key)
            _left = _mid + 1;
        else
            _right = _mid;
    }
    return _left;
}

int main(int argc, char **argv)
{
    string directname = "../data/";
    string outdirect = "../hash_data/global0/";
    string name = argv[1];  //数据集名字
    int N = stoi(argv[2]);  //顶点个数
    bool R = stoi(argv[3]); // 是range还是two-level
    string filename = directname + name + ".txt";
    vec *G = new vec[N];
    readGraph(filename, G, N);  //初始化图G的order
    int *hashes = new int[N];   //最终结果
    fill_n(hashes, N, INT_MAX); //初始化hashes向量
    int *degrees = new int[N];
    fill_n(degrees, N, 0); //初始化degrees度向量
    //相当于删除了度为1的节点
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
    int *except = new int[N]; //顶点的度是否小于等于1，小于等于1就不考虑
    for (int i = 0; i < N; i++)
    {
        if (degrees[i] <= 1)
            except[i] = 1;
    }
    if (R)
    {
        int *hashesMin = new int[N]; //每个点的rangecode的left
        int *hashesMax = new int[N]; //每个点的rangecode的right
        fill_n(hashesMin, N, INT_MAX);
        fill_n(hashesMax, N, INT_MIN);
        auto start_time = chrono::high_resolution_clock::now();
        hashMergeDiffFast(G, except, hashes, hashesMin, hashesMax, N - 1, N);
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> diff = end_time - start_time;
        cout << diff.count() << " s" << endl;
        delete[] hashesMin;
        delete[] hashesMax;
    }
    else
    {
        auto start_time = chrono::high_resolution_clock::now();
        hashMergeDiffIndex(G, except, hashes, N - 1, N);
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> diff = end_time - start_time;
        cout << diff.count() << " s" << endl;
    }
    int *ids = new int[N];
    sort_indexes(hashes, ids, N);
    string outname = name;
    if (R)
        outname += "_range";
    else
        outname += "_index";
    ofstream idfile;
    idfile.open(outdirect + outname + "_id.csv");
    string a = outdirect + outname + "_id.csv";
    cout << a << endl;
    idfile << ":ID" << endl;
    for (int i = 0; i < N; i++)
        idfile << ids[i] << endl;
    ofstream nodefile;
    nodefile.open(outdirect + outname + "_node.csv");
    nodefile << ":ID,h:int" << endl;
    for (int i = 0; i < N; i++)
        nodefile << i << "," << hashes[i] << endl;

    delete[] G;
    delete[] degrees;
    delete[] except;
    delete[] hashes;
    delete[] ids;
}