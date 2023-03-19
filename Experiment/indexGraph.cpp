#include "indexGraph.h"

void indexGraph::processHGIndex(const vec *OG)
{
    // cout << "\nHere\n"
    //      << flush;
    tqdm bar;
    hashIdsA = new int *[N];
    hashEntriesA = new int *[N];
    for (int i = 0; i < N; i++)
    {
        int newid = ids[i];
        G[newid].reserve(OG[i].size());
        for (const int &adj : OG[i])
            G[newid].push_back(ids[adj]);
        sort(G[newid].begin(), G[newid].end());
        int prev = INT_MIN;
        int j = 0;
        while (j != G[newid].size() && hashes[newid] != INT_MAX)
        {
            int adj = G[newid][j];
            if (hashes[adj] != prev)
            {
                prev = hashes[adj];
                if (prev == INT_MAX)
                    break;
                hashEntries[newid].push_back(prev);
                hashIds[newid].push_back(j);
            }
            j++;
        }
        if (!hashEntries[newid].empty())
        {
            hashesMin[newid] = hashEntries[newid].front();
            hashesMax[newid] = hashEntries[newid].back();
        }
        else
        {
            hashesMin[newid] = INT_MAX;
            hashesMax[newid] = INT_MIN;
        }
        degrees[newid] = hashEntries[newid].size();
        hashEntries[newid].push_back(INT_MAX);
        hashIds[newid].push_back(j);
        hashIdsA[newid] = new int[hashIds[newid].size()];
        // hashEntriesA[newid]=new int[hashEntries[newid].size()];
        for (int i = 0; i < hashIds[newid].size(); i++)
        {
            hashIdsA[newid][i] = hashIds[newid][i];
        }
        // for(int i = 0;i<hashEntries[newid].size();i++){
        //     hashEntriesA[newid][i]=hashEntries[newid][i];
        // }
        bar.progress(i + 1, N);
    }
}

