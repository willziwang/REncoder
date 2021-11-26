
// #include "BF.h"
using namespace std;
extern int pr;
// int levelQuery[64]={0};
// int levelFPR[64]={0};
// set<uint64_t> keySet[64];
// set<uint64_t>::iterator iter;
extern stack<pair<uint64_t,uint64_t> >psr;
extern stack<pair<uint64_t,uint64_t> >psd;
class Rosetta_LN
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
            cout << "Rosetta_LN memory initialization fail 1.\n";
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
        bool leafFlag = false;
        bool leftBorder = low == 0;
        bool rightBorder = high + 1 == 0;
        if(!leftBorder)
        {
            low -= 1;
        }
        if(!rightBorder)
        {
            high += 1;
        }
        uint64_t diff = low ^ high;
        uint64_t difflen;
        for (difflen = 0; difflen <= L; difflen += 1)
        {
            if (diff == 0)
            {
                break;
            }
            diff >>= 1;
        }
        l = (L - difflen) + 1;
        if (leftBorder && rightBorder)
        {
            if (doubt(p, l + 1))
            {
                return true;
            }
            return doubt(p + (1ll << (L - l)), l + 1);
        }
        else if (rightBorder)
        {
            for (int i = l; i <= L; i++)
            {
                if (!(low >> L - i & 1))
                {
                    p = (i == 1 ? 0 : low >> L - i + 1 << L - i + 1) + (1ll << L - i);
                    if (doubt(p, i + 1))
                    {
                        return true;
                    }
                }
            }
        }
        else if (leftBorder)
        {
            for (int i = l; i <= L; i++)
            {
                if (high >> L - i & 1)
                {
                    p = i == 1 ? 0 : high >> L - i + 1 << L - i + 1;
                    if (doubt(p, i + 1))
                    {
                        return true;
                    }
                }
            }
        }
        else
        {
            for (int i = l + 1; i <= L; i++)
            {
                if (!(low >> L - i & 1))
                {
                    p = (i == 1 ? 0 : low >> L - i + 1 << L - i + 1) + (1ll << L - i);
                    if (doubt(p, i + 1))
                    {
                        return true;
                    }
                }
            }
            for (int i = l + 1; i <= L; i++)
            {
                if (high >> L - i & 1)
                {
                    p = i == 1 ? 0 : high >> L - i + 1 << L - i + 1;
                    if (doubt(p, i + 1))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
        // while(!psr.empty())
        // {
        //     psr.pop();
        // }
        // psr.push(make_pair(p,l));
        // while(!psr.empty())
        // {
        //     p = psr.top().first;
        //     l = psr.top().second;
        //     psr.pop();
        //     if (p > high || p + (l == 1 ? 0 : 1ll << (L - l + 1)) - 1 < low)
        //         continue;
        //     if (p >= low && p + (l == 1 ? 0 : 1ll << (L - l + 1)) - 1 <= high)
        //     {
        //         if(doubt(p, l))
        //         {
        //             return true;
        //         }
        //         else
        //         {
        //             continue;
        //         }       
        //     }
        //     psr.push(make_pair(p + (1ll << (L - l)),l+1));
        //     psr.push(make_pair(p,l+1));
        // }
        // return false;
    }
    bool doubt(uint64_t p, uint64_t l)
    {
        while(!psd.empty())
        {
            psd.pop();
        }
        psd.push(make_pair(p,l));
        while(!psd.empty())
        {
            p = psd.top().first;
            l = psd.top().second;
            psd.pop();
            if (!bf[l - 2].query(p>>L-l+1))
            {
                continue;
            }
            if (l > L)
            {
                return true;
            }
            psd.push(make_pair(p + (1ll << (L - l)),l+1));
            psd.push(make_pair(p,l+1));
        }
        return false;
    }
};