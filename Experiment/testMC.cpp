#include "indexMCGraph.h"
#include "hashGraph.h"

typedef unordered_set<int> intset;
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
void getDegOrder(vec *&G, int *&Vrank, int N)
{
    // Degeneracy ordering
    // order: result
    // Vrank[i]: the order of node i
    int *order = new int[N];
    // tqdm bar;
    int *degrees = new int[N]; // 度向量
    map<int, intset> D;        // D[d]: 有哪些节点的度为d
    int d;
    for (int i = 0; i < N; i++)
    {
        d = G[i].size();
        degrees[i] = d;
        D[d].insert(i);
    }
    boost::dynamic_bitset<> mark(N);
    int marked = 0;
    int n;
    // int prevd=0;
    // for (map<int, intset>::iterator it=D.begin(); it!=D.end(); it++) {
    //     // if(prevd>it->first)
    //         cout<<it->first<<endl<<flush;
    //     // prevd=it->first;
    //     }
    // cout<<"OK!"<<endl<<flush;
    while (marked < N)
    {
        for (map<int, intset>::iterator it = D.begin(); it != D.end(); it++)
        {
            if (!it->second.empty())
            {
                n = *(it->second.begin());
                break;
            }
        }
        mark.set(n);
        d = degrees[n];
        D[d].erase(n);
        for (const auto &adj : G[n])
        {
            if (!mark.test(n))
            {
                d = degrees[adj];
                D[d].erase(adj);
                D[d - 1].insert(adj);
                degrees[adj] = d - 1;
            }
        }
        Vrank[n] = marked;
        order[marked++] = n;
        // bar.progress(marked, N);
    }
}

int main(int argc, char **argv)
{
    std::string directname = "../data/";
    std::string outdirect = "../hash_data/";
    std::string groDirect = "../gro_data/";
    std::string name = argv[1];
    int N = stoi(argv[2]);
    int PN = stoi(argv[3]);
    int test = stoi(argv[4]);
    int times = stoi(argv[5]); // 求MC的次数
    vec nodes(PN);
    for (int i = 0; i < PN; i++)
        nodes[i] = (N / PN) * i;
    vec *G = new vec[N];
    cout << "read Begin" << endl;
    utils::readGraph(directname + name + ".txt", G, N);
    cout << "read Done" << endl;
    int *Vrank = new int[N]; // Vrank[i]: degeneracy order of i
    cout << "get order begin" << endl;
    getDegOrder(G, Vrank, N);
    cout << "get order end" << endl;
    Graph OG(G, N); // Origin Graph
    cout << "origin order start" << endl;
    double OGT = 0;
    for (int i = 0; i < times; i++)
        OGT += OG.MC(nodes, Vrank); // 计算一次MC需要的时间
    cout << "origin order done" << endl;
    double origtime = 0, rangetime = 0, indextime = 0;
    {
        int *hashes = new int[N];
        iota(hashes, hashes + N, 0);
        cout << "Building!\n"
             << flush;
        hashGraph HG(G, hashes, N);
        cout << "Calculation start!\n"
             << flush;
        for (int i = 0; i < times; i++)
            origtime += HG.MC(nodes, Vrank);
        cout << "Orig Order Rate: " << OGT / origtime << endl;
        delete[] hashes;
        cout << "Orignal Order time:" << origtime << "s\n";
    }
    if (test == -1)
    {
        cout << "Original Time: " << OGT / times << endl;
        cout << "Method\tRate\tAvgTime" << endl;
        stringvec groFiles;
        utils::read_directory(groDirect, groFiles); // 读取groDirect下所有文件名
        for (const std::string &f : groFiles)
        {
            int idx = f.find(name);       // 在aa中查找bb.
            if (idx != std::string::npos) // 不存在。
            {
                hashGraph HG(G, N, f);
                int *VrankN = new int[N];
                for (int i = 0; i != N; i++)
                    VrankN[HG.getId(i)] = Vrank[i];
                vec newNodes(nodes.size());
                for (int i = 0; i != nodes.size(); i++)
                    newNodes[i] = HG.getId(nodes[i]);
                double GROT = 0;
                for (int i = 0; i < times; i++)
                {
                    GROT += HG.MC(newNodes, VrankN);
                    cout << "\r";
                }

                cout << f << " Rate: " << OGT / GROT << endl;
                delete[] VrankN;
                cout << get_FileBaseName(f) << "\t" << OGT / GROT << "\t" << GROT / times << endl;
            }
        }
    }
    // groDirect += name;
    // stringvec groFiles;
    // utils::read_directory(groDirect, groFiles);
    // for (const string& f:groFiles) {

    // }
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
        outdirect = "../degree_hash_data/1/";
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
        outdirect = "../onlyp2_hash_data/";
    }
    cout << "test:" << test << endl;
    std::string hdirect = outdirect + name + "/" + name + "_range_nb";
    hdirect += "/" + name + "_range_nb_";
    std::string hnode = hdirect + "node.csv";
    std::string hid = hdirect + "id.csv";
    // cout << hnode << endl;
    // cout << hid << endl;
    {
        cout << hnode << endl;
        hashGraph HG(G, N, hnode, hid);
        int *VrankN = new int[N]; // 更新过id后每个点的BK order顺序
        cout << "get ID" << endl;
        for (int i = 0; i != N; i++)
            VrankN[HG.getId(i)] = Vrank[i];
        cout << "get ID done" << endl;
        vec newNodes(nodes.size());
        for (int i = 0; i != nodes.size(); i++)
            newNodes[i] = HG.getId(nodes[i]);
        cout << "Range Begin" << endl;
        for (int i = 0; i < times; i++)
            rangetime += HG.MC(newNodes, VrankN);
        cout << "HG Rate: " << OGT / rangetime << endl;
        delete[] VrankN;
        cout << "Range time:" << rangetime << "s\n";
    }
    std::string idirect = outdirect + name + "/" + name + "_index_nb";
    idirect += "/" + name + "_index_nb_";
    std::string inode = idirect + "node.csv";
    std::string iid = idirect + "id.csv";
    // cout << inode << endl;
    // cout << iid << endl;
    {
        indexVecGraph IG(G, N, inode, iid);
        // cout << "Build Index Graph Done" << endl;
        int *VrankN = new int[N];
        for (int i = 0; i != N; i++)
            VrankN[IG.getId(i)] = Vrank[i];
        vec newNodes(nodes.size());
        for (int i = 0; i != nodes.size(); i++)
            newNodes[i] = IG.getId(nodes[i]);
        for (int i = 0; i < times; i++)
            indextime += IG.MC(newNodes, VrankN);
        cout << "IG Rate: " << OGT / indextime << endl;
        delete[] VrankN;
        cout << "Index time:" << indextime << "s\n";
    }
    cout << "\n>" << name << "<\n"
         << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    cout << OGT / times << "\t" << origtime / times << "\t" << rangetime / times << "\t" << indextime / times << endl;
    cout << OGT / origtime << "\t" << OGT / rangetime << "\t" << OGT / indextime << endl;
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    delete[] Vrank;
    delete[] G;
}