#if CNV == 1
void indexGraph::commonNeighbor(const int i, const int n, unsigned long &result)
{
    int l1 = degrees[i];
    int l2 = degrees[n];
    vec &vec1 = G[i];
    vec &vec2 = G[n];
    int s1 = vec1.size();
    int s2 = vec2.size();
    int m1 = hashesMin[i];
    int m2 = hashesMin[n];
    int M1 = hashesMax[i];
    int M2 = hashesMax[n];
    if (s1 == 1 && l1 == 0)
    {
        l2 = hashIds[n][l2];
        int v = vec1[0];
        if (s2 == 1 && v == vec2[0])
            result++;
        else if (l2 < s2 && v >= vec2[l2] && v <= vec2[s2 - 1])
        {
            int lb = utils::binarySearch(v, vec2, l2, s2);
            if (lb != s2 && vec2[lb] == v)
                result++;
        }
    }
    else if (s2 == 1 && l2 == 0 && l1 < s1)
    {
        l1 = hashIds[i][l1];
        int v = vec2[0];
        if (v >= vec1[l1] && v <= vec1[s1 - 1])
        {
            int lb = utils::binarySearch(v, vec1, l1, s1);
            if (lb != s1 && vec1[lb] == v)
                result++;
        }
    }
    else if (l1 != 0 && l2 != 0)
    {
        vec &hashEnts1 = hashEntries[i];
        vec &hashEnts2 = hashEntries[n];
        vec &hashId1 = hashIds[i];
        vec &hashId2 = hashIds[n];
        if (l1 == 1)
        {
            if (l2 == 1 && vec1[0] == vec2[0])
                result++;
            else
            {
                int v = vec1[0];
                if (v >= vec2[0] && v <= vec2[l2 - 1])
                {
                    int lb = utils::binarySearch(v, vec2, 0, l2);
                    if (lb != l2 && vec2[lb] == v)
                        result++;
                }
            }
        }
        else if (l2 == 1)
        {
            int v = vec2[0];
            if (v >= vec1[0] && v <= vec1[s1 - 1])
            {
                int lb = utils::binarySearch(v, vec1, 0, l1);
                if (lb != l1 && vec1[lb] == v)
                    result++;
            }
        }
        else if (max(m1, m2) <= min(M1, M2))
        {
            int b1 = 0;
            int f1 = l1;
            int b2 = 0;
            int f2 = l2;
            if (m1 < m2)
                b1 = utils::binarySearch(m2, hashEnts1, b1, f1);
            else if (m1 > m2)
                b2 = utils::binarySearch(m1, hashEnts2, b2, f2);
            if (M1 > M2)
                f1 = utils::binarySearch(M2 + 1, hashEnts1, b1, f1);
            else if (M1 < M2)
                f2 = utils::binarySearch(M1 + 1, hashEnts2, b2, f2);
            while (b1 != f1 && b2 != f2)
            {
                if (hashEnts1[b1] == hashEnts2[b2])
                {
                    int vit1 = hashId1[b1];
                    int vit2 = hashId2[b2];
                    int ved1 = hashId1[b1 + 1];
                    int ved2 = hashId2[b2 + 1];
                    while (vit1 != ved1 && vit2 != ved2)
                    {
                        if (vec1[vit1] == vec2[vit2])
                        {
                            result++;
                            vit1++;
                            vit2++;
                        }
                        else if (vec1[vit1] < vec2[vit2])
                            vit1++;
                        else
                            vit2++;
                    }
                    b1++;
                    b2++;
                }
                else if (hashEnts1[b1] < hashEnts2[b2])
                    b1++;
                else
                    b2++;
            }
        }
    }
}
#elif CNV == 2
void indexGraph::commonNeighbor(const int i, const int n, unsigned long &result)
{
    int l1 = degrees[i];
    int l2 = degrees[n];
    int m1 = hashesMin[i];
    int m2 = hashesMin[n];
    int M1 = hashesMax[i];
    int M2 = hashesMax[n];
    if (l1 != 0 && l2 != 0)
    {
        vec &vec1 = G[i];
        vec &vec2 = G[n];
        vec &hashEnts1 = hashEntries[i];
        vec &hashEnts2 = hashEntries[n];
        vec &hashId1 = hashIds[i];
        vec &hashId2 = hashIds[n];
        int s1 = hashId1[l1];
        int s2 = hashId2[l2];
        if (s1 == 1)
        {
            if (s2 == 1 && vec1[0] == vec2[0])
                result++;
            else
            {
                int v = vec1[0];
                if (v >= vec2[0] && v <= vec2[s2 - 1])
                {
                    int lb = utils::binarySearch(v, vec2, 0, s2);
                    if (lb != s2 && vec2[lb] == v)
                        result++;
                }
            }
        }
        else if (s2 == 1)
        {
            int v = vec2[0];
            if (v >= vec1[0] && v <= vec1[s1 - 1])
            {
                int lb = utils::binarySearch(v, vec1, 0, s1);
                if (lb != s1 && vec1[lb] == v)
                    result++;
            }
        }
        else if (max(m1, m2) <= min(M1, M2))
        {
            int b1 = 0;
            int f1 = l1;
            int b2 = 0;
            int f2 = l2;
            if (m1 < m2)
                b1 = utils::binarySearch(m2, hashEnts1, b1, f1);
            else if (m1 > m2)
                b2 = utils::binarySearch(m1, hashEnts2, b2, f2);
            if (M1 > M2)
                f1 = utils::binarySearch(M2 + 1, hashEnts1, b1, f1);
            else if (M1 < M2)
                f2 = utils::binarySearch(M1 + 1, hashEnts2, b2, f2);
            while (b1 != f1 && b2 != f2)
            {
                if (hashEnts1[b1] == hashEnts2[b2])
                {
                    int vit1 = hashId1[b1];
                    int vit2 = hashId2[b2];
                    int ved1 = hashId1[b1 + 1];
                    int ved2 = hashId2[b2 + 1];
                    while (vit1 != ved1 && vit2 != ved2)
                    {
                        if (vec1[vit1] == vec2[vit2])
                        {
                            result++;
                            vit1++;
                            vit2++;
                        }
                        else if (vec1[vit1] < vec2[vit2])
                            vit1++;
                        else
                            vit2++;
                    }
                    b1++;
                    b2++;
                }
                else if (hashEnts1[b1] < hashEnts2[b2])
                    b1++;
                else
                    b2++;
            }
        }
    }
}
#elif CNV == 3
void indexGraph::commonNeighbor(const int i, const int n, unsigned long &result)
{
    int l1 = degrees[i];
    int l2 = degrees[n];
    vec &vec1 = G[i];
    vec &vec2 = G[n];
    int s1 = vec1.size();
    int s2 = vec2.size();
    int m1 = hashesMin[i];
    int m2 = hashesMin[n];
    int M1 = hashesMax[i];
    int M2 = hashesMax[n];
    if (s1 == 1 && l1 == 0)
    {
        l2 = hashIds[n][l2];
        int v = vec1[0];
        if (s2 == 1 && v == vec2[0])
            result++;
        else if (l2 < s2 && v >= vec2[l2] && v <= vec2[s2 - 1])
        {
            int lb = utils::binarySearch(v, vec2, l2, s2);
            if (lb != s2 && vec2[lb] == v)
                result++;
        }
    }
    else if (s2 == 1 && l2 == 0 && l1 < s1)
    {
        l1 = hashIds[i][l1];
        int v = vec2[0];
        if (v >= vec1[l1] && v <= vec1[s1 - 1])
        {
            int lb = utils::binarySearch(v, vec1, l1, s1);
            if (lb != s1 && vec1[lb] == v)
                result++;
        }
    }
    else if (l1 != 0 && l2 != 0 && max(m1, m2) <= min(M1, M2))
    {
        vec &hashEnts1 = hashEntries[i];
        vec &hashEnts2 = hashEntries[n];
        vec &hashId1 = hashIds[i];
        vec &hashId2 = hashIds[n];
        int b1 = 0;
        int f1 = l1;
        int b2 = 0;
        int f2 = l2;
        if (m1 < m2)
            b1 = utils::binarySearch(m2, hashEnts1, b1, f1);
        else if (m1 > m2)
            b2 = utils::binarySearch(m1, hashEnts2, b2, f2);
        if (M1 > M2)
            f1 = utils::binarySearch(M2 + 1, hashEnts1, b1, f1);
        else if (M1 < M2)
            f2 = utils::binarySearch(M1 + 1, hashEnts2, b2, f2);
        while (b1 != f1 && b2 != f2)
        {
            if (hashEnts1[b1] == hashEnts2[b2])
            {
                int vit1 = hashId1[b1];
                int vit2 = hashId2[b2];
                int ved1 = hashId1[b1 + 1];
                int ved2 = hashId2[b2 + 1];
                while (vit1 != ved1 && vit2 != ved2)
                {
                    if (vec1[vit1] == vec2[vit2])
                    {
                        result++;
                        vit1++;
                        vit2++;
                    }
                    else if (vec1[vit1] < vec2[vit2])
                        vit1++;
                    else
                        vit2++;
                }
                b1++;
                b2++;
            }
            else if (hashEnts1[b1] < hashEnts2[b2])
                b1++;
            else
                b2++;
        }
    }
}
#endif
extern unsigned long long mins;
extern unsigned long long maxs;
extern float avgs;
extern unsigned long long times;
extern unsigned long long actcmpr;
extern unsigned long long orgcmpr;
extern chrono::duration<double> mergetime;
extern chrono::duration<double> bintime;
void indexGraph::reportRatio()
{

    // cout<<"Reduced ratio:"<<(float)mins/maxs<<endl<<flush;
    // cout<<"Reduced ratio(AVG):"<<avgs/times<<endl<<flush;
    if (orgcmpr)
        cout << "Reduced ratio(Actual):" << (float)actcmpr / orgcmpr << endl
             << flush;
    // cout<<"Binary Search Time:"<<bintime.count()<<endl<<flush;
    cout << "Merge Time:" << mergetime.count() << endl
         << flush;
    mins = 0;
    maxs = 0;
    avgs = 0;
    times = 0;
    actcmpr = 0;
    mergetime = chrono::duration<double>(0);
    bintime = chrono::duration<double>(0);
}
#if CNNBV == 3
void indexGraph::commonNeighborNB(const int i1, const int n1, unsigned long &result)
{
    int i = i1, n = n1;
    // times+=1;
    // if(i1>n1){
    //     i=n1;
    //     n=i1;
    // }
    int l1 = degrees[i];
    int l2 = degrees[n];
    // maxs+=G[i].size()+G[n].size();
    // int cmprs=0;
    if (l1 != 0 && l2 != 0)
    {
        int m1 = hashesMin[i];
        int m2 = hashesMin[n];
        int M1 = hashesMax[i];
        int M2 = hashesMax[n];
        vec &vec1 = G[i];
        vec &vec2 = G[n];
        vec &hashEnts1 = hashEntries[i];
        vec &hashEnts2 = hashEntries[n];
        int *hashId1 = hashIdsA[i];
        int *hashId2 = hashIdsA[n];
        int s1 = vec1.size();
        int s2 = vec2.size();
        if (s1 == 1)
        {
            if (s2 == 1 && vec1[0] == vec2[0])
            {
                result++;
                cout << "Something strange happened!" << endl
                     << flush;
            }
            else
            {
                int v = vec1[0];
                if (v >= vec2[0] && v <= vec2[s2 - 1])
                {
                    int lb = utils::binarySearch(v, vec2, 0, s2);
                    if (lb != s2 && vec2[lb] == v)
                    {
                        result++;
                        cout << "Something strange happened!" << endl
                             << flush;
                    }
                }
            }
        }
        else if (s2 == 1)
        {
            int v = vec2[0];
            if (v >= vec1[0] && v <= vec1[s1 - 1])
            {
                int lb = utils::binarySearch(v, vec1, 0, s1);
                if (lb != s1 && vec1[lb] == v)
                {
                    result++;
                    cout << "Something strange happened!" << endl
                         << flush;
                }
            }
        }
        else if (max(m1, m2) <= min(M1, M2))
        {
            int b1 = 0;
            int f1 = l1;
            int b2 = 0;
            int f2 = l2;
            // auto bin_start_time = chrono::high_resolution_clock::now();
            if (m1 < m2)
                b1 = utils::binarySearch(m2, hashEnts1, b1, f1);
            else if (m1 > m2)
                b2 = utils::binarySearch(m1, hashEnts2, b2, f2);
            if (M1 > M2)
                f1 = utils::binarySearch(M2 + 1, hashEnts1, b1, f1);
            else if (M1 < M2)
                f2 = utils::binarySearch(M1 + 1, hashEnts2, b2, f2);
            // bintime += chrono::high_resolution_clock::now()-bin_start_time;
            // mins+=f1-b1+f2-b2;
            // actcmpr+=f1-b1+f2-b2;
            // cmprs+=f1-b1+f2-b2;
            // auto merge_start_time = chrono::high_resolution_clock::now();
            // std::chrono::_V2::system_clock::time_point merge_start_time;
            // auto merge_start_time = chrono::high_resolution_clock::now();
            // mergetime += chrono::high_resolution_clock::now()-merge_start_time;
            while (b1 != f1 && b2 != f2)
            {
                if (hashEnts1[b1] == hashEnts2[b2])
                {

                    // mergetime += chrono::high_resolution_clock::now()-merge_start_time;
                    int vit1 = hashId1[b1];
                    int vit2 = hashId2[b2];
                    int ved1 = hashId1[b1 + 1];
                    int ved2 = hashId2[b2 + 1];

                    // actcmpr++;
                    // mins+=ved1-vit1+ved2-vit2;
                    // actcmpr+=ved1-vit1+ved2-vit2-1;
                    // cmprs+=ved1-vit1+ved2-vit2;
                    while (vit1 != ved1 && vit2 != ved2)
                    {
                        if (vec1[vit1] == vec2[vit2])
                        {
                            result++;
                            // actcmpr--;
                            // actcmpr++;
                            vit1++;
                            vit2++;
                        }
                        else if (vec1[vit1] < vec2[vit2])
                        {
                            // if ((ved1 - vit1) > 32 * (ved2 - vit2))
                                vit1 = utils::binarySearch(vec2[vit2], vec1, vit1, ved1);
                            // else
                            // {
                            //     while (vec1[vit1] < vec2[vit2])
                            //         vit1++;
                            // }
                            // actcmpr++;
                        }
                        else
                        {
                            // if ((ved2 - vit2) > 32 * (ved1 - vit1))
                                vit2 = utils::binarySearch(vec1[vit1], vec2, vit2, ved2);
                            // else
                            // {
                            //     while (vec1[vit1] > vec2[vit2])
                            //         vit2++;
                            // }
                            // actcmpr++;
                        }
                        if (vit1 == ved1 || vit2 == ved2)
                        {
                            break;
                        }
                        else if (vec1[ved1 - 1] == vec2[ved2 - 1])
                        {
                            result++;
                            ved1--;
                            ved2--;
                        }
                        else if (vec1[ved1 - 1] > vec2[ved2 - 1])
                        {
                            // if ((ved1 - vit1) > 32 * (ved2 - vit2))
                                ved1 = utils::binarySearch(vec2[ved2 - 1] + 1, vec1, vit1, ved1);
                            // else
                            // {
                            //     while (vec1[ved1 - 1] > vec2[ved2 - 1])
                            //     {
                            //         ved1--;
                            //     }
                            // }
                        }
                        else
                        {
                            // if ((ved2 - vit2) > 32 * (ved1 - vit1))
                                ved2 = utils::binarySearch(vec1[ved1 - 1] + 1, vec2, vit2, ved2);
                            // else
                            // {
                            //     while (vec1[ved1 - 1] < vec2[ved2 - 1])
                            //     {
                            //         ved2--;
                            //     }
                            // }
                            
                        }
                    }
                    // actcmpr-=ved1-vit1+ved2-vit2;
                    b1++;
                    b2++;
                }
                else if (hashEnts1[b1] < hashEnts2[b2])
                {
                    // if ((f1 - b1) > 32 * (f2 - b2))
                        b1 = utils::binarySearch(hashEnts2[b2], hashEnts1, b1, f1);
                    // else
                    // {
                    //     while (hashEnts1[b1] < hashEnts2[b2])
                    //         b1++;
                    // }

                    // actcmpr++;
                }
                else
                {
                    // if ((f2 - b2) > 32 * (f1 - b1))
                        b2 = utils::binarySearch(hashEnts1[b1], hashEnts2, b2, f2);
                    // else
                    // {
                    //     while (hashEnts1[b1] > hashEnts2[b2])
                    //         b2++;
                    // }
                    // actcmpr++;
                }
                if (b1 == f1 || b2 == f2)
                {
                    break;
                }
                if (hashEnts1[f1 - 1] == hashEnts2[f2 - 1])
                {

                    // mergetime += chrono::high_resolution_clock::now()-merge_start_time;
                    int vit1 = hashId1[f1 - 1];
                    int vit2 = hashId2[f2 - 1];
                    int ved1 = hashId1[f1];
                    int ved2 = hashId2[f2];

                    // actcmpr++;
                    // mins+=ved1-vit1+ved2-vit2;
                    // actcmpr+=ved1-vit1+ved2-vit2-1;
                    // cmprs+=ved1-vit1+ved2-vit2;
                    while (vit1 != ved1 && vit2 != ved2)
                    {
                        if (vec1[vit1] == vec2[vit2])
                        {
                            result++;
                            // actcmpr--;
                            // actcmpr++;
                            vit1++;
                            vit2++;
                        }
                        else if (vec1[vit1] < vec2[vit2])
                        {
                            vit1 = utils::binarySearch(vec2[vit2], vec1, vit1, ved1);
                            // actcmpr++;
                        }
                        else
                        {
                            vit2 = utils::binarySearch(vec1[vit1], vec2, vit2, ved2);
                            // actcmpr++;
                        }
                        if (vit1 == ved1 || vit2 == ved2)
                        {
                            break;
                        }
                        else if (vec1[ved1 - 1] == vec2[ved2 - 1])
                        {
                            result++;
                            ved1--;
                            ved2--;
                        }
                        else if (vec1[ved1 - 1] > vec2[ved2 - 1])
                            ved1 = utils::binarySearch(vec2[ved2 - 1] + 1, vec1, vit1, ved1);
                        else
                            ved2 = utils::binarySearch(vec1[ved1 - 1] + 1, vec2, vit2, ved2);
                    }
                    // actcmpr-=ved1-vit1+ved2-vit2;
                    f1--;
                    f2--;
                }
                else if (hashEnts1[f1 - 1] > hashEnts2[f2 - 1])
                {
                    // if ((f1 - b1) > 32 * (f2 - b2))
                        f1 = utils::binarySearch(hashEnts2[f2 - 1] + 1, hashEnts1, b1, f1);
                    // else
                    // {
                    //     while (hashEnts1[f1 - 1] > hashEnts2[f2 - 1])
                    //     {
                    //         f1--;
                    //     }
                    // }

                    // actcmpr++;
                }
                else
                {
                    // if ((f2 - b2) > 32 * (f1 - b1))
                        f2 = utils::binarySearch(hashEnts1[f1 - 1] + 1, hashEnts2, b2, f2);
                    // else
                    // {
                    //     while (hashEnts1[f1 - 1] < hashEnts2[f2 - 1])
                    //     {
                    //         f2--;
                    //     }
                    // }

                    // actcmpr++;
                }
            }

            // actcmpr-=f1-b1+f2-b2;
        }
    }
    // avgs+=(float)cmprs/(G[i].size()+G[n].size());
}
#elif CNNBV == 4
void indexGraph::commonNeighborNB(const int i1, const int n1, unsigned long &result)
{
    int i = i1, n = n1;
    // times+=1;
    // if(i1>n1){
    //     i=n1;
    //     n=i1;
    // }
    int l1 = degrees[i];
    int l2 = degrees[n];
    // maxs+=G[i].size()+G[n].size();
    // int cmprs=0;
    if (l1 != 0 && l2 != 0)
    {
        int m1 = hashesMin[i];
        int m2 = hashesMin[n];
        int M1 = hashesMax[i];
        int M2 = hashesMax[n];
        vec &vec1 = G[i];
        vec &vec2 = G[n];
        vec &hashEnts1 = hashEntries[i];
        vec &hashEnts2 = hashEntries[n];
        int *hashId1 = hashIdsA[i];
        int *hashId2 = hashIdsA[n];
        int s1 = vec1.size();
        int s2 = vec2.size();
        if (s1 == 1)
        {
            if (s2 == 1 && vec1[0] == vec2[0])
            {
                result++;
                cout << "Something strange happened!" << endl
                     << flush;
            }
            else
            {
                int v = vec1[0];
                if (v >= vec2[0] && v <= vec2[s2 - 1])
                {
                    int lb = utils::binarySearch(v, vec2, 0, s2);
                    if (lb != s2 && vec2[lb] == v)
                    {
                        result++;
                        cout << "Something strange happened!" << endl
                             << flush;
                    }
                }
            }
        }
        else if (s2 == 1)
        {
            int v = vec2[0];
            if (v >= vec1[0] && v <= vec1[s1 - 1])
            {
                int lb = utils::binarySearch(v, vec1, 0, s1);
                if (lb != s1 && vec1[lb] == v)
                {
                    result++;
                    cout << "Something strange happened!" << endl
                         << flush;
                }
            }
        }
        else if (max(m1, m2) <= min(M1, M2))
        {
            int b1 = 0;
            int f1 = l1;
            int b2 = 0;
            int f2 = l2;
            // auto bin_start_time = chrono::high_resolution_clock::now();
            if (m1 < m2)
                b1 = utils::binarySearch(m2, hashEnts1, b1, f1);
            else if (m1 > m2)
                b2 = utils::binarySearch(m1, hashEnts2, b2, f2);
            if (M1 > M2)
                f1 = utils::binarySearch(M2 + 1, hashEnts1, b1, f1);
            else if (M1 < M2)
                f2 = utils::binarySearch(M1 + 1, hashEnts2, b2, f2);
            // bintime += chrono::high_resolution_clock::now()-bin_start_time;
            // mins+=f1-b1+f2-b2;
            // actcmpr+=f1-b1+f2-b2;
            // cmprs+=f1-b1+f2-b2;
            // auto merge_start_time = chrono::high_resolution_clock::now();
            // std::chrono::_V2::system_clock::time_point merge_start_time;
            // auto merge_start_time = chrono::high_resolution_clock::now();
            // mergetime += chrono::high_resolution_clock::now()-merge_start_time;
            while (b1 != f1 && b2 != f2)
            {
                if (hashEnts1[b1] == hashEnts2[b2])
                {

                    // mergetime += chrono::high_resolution_clock::now()-merge_start_time;
                    int vit1 = hashId1[b1];
                    int vit2 = hashId2[b2];
                    int ved1 = hashId1[b1 + 1];
                    int ved2 = hashId2[b2 + 1];

                    // actcmpr++;
                    // mins+=ved1-vit1+ved2-vit2;
                    // actcmpr+=ved1-vit1+ved2-vit2-1;
                    // cmprs+=ved1-vit1+ved2-vit2;
                    while (vit1 != ved1 && vit2 != ved2)
                    {
                        if (vec1[vit1] == vec2[vit2])
                        {
                            result++;
                            // actcmpr--;
                            // actcmpr++;
                            vit1++;
                            vit2++;
                        }
                        else if (vec1[vit1] < vec2[vit2])
                        {
                            if ((ved1 - vit1) > 32 * (ved2 - vit2))
                                vit1 = utils::binarySearch(vec2[vit2], vec1, vit1, ved1);
                            else
                            {
                                while (vec1[vit1] < vec2[vit2])
                                    vit1++;
                            }
                            // actcmpr++;
                        }
                        else
                        {
                            if ((ved2 - vit2) > 32 * (ved1 - vit1))
                                vit2 = utils::binarySearch(vec1[vit1], vec2, vit2, ved2);
                            else
                            {
                                while (vec1[vit1] > vec2[vit2])
                                    vit2++;
                            }
                            // actcmpr++;
                        }
                        if (vit1 == ved1 || vit2 == ved2)
                        {
                            break;
                        }
                        else if (vec1[ved1 - 1] == vec2[ved2 - 1])
                        {
                            result++;
                            ved1--;
                            ved2--;
                        }
                        else if (vec1[ved1 - 1] > vec2[ved2 - 1])
                        {
                            if ((ved1 - vit1) > 32 * (ved2 - vit2))
                                ved1 = utils::binarySearch(vec2[ved2 - 1] + 1, vec1, vit1, ved1);
                            else
                            {
                                while (vec1[ved1 - 1] > vec2[ved2 - 1])
                                {
                                    ved1--;
                                }
                            }
                        }
                        else
                        {
                            if ((ved2 - vit2) > 32 * (ved1 - vit1))
                                ved2 = utils::binarySearch(vec1[ved1 - 1] + 1, vec2, vit2, ved2);
                            else
                            {
                                while (vec1[ved1 - 1] < vec2[ved2 - 1])
                                {
                                    ved2--;
                                }
                            }
                            
                        }
                    }
                    // actcmpr-=ved1-vit1+ved2-vit2;
                    b1++;
                    b2++;
                }
                else if (hashEnts1[b1] < hashEnts2[b2])
                {
                    if ((f1 - b1) > 32 * (f2 - b2))
                        b1 = utils::binarySearch(hashEnts2[b2], hashEnts1, b1, f1);
                    else
                    {
                        while (hashEnts1[b1] < hashEnts2[b2])
                            b1++;
                    }

                    // actcmpr++;
                }
                else
                {
                    if ((f2 - b2) > 32 * (f1 - b1))
                        b2 = utils::binarySearch(hashEnts1[b1], hashEnts2, b2, f2);
                    else
                    {
                        while (hashEnts1[b1] > hashEnts2[b2])
                            b2++;
                    }
                    // actcmpr++;
                }
                if (b1 == f1 || b2 == f2)
                {
                    break;
                }
                if (hashEnts1[f1 - 1] == hashEnts2[f2 - 1])
                {

                    // mergetime += chrono::high_resolution_clock::now()-merge_start_time;
                    int vit1 = hashId1[f1 - 1];
                    int vit2 = hashId2[f2 - 1];
                    int ved1 = hashId1[f1];
                    int ved2 = hashId2[f2];

                    // actcmpr++;
                    // mins+=ved1-vit1+ved2-vit2;
                    // actcmpr+=ved1-vit1+ved2-vit2-1;
                    // cmprs+=ved1-vit1+ved2-vit2;
                    while (vit1 != ved1 && vit2 != ved2)
                    {
                        if (vec1[vit1] == vec2[vit2])
                        {
                            result++;
                            // actcmpr--;
                            // actcmpr++;
                            vit1++;
                            vit2++;
                        }
                        else if (vec1[vit1] < vec2[vit2])
                        {
                            if ((ved1 - vit1) > 32 * (ved2 - vit2))
                                vit1 = utils::binarySearch(vec2[vit2], vec1, vit1, ved1);
                            else
                            {
                                while (vec1[vit1] < vec2[vit2])
                                    vit1++;
                            }
                            // actcmpr++;
                        }
                        else
                        {
                            if ((ved2 - vit2) > 32 * (ved1 - vit1))
                                vit2 = utils::binarySearch(vec1[vit1], vec2, vit2, ved2);
                            else
                            {
                                while (vec1[vit1] > vec2[vit2])
                                    vit2++;
                            }
                            // actcmpr++;
                        }
                        if (vit1 == ved1 || vit2 == ved2)
                        {
                            break;
                        }
                        else if (vec1[ved1 - 1] == vec2[ved2 - 1])
                        {
                            result++;
                            ved1--;
                            ved2--;
                        }
                        else if (vec1[ved1 - 1] > vec2[ved2 - 1])
                        {
                            if ((ved1 - vit1) > 32 * (ved2 - vit2))
                                ved1 = utils::binarySearch(vec2[ved2 - 1] + 1, vec1, vit1, ved1);
                            else
                            {
                                while (vec1[ved1 - 1] > vec2[ved2 - 1])
                                {
                                    ved1--;
                                }
                            }
                        }
                        else
                        {
                            if ((ved2 - vit2) > 32 * (ved1 - vit1))
                                ved2 = utils::binarySearch(vec1[ved1 - 1] + 1, vec2, vit2, ved2);
                            else
                            {
                                while (vec1[ved1 - 1] < vec2[ved2 - 1])
                                {
                                    ved2--;
                                }
                            }
                            
                        }
                    }
                    // actcmpr-=ved1-vit1+ved2-vit2;
                    f1--;
                    f2--;
                }
                else if (hashEnts1[f1 - 1] > hashEnts2[f2 - 1])
                {
                    if ((f1 - b1) > 32 * (f2 - b2))
                        f1 = utils::binarySearch(hashEnts2[f2 - 1] + 1, hashEnts1, b1, f1);
                    else
                    {
                        while (hashEnts1[f1 - 1] > hashEnts2[f2 - 1])
                        {
                            f1--;
                        }
                    }

                    // actcmpr++;
                }
                else
                {
                    if ((f2 - b2) > 32 * (f1 - b1))
                        f2 = utils::binarySearch(hashEnts1[f1 - 1] + 1, hashEnts2, b2, f2);
                    else
                    {
                        while (hashEnts1[f1 - 1] < hashEnts2[f2 - 1])
                        {
                            f2--;
                        }
                    }

                    // actcmpr++;
                }
            }

            // actcmpr-=f1-b1+f2-b2;
        }
    }
    // avgs+=(float)cmprs/(G[i].size()+G[n].size());
}
#elif CNNBV == 1
void indexGraph::commonNeighborNB(const int i1, const int n1, unsigned long &result)
{
    int i = i1, n = n1;
    // times+=1;
    // if(i1>n1){
    //     i=n1;
    //     n=i1;
    // }
    int l1 = degrees[i];
    int l2 = degrees[n];
    // maxs+=G[i].size()+G[n].size();
    // int cmprs=0;
    if (l1 != 0 && l2 != 0)
    {
        int m1 = hashesMin[i];
        int m2 = hashesMin[n];
        int M1 = hashesMax[i];
        int M2 = hashesMax[n];
        vec &vec1 = G[i];
        vec &vec2 = G[n];
        vec &hashEnts1 = hashEntries[i];
        vec &hashEnts2 = hashEntries[n];
        int *hashId1 = hashIdsA[i];
        int *hashId2 = hashIdsA[n];
        int s1 = vec1.size();
        int s2 = vec2.size();
        if (s1 == 1)
        {
            if (s2 == 1 && vec1[0] == vec2[0])
            {
                result++;
                cout << "Something strange happened!" << endl
                     << flush;
            }
            else
            {
                int v = vec1[0];
                if (v >= vec2[0] && v <= vec2[s2 - 1])
                {
                    int lb = utils::binarySearch(v, vec2, 0, s2);
                    if (lb != s2 && vec2[lb] == v)
                    {
                        result++;
                        cout << "Something strange happened!" << endl
                             << flush;
                    }
                }
            }
        }
        else if (s2 == 1)
        {
            int v = vec2[0];
            if (v >= vec1[0] && v <= vec1[s1 - 1])
            {
                int lb = utils::binarySearch(v, vec1, 0, s1);
                if (lb != s1 && vec1[lb] == v)
                {
                    result++;
                    cout << "Something strange happened!" << endl
                         << flush;
                }
            }
        }
        else if (max(m1, m2) <= min(M1, M2))
        {
            int b1 = 0;
            int f1 = l1;
            int b2 = 0;
            int f2 = l2;
            // auto bin_start_time = chrono::high_resolution_clock::now();
            if (m1 < m2)
                b1 = utils::binarySearch(m2, hashEnts1, b1, f1);
            else if (m1 > m2)
                b2 = utils::binarySearch(m1, hashEnts2, b2, f2);
            if (M1 > M2)
                f1 = utils::binarySearch(M2 + 1, hashEnts1, b1, f1);
            else if (M1 < M2)
                f2 = utils::binarySearch(M1 + 1, hashEnts2, b2, f2);
            // bintime += chrono::high_resolution_clock::now()-bin_start_time;
            // mins+=f1-b1+f2-b2;
            // actcmpr+=f1-b1+f2-b2;
            // cmprs+=f1-b1+f2-b2;
            // auto merge_start_time = chrono::high_resolution_clock::now();
            // std::chrono::_V2::system_clock::time_point merge_start_time;
            // auto merge_start_time = chrono::high_resolution_clock::now();
            // mergetime += chrono::high_resolution_clock::now()-merge_start_time;
            while (b1 != f1 && b2 != f2)
            {
                if (hashEnts1[b1] == hashEnts2[b2])
                {

                    // mergetime += chrono::high_resolution_clock::now()-merge_start_time;
                    int vit1 = hashId1[b1];
                    int vit2 = hashId2[b2];
                    int ved1 = hashId1[b1 + 1];
                    int ved2 = hashId2[b2 + 1];

                    // actcmpr++;
                    // mins+=ved1-vit1+ved2-vit2;
                    // actcmpr+=ved1-vit1+ved2-vit2-1;
                    // cmprs+=ved1-vit1+ved2-vit2;
                    while (vit1 != ved1 && vit2 != ved2)
                    {
                        if (vec1[vit1] == vec2[vit2])
                        {
                            result++;
                            // actcmpr--;
                            // actcmpr++;
                            vit1++;
                            vit2++;
                        }
                        else if (vec1[vit1] < vec2[vit2])
                        {
                            vit1++;
                            // actcmpr++;
                        }
                        else
                        {
                            vit2++;
                            // actcmpr++;
                        }
                    }
                    // actcmpr-=ved1-vit1+ved2-vit2;
                    b1++;
                    b2++;
                }
                else if (hashEnts1[b1] < hashEnts2[b2])
                {
                    b1++;
                    // actcmpr++;
                }
                else
                {
                    b2++;
                    // actcmpr++;
                }
            }

            // actcmpr-=f1-b1+f2-b2;
        }
    }
    // avgs+=(float)cmprs/(G[i].size()+G[n].size());
}
#elif CNNBV == 2
void indexGraph::commonNeighborNB(const int i, const int n, unsigned long &result)
{
    int l1 = degrees[i];
    int l2 = degrees[n];
    int m1 = hashesMin[i];
    int m2 = hashesMin[n];
    int M1 = hashesMax[i];
    int M2 = hashesMax[n];
    if (l1 != 0 && l2 != 0 && max(m1, m2) <= min(M1, M2))
    {
        vec &vec1 = G[i];
        vec &vec2 = G[n];
        vec &hashEnts1 = hashEntries[i];
        vec &hashEnts2 = hashEntries[n];
        vec &hashId1 = hashIds[i];
        vec &hashId2 = hashIds[n];
        int b1 = 0;
        int f1 = l1;
        int b2 = 0;
        int f2 = l2;
        if (m1 < m2)
            b1 = utils::binarySearch(m2, hashEnts1, b1, f1);
        else if (m1 > m2)
            b2 = utils::binarySearch(m1, hashEnts2, b2, f2);
        if (M1 > M2)
            f1 = utils::binarySearch(M2 + 1, hashEnts1, b1, f1);
        else if (M1 < M2)
            f2 = utils::binarySearch(M1 + 1, hashEnts2, b2, f2);
        while (b1 != f1 && b2 != f2)
        {
            if (hashEnts1[b1] == hashEnts2[b2])
            {
                int vit1 = hashId1[b1];
                int vit2 = hashId2[b2];
                int ved1 = hashId1[b1 + 1];
                int ved2 = hashId2[b2 + 1];
                while (vit1 != ved1 && vit2 != ved2)
                {
                    if (vec1[vit1] == vec2[vit2])
                    {
                        result++;
                        vit1++;
                        vit2++;
                    }
                    else if (vec1[vit1] < vec2[vit2])
                        vit1++;
                    else
                        vit2++;
                }
                b1++;
                b2++;
            }
            else if (hashEnts1[b1] < hashEnts2[b2])
                b1++;
            else
                b2++;
        }
    }
}
#endif

