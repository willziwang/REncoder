
// #include "BF.h"
using namespace std;
extern int pr;
// int levelQuery[64]={0};
// int levelFPR[64]={0};
// set<uint64_t> keySet[64];
// set<uint64_t>::iterator iter;
class Rosetta_RP
{
public:
    static const int
            MAX_BF_NUM = 64;
    uint64_t memory;
    int     hash_num;
    uint64_t L;
    Bloomfilter bf[MAX_BF_NUM];
    void init(uint64_t _memory,
              int _hash_num,
              uint64_t _L)
    {
        memory = _memory,
        hash_num = _hash_num,
        L = _L;
        if (MAX_BF_NUM < _L)
        {
            cout << "Rosetta_RP memory initialization fail 1.\n";
            exit(-1);
        }
        for (int i = 0; i < L; i++)
        {
            bf[i].init(memory / L, hash_num);
        }
    }
    void Insert(uint64_t key)
    {
        for (int i = L; i >=1; i--)
        {
            // if(i==32&&key==10)cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAAa";
            // if(pr) cout<<i<<"_"<<key<<"_";
            // if(pr) cout<<bf[31].query(10)<<"=";
            bf[i-1].insert(key);
            // keySet[i-1].insert(key);
            // if(pr) cout<<bf[31].query(10)<<"=";
            // cout<<"insert "<<key<<" "<<i<<endl;
            key >>= 1;
            // key^=key&(1u<<(L-i));
        }
    }
    bool RangeQuery(uint64_t low, uint64_t high, uint64_t p = 0, uint64_t l = 1)
    {
        // cout << l<<endl;
        // cout<<p<<"_"<< p + (l == 1 ? 0 : 1u << (L - l + 1)) - 1 <<endl;
        if (p > high || p + (l == 1 ? 0 : 1ll << (L - l + 1)) - 1 < low)
            return false;
        if (p >= low && p + (l == 1 ? 0 : 1ll << (L - l + 1)) - 1 <= high)
            return doubt(p, l);
        // cout << l<<endl;
        if (bf[l - 1].query(p>>L-l)&&RangeQuery(low, high, p, l + 1))
        {
            return true;
        }
        else if(bf[l - 1].query(p + (1ll << (L - l))>>L-l))
        {
            return RangeQuery(low, high, p + (1ll << (L - l)), l + 1);
        }
        else
        {
            return false;
        }
    }
    bool doubt(uint64_t p, uint64_t l)
    {
        // cout<<p<<"-"<<l<<endl;
        // if(pr) cout<<(p>>L-l+1)<<"-"<<l-1<<endl;
        if (!bf[l - 2].query(p>>L-l+1))
        {
            // levelQuery[l-2]++;
            return false;
        }
        // if(pr) cout<<p<<"==="<<l<<endl;
        // if((iter=keySet[l-2].find(p>>L-l+1))==keySet[l-2].end())
        // {
        //     levelQuery[l-2]++;
        //     levelFPR[l-2]++;
        // }
        if (l > L)
            return true;
        if (doubt(p, l + 1))
            return true;
        return doubt(p + (1ll << (L - l)), l + 1);
    }
};