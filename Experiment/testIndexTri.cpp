#include "indexGraph.h"
;
int main(int argc,char** argv) {
    string directname = "../data/";
    string outdirect = "../hash_data/";
    string groDirect = "../gro_data/";
    string name = argv[1];
    int N = stoi(argv[2]);
    bool test = stoi(argv[3]);
    vec* G = new vec[N];
    utils::readGraph(directname+name+".txt", G, N);
    Graph OG(G, N);
    double OGT=0;
    OGT = OG.calTri();
    // cout << "Origin Time: " << OGT << endl;
    // {
    //     int* hashes = new int[N];
    //     iota(hashes, hashes+N, 0);
    //     hashGraph HG(G, hashes, N);
    //     double HGT = HG.calTri();
    //      HG.reportRatio();
    //     cout << "Orig Order Rate: " << OGT/HGT << endl;
    //     cout << "Orig Order Time: " << HGT << endl;
    //     delete[] hashes;
    // }
    // {
    //     groDirect += name;
    //     stringvec groFiles;
    //     utils::read_directory(groDirect, groFiles);
    //     for (const string& f:groFiles) {
    //         hashGraph HG(G, N, f);
    //         double GROT = HG.calTri();
    //         cout << f << " Rate: " << OGT/GROT << endl;
    //     }
    // }
    if(test){
        name="test_"+name;
    }
    // {
    //     string hdirect = outdirect  + name + "_range_nb_";
    //     string hnode = hdirect + "node.csv";
    //     string hid = hdirect + "id.csv";
    //     cout << hnode << endl;
    //     cout << hid << endl;
    //     hashGraph HG(G, N, hnode, hid);
    //     double HGT = HG.calTri();
    //     HG.reportRatio();
    //     cout << "HG Rate: " << OGT/HGT << endl;
    //     cout << "Range Time: " << HGT << endl;
    // }
    extern chrono::duration<double> mergetime;
    {
        string idirect = outdirect  + name + "_index_nb_";
        string inode = idirect + "node.csv";
        string iid = idirect + "id.csv";
        cout << inode << endl;
        cout << iid << endl;
        indexGraph IG(G, N, inode, iid);
        double IGT = IG.calTri();
        IG.reportRatio();
        cout << "IG Rate: " << OGT/IGT << endl;
        // cout<<"Fourline Time:"<<mergetime.count()<<endl<<flush;
        cout << "Index Time: " << IGT << endl;
    }
    delete[] G;
}