pair<int, int> indexGraph::calCP2(const int i, const int n, const int m, const int M)
{
    int result = 0;
    vec &hashEnts1 = hashEntries[i];
    vec &hashEnts2 = hashEntries[n];
    vec &hashId1 = hashIds[i];
    vec &hashId2 = hashIds[n];
    int l1 = degrees[i];
    int l2 = degrees[n];
    int b1 = 0;
    int f1 = l1;
    int b2 = 0;
    int f2 = l2;
    if (hashesMin[i] < hashesMin[n])
        b1 = utils::binarySearch(hashesMin[n], hashEnts1, b1, f1);
    else if (hashesMin[i] > hashesMin[n])
        b2 = utils::binarySearch(hashesMin[i], hashEnts2, b2, f2);
    if (hashesMax[i] > hashesMax[n])
        f1 = utils::binarySearch(hashesMax[n] + 1, hashEnts1, b1, f1);
    else if (hashesMax[i] < hashesMax[n])
        f2 = utils::binarySearch(hashesMax[i] + 1, hashEnts2, b2, f2);
    int add = f1 - b1 + f2 - b2;
    while (b1 != f1 && b2 != f2)
    {
        if (hashEnts1[b1] == hashEnts2[b2])
        {
            result += hashId1[b1 + 1] - hashId1[b1] + hashId2[b2 + 1] - hashId2[b2];
            b1++;
            b2++;
        }
        else if (hashEnts1[b1] < hashEnts2[b2])
            b1++;
        else
            b2++;
    }
    return make_pair(result, result + add);
}

