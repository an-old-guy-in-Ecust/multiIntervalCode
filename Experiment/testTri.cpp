#include "indexGraph.h"
std::string get_FileBaseName(std::string path)
{
    std::string name;
    for (int i = path.size() - 1; i > 0; i--)
    {
        if (path[i] == '\\' || path[i] == '/')
        {
            name = path.substr(i + 1);
            return name;
        }
    }
    name = path;
    return name;
}
unsigned long long minimal = 0, maximal = 0;
void reportRatio(vec *G, unordered_map<std::pair<int, int>, int, utils::pair_hash> &turn, hashGraph **multiHG, int N)
{
    float ratio = 0;
    for (int i = 0; i != N; i++)
    {
        vec adj = G[i];
        for (int j : adj)
        {
            std::pair<int, int> p(i, j);
            int round = turn[p];
            if (!round)
            {
                round = 1;
            }
            // cout << "round:" << round << endl;
            hashGraph *current = multiHG[round - 1];
            // cout << "i,j:" << i << "," << j << endl;
            int newi = current->getId(i), newj = current->getId(j);
            // cout << "new i,new j:" << newi << "," << newj << endl;
            current->reportRatio(newi, newj, minimal, maximal);
            // cout << "result:" << result << endl;
        }
    }
    ratio = (float)minimal / maximal;
    cout << "multi ratio:" << ratio << endl;
}

