// #include "sBF.h"

using namespace std;

// extern int testt;

class RSBF
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
              int _k_tree,
              uint64_t _L)
    {
        count = 0;
        memory = _memory,
        hash_num = _hash_num,
        k_tree = _k_tree,
        L = _L;
        if (MAX_BF_NUM < _L)
        {
            cout << "RSBF memory initialization fail 1.\n";
            exit(-1);
        }
        sbf.init(_memory, hash_num);
    }
    void Insert4(uint64_t p)
    {
        for (int i = k_tree; i <= L; i += k_tree)
        {
            sbf.insertsbm4((p >> k_tree) + ((L - i) / k_tree) * 1000000007, 1u << ((p & (1u << k_tree) - 1)));
            p >>= k_tree;
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
        uint64_t diff = low ^ high;
        uint64_t difflen;
        bool leafFlag = false;
        for (difflen = 0; difflen <= L; difflen += k_tree)
        {
            if(diff==0)
            {
                break;
            }
            diff>>=k_tree;
        }
        if(difflen == 0)
            return check(low,L/k_tree+1);
        difflen -= k_tree;
        if(difflen == 0)
            leafFlag = true;
        l = (L - difflen)/k_tree + 1;
        uint32_t prevalue = sbf.querysbm4((difflen + k_tree == 64 ? 0 : low >> difflen + k_tree) + (L - difflen - k_tree)/k_tree * 1000000007);
        uint64_t lowIndex=low>>difflen&(1u<<k_tree)-1;
        uint64_t highIndex=high>>difflen&(1u<<k_tree)-1;
        uint64_t doubtFlag = prevalue >> lowIndex + 1;
        uint64_t lqueryFlag = prevalue >> lowIndex;
        uint64_t rqueryFlag = prevalue >> highIndex;
        p = (low>>difflen) + 1 << difflen;
        for (int i = lowIndex + 1; i < highIndex; i++)
        {
            if (doubtFlag & 1)
            {
                if(leafFlag || doubt(p,l))
                    return true;
            }
            p += 1<<difflen;
            doubtFlag >>= 1;
        }
        prevalue >>= lowIndex;
        if (lqueryFlag & 1)
        {
            if (leafFlag || RangeQuery(low, low | (1ll<<difflen)-1, p , l))
                return true;
        }
        if (rqueryFlag & 1)
        {
            if (leafFlag || RangeQuery(high >> difflen << difflen, high, p , l))
                return true;
        }
        return false;
        // for (int i = 0; i < (1u << k_tree) - 1; i++)
        // {
        //     if (RangeQuery(low, high, p + i * (1u << (L - l * k_tree)), l + 1))
        //         return true;
        // }
        // cout << l<<endl;
        // return RangeQuery(low, high, p + ((1u << k_tree) - 1) * (1u << (L - l * k_tree)), l + 1);
    }
    bool doubt(uint64_t p, uint64_t l)
    {
        count++;
        // cout<<p<<"-"<<l<<endl;
        // if(pr) cout<<(p>>L-l+1)<<"-"<<l-1<<endl;
        // if(pr) cout<<p<<"==="<<l<<endl;
        if (l * k_tree > L)
            return true;
        uint32_t prevalue = sbf.querysbm4((L - l * k_tree + k_tree == 64 ? 0 : p >> L - l * k_tree + k_tree) + (l - 1) * 1000000007);
        for (int i = 0; i < (1u << k_tree); i++)
        {
            if (prevalue & 1)
            {
                if (doubt(p + i * (1ll << (L - l * k_tree)), l + 1))
                    return true;
            }
            prevalue >>= 1;
        }
        return false;
        // for (int i = 0; i < (1u << k_tree) - 1; i++)
        // {
        //     if (doubt(p + i * (1u << (L - l * k_tree)), l + 1))
        //         return true;
        // }
        // return doubt(p + ((1u << k_tree) - 1) * (1u << (L - l * k_tree)), l + 1);
    }
    inline bool check(const uint64_t p, const uint64_t l) //todo -O2
    {
        uint64_t key = (p >> L - l * k_tree + 2 * k_tree) + (l - 2) * 1000000007;
        uint64_t mask = (1u << k_tree) - 1;
        return (sbf.querysbm4(key) >> (p >> L - l * k_tree + k_tree & mask)) & 1;
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