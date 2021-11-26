//#include "sBF.h"

using namespace std;

// extern int testt;

class RSBF_2
{
public:
    static const int
        MAX_BF_NUM = 64;
    int
        count,
        hash_num,
        k_tree;
    uint32_t memory;
    uint64_t L;
    ShiftingBloomfilter sbf;
    void init(uint32_t _memory,
              int _hash_num,
              uint64_t _L)
    {
        count=0;
        memory = _memory,
        hash_num = _hash_num,
        L = _L;
        if (MAX_BF_NUM < _L)
        {
            cout << "RSBF_2 memory initialization fail 1.\n";
            exit(-1);
        }
        sbf.init(_memory, hash_num);
    }
    void Insert4(uint64_t p)
    {
        for (int i = 1; i <= L; i += 1)
        {
            sbf.insertsbm4((p >> 1) + (L - i) * 1000000007, 1u << (p & 1));
            p >>= 1;
        }
        // while(1)
        // {
        //     sbf.insertsbm4(p>>k_tree, p==0?1u:1u<<(p&(1u<<k_tree)-1));
        //     p >>= k_tree;
        //     if(p==0)
        //     {
        //         break;
        //     }
        // }
        // sbf.insertsbm4(p>>k_tree, p==0?1u:1u<<(p&(1u<<k_tree)-1));
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
        uint32_t prevalue = sbf.querysbm4((L - l + 1 == 64 ? 0 : p >> L - l + 1) + (l - 1) * 1000000007);
        if ((prevalue & 1)&&RangeQuery(low, high, p, l + 1))
        {
            return true;
        }
        else if(prevalue & 2)
        {
            return RangeQuery(low, high, p + (1ll << (L - l)), l + 1);
        }
        else
        {
            return false;
        }
        // if (RangeQuery(low, high, p, l + 1))
        //     return true;
        // // cout << l<<endl;
        // return RangeQuery(low, high, p + (1u << (L - l)), l + 1);
    }
    bool doubt(uint64_t p, uint64_t l)
    {
        count++;
        // cout<<p<<"-"<<l<<endl;
        // if(pr) cout<<(p>>L-l+1)<<"-"<<l-1<<endl;
        if (!check(p, l))
            return false;
        // if(pr) cout<<p<<"==="<<l<<endl;
        if (l > L)
            return true;
        if (doubt(p, l + 1))
            return true;
        return doubt(p + (1ll << (L - l)), l + 1);
    }
    inline bool check(uint64_t p, uint64_t l)
    {
        uint64_t key = (p >> L - l + 2) + (l - 2) * 1000000007;
        uint64_t mask = 1;
        return (sbf.querysbm4(key) >> (p >> L - l + 1 & mask)) & 1;
    }
    // bool RangeQuery(uint32_t low, uint32_t high)
    // {
    //     // cout<<low<<" "<<high<<endl;
    //     uint64_t x=sbf.querysbm4((high>>k_tree)+(L - k_tree >> 2) * 1000000007);
    //     // cout<<low<<" "<<high<<endl;
    //     if((low>>k_tree)!=(high>>k_tree)){
    //         x=(x<<(1u<<k_tree))|sbf.querysbm4((low>>k_tree)+(L - k_tree >> 2) * 1000000007);
    //     }
    //     // cout<<low<<" "<<high<<endl;
    //     x=x>>(low&((1u<<k_tree)-1));
    //     return ((x>>high-low+1)<<high-low+1)!=x;
    // }
};

// void InsertPair1(uint32_t plen, uint32_t p)
// {
//     // if(p==10007)
//     // cout<<"insert "<<plen<<" "<<p<<"\t\t";
//     uint32_t level = (plen - 1) & 3;
//     uint32_t mask = ((1u << level) - 1) << (4 - level);
//     // cout<<"insert2 "<<(p>>level)<<" "<<(1u<<((p&((1u<<4)-1))|mask))<<endl;
//     // if(p==10007)
//     //  cout<<"insert2 "<<(p>>level)<<" "<<(1u<<((p&((1u<<4)-1))|mask))<<endl;
//     sbf.insertsbm4((p >> level+1) + (plen - level >> 2) * 1000000007, 1u << ((p & ((1u << 4) - 1)) | mask));
// }

// void Insert1(uint32_t p)
// {
//     for (int i = L; i >= 1; i--)
//     {
//         InsertPair1(i, p);
//         p >>= 1;
//         // p^=p&(1u<<(L-i));
//     }
// }