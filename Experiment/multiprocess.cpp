#include "indexGraph.h"
;
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
    utils::readGraph(directname + name + ".txt", G, N);
    Graph OG(G, N);
    double OGT = 0;
    for (int i = 0; i < times; i++)
    {
        OGT += OG.calTri(times==1);
        cout << "\r";
    }

    delete[] G;
}