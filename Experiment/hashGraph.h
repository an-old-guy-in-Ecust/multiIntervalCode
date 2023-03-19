#ifndef HASH_GRAPH_H
#define HASH_GRAPH_H
#include "Graph.h"

class hashGraph : public Graph
{
public:
    hashGraph(int *hashes, const int N)
        : ids(new int[N]), hashes(new int[N]), hashesMin(new int[N]), hashesMax(new int[N]), Graph(N)
    {
        utils::sort_indexes(hashes, this->hashes, this->ids, N);
    }
    hashGraph(const int N, const string hfilename, const string idfilename)
        : ids(new int[N]), hashes(new int[N]), hashesMin(new int[N]), hashesMax(new int[N]), Graph(N)
    {
        utils::readCSV(hfilename, hashes);
        utils::readId(idfilename, ids);
    }
    hashGraph(const vec *OG, int *hashes, const int N)
        : hashGraph(hashes, N)
    {
        processHG(OG);
    }
    hashGraph(const vec *OG, const int N, const string hfilename, const string idfilename)
        : hashGraph(N, hfilename, idfilename)
    {
        processHG(OG);
    }
    hashGraph(const vec *OG, const int N, const string grofilename)
        : ids(new int[N]), hashes(new int[N]), hashesMin(new int[N]), hashesMax(new int[N]), Graph(N)
    {
        int *currHashes = new int[N];
        utils::readCSV(grofilename, currHashes, false, ' ');
        utils::sort_indexes(currHashes, this->hashes, this->ids, N);
        delete[] currHashes;
        processHG(OG);
    }
    ~hashGraph()
    {
        delete[] G;
        delete[] ids;
        delete[] hashes;
        delete[] hashesMin;
        delete[] hashesMax;
    }
    int getId(const int i) const
    {
        return ids[i];
    }

    void reportRatio();
    void reportRatio(int i, int n, unsigned long long &minimal, unsigned long long &maximal);

    virtual void commonNeighbor(const int i, const int n, unsigned long &result);
    virtual void commonNeighborNB(const int i, const int n, unsigned long &result);
    virtual void commonNeighborBS(const int i, const int j, unsigned long &cn);
    virtual void commonNeighborNBBS(const int i, const int j, unsigned long &cn);
    virtual void commonNeighborCP(const int i, const int n, unsigned long &result);
    virtual void commonNeighborNBCP(const int i, const int n, unsigned long &result);
    virtual void CN(const int i, const int n, unsigned long &result);
    virtual void binaryCN(const int i, const int n, unsigned long &result);
    virtual vec neighbor(const int i);

    virtual double calCN(const pvec &nodePairs, const bool NB);
    virtual double calCNBS(const pvec &nodePairs, const bool NB);
    virtual unsigned long calCNCP(const pvec &nodePairs, const bool NB);
    virtual double calF(const pvec &nodePairs, const bool NB);
    virtual double calR(const pvec &nodePairs);
    // virtual double calTri(bool reportNumber = true);

    virtual double MC(const vec &nodes, const int *Vrank);

    void writeHashId(const string hfilename, const string idfilename);

protected:
    int *ids;
    int *hashes;
    unordered_map<std::pair<int, int>, int, utils::pair_hash> turn;
    int *hashesMin;
    int *hashesMax;
    int selectPivot(const vec &P, const vec &X, const int mp, const int Mp);
    virtual void BKP(vec &P, vec &X, unsigned long &result);

private:
    void processHG(const vec *OG);
    void commonNeighborNB(const vec &vec1, const vec &vec2, unsigned long &result, const int l1, const int l2, const int m, const int M);
    void commonNeighborNB(const vec &vec1, const vec &vec2, vec &result, const int l1, const int l2, const int m, const int M);
};

#endif