double multicalTri(vec *G, unordered_map<std::pair<int, int>, int, utils::pair_hash> &turn, hashGraph **multiHG, bool reportNumber, int N, int *degrees)
{
    auto start_time = chrono::high_resolution_clock::now();
    cout << "start:" << endl;
    unsigned long result = 0;
    double d = 0.0;
    for (int i = 0; i != N; i++)
    {
        vec adj = G[i];
        if (!degrees[i])
        {
            continue;
        }
        for (int j : G[i])
        {
            // cout << "j:" << j << endl;
            if (!degrees[j])
            {
                continue;
            }
            // cout << "n:" << n << endl;
            auto start_time1 = chrono::high_resolution_clock::now();
            std::pair<int, int> p(i, j);
            int round = turn[p];
            hashGraph *current = multiHG[round - 1];
            int newi = current->getId(i), newj = current->getId(j);
            auto end_time1 = chrono::high_resolution_clock::now();
            chrono::duration<double> di = end_time1 - start_time1;
            d += di.count();
            // cout << "new i,new j:" << newi << "," << newj << endl;
            current->CN(newi, newj, result);
            // cout << "result:" << result << endl;
        }
    }
    auto end_time = chrono::high_resolution_clock::now();
    cout << "id转化时间:" << d << 's' << endl;
    if (reportNumber)
        cout << endl
             << "Total Triangles:" << result << endl;
    chrono::duration<double> diff = end_time - start_time;
    return diff.count() - d;
}
void viewRemain(string file, hashGraph *HG, string lambda, string filename)
{
    ifstream BadRemain(file.c_str());
    ofstream BadIntersect;
    BadIntersect.open(filename + lambda + "_BadIntersect.txt", ios::app);
    string currLine;
    BadIntersect << "Intersection:";
    cout << file << endl;
    while (getline(BadRemain, currLine))
    {
        stringstream linestream(currLine);
        string ids;
        getline(linestream, ids, ',');
        int idx = stoi(ids);
        idx = HG->getId(idx);
        string ns;
        getline(linestream, ns, ',');
        int nid = stoi(ns);
        nid = HG->getId(nid);
        unsigned long result = 0;
        HG->CN(idx, nid, result);
        BadIntersect << result << ",";
    }
}
void ReadRemain(int u, int v, hashGraph *HG, string lambda, string filename)
{
    int newu = HG->getId(u), newv = HG->getId(v);
    unsigned long result = 0;
    ofstream BadIntersect;
    BadIntersect.open(filename + lambda + "_BadIntersect.txt", ios::app);
    HG->CN(newu, newv, result);
    BadIntersect << "Intersection:" << result << endl;
    BadIntersect << newu << ":" << endl;
    for (int j : HG->neighbor(newu))
    {
        BadIntersect << j << ',';
    }
    BadIntersect << endl
                 << "degree1:" << HG->getdegree(newu) << endl
                 << newv << ":" << endl;
    for (int j : HG->neighbor(newv))
    {
        BadIntersect << j << ',';
    }
    BadIntersect << "degree2:" << HG->getdegree(newv) << endl;
    BadIntersect << endl;
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
int main(int argc, char **argv)
{
    string directname = "../data/";
    string outdirect = "../hash_data/";
    string groDirect = "../gro_data/";
    string name = argv[1];
    int N = stoi(argv[2]);
    int test = stoi(argv[3]);
    int times = stoi(argv[4]);
    vec *G = new vec[N];
    utils::readGraph(directname + name + "_subgraph.txt", G, N);
    cout << "read done";
    Graph OG(G, N);
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
    double OGT = 0;
    for (int i = 0; i < times; i++)
    {
        OGT += OG.calTri();
        cout << "\r";
    }
    double origtime = 0, rangetime = 0, indextime = 0, multirangetime = 0;
    // cout << "Origin Time: " << OGT << endl;
    if (test == -1)
    {
        cout << "Original Time: " << OGT / times << endl;
        cout << "Method\tRate\tAvgTime" << endl;
        stringvec groFiles;
        utils::read_directory(groDirect, groFiles);
        for (const string &f : groFiles)
        {
            int idx = f.find(name);       // 在aa中查找bb.
            if (idx != std::string::npos) // 不存在。
            {
                hashGraph HG(G, N, f);
                double GROT = 0;
                for (int i = 0; i < times; i++)
                {
                    GROT += HG.calTri();
                    cout << "\r";
                }
                cout << get_FileBaseName(f) << "\t" << OGT / GROT << "\t" << GROT / times << endl;
            }
        }
    }
    if (test == -1)
    {
        delete[] G;
        return 0;
    }
    else if (test == 1)
    {
        name = "test_" + name;
        outdirect = "../maxnode_hash_data/";
    }
    else if (test == 2)
    {
        outdirect = "../degree_hash_data/0/";
    }
    else if (test == 3)
    {
        outdirect += "d1select_d2limit/adjust/";
    }
    else if (test == 4)
    {
        outdirect = "../bfsdegree_hash_data/0/";
    }
    else if (test == 5)
    {
        outdirect = "../bfsdegree_hash_data/1/";
    }
    else if (test == 6)
    {
        outdirect += "d1select_d2limit/adjust/";
        outdirect += name;
        outdirect += "/";
        outdirect += argv[5];
        outdirect += "_";
        outdirect += argv[6];
    }
    else if (test == 7)
    {
        outdirect += "d1select_d2limit_Rlimit_diff_part_time/";
        outdirect += argv[5];
        outdirect += "_";
        outdirect += argv[6];
        outdirect += '_';
        outdirect += argv[7];
    }
    {
        cout << "originrange start:" << endl;
        int *hashes = new int[N];
        iota(hashes, hashes + N, 0);
        hashGraph HG(G, hashes, N);
        for (int i = 0; i < times; i++)
        {
            origtime += HG.calTri();
            cout << "\r";
        }
        HG.reportRatio();
        // cout << "Orig Order Rate: " << OGT / HGT << endl;
        // cout << "Orig Order Time: " << HGT << endl;
        delete[] hashes;
    }
    if (test != 7)
    {
        cout << endl
             << "test:" << test << endl
             << endl;
        std::string hdirect = outdirect + name + "/" + name + "_range_nb";
        hdirect += "/" + name + "_range_nb_";
        std::string hnode = hdirect + "node.csv";
        std::string hid = hdirect + "id.csv";
        // cout << "hnode = " << hnode << endl;
        // cout << hid << endl;
        hashGraph HG(G, N, hnode, hid);
        cout << "range start" << endl;
        for (int i = 0; i < times; i++)
        {
            rangetime += HG.calTri();
            cout << "\r";
        }
        cout << "range";
        HG.reportRatio();
        // cout << "HG Rate: " << OGT / HGT << endl;
        // cout << "Range Time: " << HGT << endl;
    }
    else
    {
        rangetime = 1;
    }
    {
        // std::string idirect = outdirect + name + "/" + name + "_index_nb";
        // idirect += "/" + name + "_index_nb_";
        // std::string inode = idirect + "node.csv";
        // std::string iid = idirect + "id.csv";
        // // cout << "inode = " << inode << endl;
        // // cout << iid << endl;
        // indexGraph IG(G, N, inode, iid);
        // cout << "index start" << endl;
        // for (int i = 0; i < times; i++)
        // {
        //     indextime += IG.calTri(times == 1);
        //     cout << "\r";
        // }
        // IG.reportRatio();
        // cout << "IG Rate: " << OGT / IGT << endl;
        // cout << "Index Time: " << IGT << endl;
    } {
        int round = stoi(argv[5]);
        cout << endl
             << "test:" << test << endl
             << endl;
        std::string hdirect = outdirect + name + "/" + name + "_range_nb";
        hdirect += "/" + name + "_range_nb_";
        // cout << hid << endl;
        hashGraph *multiHG[round];
        for (int i = 0; i < round; i++)
        {
            std::string hnode = hdirect + "node_" + to_string(i + 1) + ".csv";
            std::string hid = hdirect + "id_" + to_string(i + 1) + ".csv";
            multiHG[i] = new hashGraph(G, N, hnode, hid);
        }
        std::string hGood = hdirect + "Good.csv";
        unordered_map<std::pair<int, int>, int, utils::pair_hash> turn;
        auto start_time2 = chrono::high_resolution_clock::now();
        utils::readGood(hGood, turn);
        auto end_time2 = chrono::high_resolution_clock::now();
        chrono::duration<double> diff2 = end_time2 - start_time2;
        cout << "readGood time:" << diff2.count() << endl;
        cout << "multi start" << endl;
        for (int i = 0; i < times; i++)
        {
            multirangetime += multicalTri(G, turn, multiHG, 1, N, degrees);
            cout << "\r";
        }
        cout << "multirange" << endl;
        reportRatio(G, turn, multiHG, N);
        // for (int i = 1; i <= round; i++)
        // {
        //     hashGraph *current = multiHG[i - 1];
        //     ReadRemain(0, 3766, current, argv[4], hdirect);
        // }
        // string lambda = "10";
        // viewRemain(hdirect + lambda + "_BadRemain.csv", multiHG[0], lambda, hdirect);
    }
    cout << "Time for range:" << rangetime / times << endl;
    cout << "Time for multiglobal:" << multirangetime / times << endl;
    cout << "\n>" << name << "<\n"
         << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    cout << OGT / times << "\t" << origtime / times << "\t" << rangetime / times << "\t" << indextime / times << endl;
    cout << OGT / origtime << "\t" << OGT / rangetime << "\t" << OGT / indextime << endl;
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    delete[] G;
}
