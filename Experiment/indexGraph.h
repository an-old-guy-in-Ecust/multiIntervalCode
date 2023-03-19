#ifndef INDEX_GRAPH_H
#define INDEX_GRAPH_H
#include "hashGraph.h"
// #define CNNBV 2
class indexGraph : public hashGraph {
public:
    indexGraph(const vec* OG, int* hashes, const int N)
    : hashEntries(new vec[N]), hashIds(new vec[N]), hashGraph(hashes, N) {
        processHGIndex(OG);
    }
    indexGraph(const vec* OG, const int N, const string hfilename, const string idfilename)
    : hashEntries(new vec[N]), hashIds(new vec[N]), hashGraph(N, hfilename, idfilename) {
        processHGIndex(OG);
    }
    ~indexGraph() {
        delete[] hashEntries;
        delete[] hashIds;
    }

    virtual void commonNeighbor(const int i, const int n, unsigned long& result);
    virtual void commonNeighborNB(const int i, const int n, unsigned long& result);
    void reportRatio();
    virtual double calF(const pvec& nodePairs, const bool NB);
    double calF2(const pvec& nodePairs, const bool NB);

    void calRate();
    
protected:
    vec* hashEntries;
    vec* hashIds;
    int** hashEntriesA;
    int** hashIdsA;

private:
    void processHGIndex(const vec* OG);
    pair<int, int> calCP2(const int i, const int adj2, const int m, const int M);
};

#endif