double indexGraph::calF(const pvec &nodePairs, const bool NB)
{
    tqdm bar;
    int marked = 0;
    long double CNDP = 0.0;
    for (const pair<int, int> &p : nodePairs)
    {
        int i = ids[p.first];
        int n = ids[p.second];
        unsigned long CN = 0;
        int CP = 0;
        Graph::commonNeighbor(i, n, CN);
        if (G[i].size() == 1 || G[n].size() == 1 || hashIds[i][degrees[i]] == 1 || hashIds[n][degrees[n]] == 1)
            CP = 2;
        else if (degrees[i] != 0 && degrees[n] != 0)
        {
            int m = max(hashesMin[i], hashesMin[n]);
            int M = min(hashesMax[i], hashesMax[n]);
            if (m <= M)
                CP = calCP2(i, n, m, M).second;
        }
        CNDP = CNDP + (double)(2 * CN + 1) / (CP + 1);
        bar.progress(++marked, N);
    }
    CNDP = CNDP / nodePairs.size();
    return CNDP;
}

double indexGraph::calF2(const pvec &nodePairs, const bool NB)
{
    tqdm bar;
    int marked = 0;
    long double CNDP = 0.0;
    for (const pair<int, int> &p : nodePairs)
    {
        int i = ids[p.first];
        int n = ids[p.second];
        unsigned long CN = 0;
        int CP = 0;
        Graph::commonNeighbor(i, n, CN);
        if (G[i].size() == 1 || G[n].size() == 1 || hashIds[i][degrees[i]] == 1 || hashIds[n][degrees[n]] == 1)
            CP = 2;
        else if (degrees[i] != 0 && degrees[n] != 0)
        {
            int m = max(hashesMin[i], hashesMin[n]);
            int M = min(hashesMax[i], hashesMax[n]);
            if (m <= M)
                CP = calCP2(i, n, m, M).first;
        }
        CNDP = CNDP + (double)(2 * CN + 1) / (CP + 1);
        bar.progress(++marked, N);
    }
    CNDP = CNDP / nodePairs.size();
    return CNDP;
}

void indexGraph::calRate()
{
    double r = 0.0;
    for (int i = 0; i < N; i++)
    {
        r = r + (double)(hashEntries[i].size() - 1) / G[i].size();
    }
    r = r / N;
    cout << "R: " << r << endl;
}