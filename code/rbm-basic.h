// #include "sBF.h"

using namespace std;

extern int testt;
int RBMQuery[64]={0};
int RBMFPR[64]={0};
set<uint64_t> RBMkeySet[64];
set<uint64_t>::iterator RBMiter;
class RBM
{
public:
    static const int
        MAX_BF_NUM = 64;
    int
        count,
        hash_num;
    uint64_t memory;
    int k_tree = 4;
    uint64_t L;
    ShiftingBloomfilter sbf;
    void init(uint64_t _memory,
              int _hash_num,
              uint64_t _L)
    {
        count=0;
        memory = _memory,
        hash_num = _hash_num,
        L = _L;
        if (MAX_BF_NUM < _L)
        {
            cout << "Rosetta memory initialization fail 1.\n";
            exit(-1);
        }
        sbf.init(_memory, hash_num);
    }
    bool QueryPair1(uint64_t plen, uint64_t p)
    {

        uint64_t level = (plen - 1) & 3;
        uint64_t mask = 0xF << (level + 2);
        uint32_t res = (sbf.querysbm4((p >> level + 1) + (plen - level -1 >> 2) * 1000000007) >>  (p & (~(1u << level + 1)) | mask ) ) & 1;
        // RBMQuery[63]++;
        // if (res == 1)
        // {
        //     if((RBMiter=RBMkeySet[63].find(p))==RBMkeySet[63].end())
        //     {
        //         RBMFPR[63]++;
        //     }
        //     else
        //     {
        //         RBMQuery[63]--;
        //     }
        // }
        return res;
        // cout<<(p >> level + 1) + (plen - level >> 2) * 1000000007<<endl;
        // return (sbf.querysbm4((p >> level + 1) + (plen - level -1 >> 2) * 1000000007) >>  (p & (~(1u << level + 1)) | mask ) ) & 1;
            //plen-level-1? todo
        // cout<<"insert "<<plen<<" "<<p<<"\t\t";
        // uint32_t level = (plen - 1) & 3;
        // uint32_t mask = ((1u << 3 - level) - 1) << (level + 2);

        // return (sbf.querysbm4((p >> level + 1) + (plen - level >> 2) * 1000000007) >> ((p & ((1u << level + 1) - 1)) | mask)) & 1;
    }
    void Insert4(uint64_t p)
    {
        uint32_t rbm = 0;
        uint64_t p0 = p;
        for (int i = 4; i <= L; i += 4)
        {

            uint64_t u = (p & 0x0000000FU) | 0xFFFFFFE0U;
            //todo
            rbm = 0;
            rbm |= (1u << u);
            rbm |= (1u << (u >> 1));
            rbm |= (1u << (u >> 2));
            rbm |= (1u << (u >> 3));
            // RBMkeySet[67-i].insert(p);
            // RBMkeySet[66-i].insert(p>>1);
            // RBMkeySet[65-i].insert(p>>2);
            // RBMkeySet[64-i].insert(p>>3);
            //todo
            // cout << "p :" << (p & 0x1F) << endl;
            // cout << "|" << (u & 0x1F) << "|";
            // cout << "|" << ((u >> 1) & 0x1F) << "|";
            // cout << "|" << ((u >> 2) & 0x1F) << "|";
            // cout << "|" << ((u >> 3) & 0x1F) << "|";
            // cout << "+++" << (p >> 4) << " " << (L - i >> 2) << "\t\t" << rbm << endl;
            sbf.insertsbm4((p >> 4) + (L - i >> 2) * 1000000007, rbm);
            testt+=rbm;
            p >>= 4;
        }

        // p = p0;

        // for (int i = L; i >= 1; i--)
        // {
        //     uint32_t plen = i;
        //     uint32_t p = p;
        //     uint32_t level = (plen - 1) & 3;
        //     uint32_t mask = ((1u << 3 - level) - 1) << (level + 2);
        //     rbm |= (1u << ((p & ((1u << level + 1) - 1)) | mask));
        //     cout << "|" << (p & ((1u << level + 1) - 1)) << " " << mask << "|";
        //     if (((plen - 1) & 3) == 0)
        //     {
        //         cout << "---" << (p >> level + 1) << " " << (plen - level >> 2) << "\t\t" << rbm << endl;
        //         sbf.insertsbm4((p >> level + 1) + (plen - level >> 2) * 1000000007, rbm);
        //         rbm = 0;
        //     }
        //     // InsertPair1(i, p);
        //     p >>= 1;
        //     // p^=p&(1u<<(L-i));
        // }
    }

    bool RangeQuery(uint64_t low, uint64_t high, uint64_t p = 0, uint64_t l = 1)
    {
        // 16叉
        // uint32_t diff = low ^ high;
        // uint32_t difflen;
        // bool leafFlag = false;
        // for (difflen = 0; difflen <= L; difflen += k_tree)
        // {
        //     if(diff==0)
        //     {
        //         break;
        //     }
        //     diff>>=k_tree;
        // }
        // if(difflen == 0)
        //     return QueryPair1(L,low);
        // difflen -= k_tree;
        // if(difflen == 0)
        //     leafFlag = true;
        // l = (L - difflen) / k_tree + 1;
        // uint32_t plen = L - difflen;
        // uint32_t level = (plen - 1) & 3;
        // uint32_t mask = 0xF << (level + 2);
        // uint32_t prevalue = sbf.querysbm4((low >> difflen + level + 1) + (plen - level -1 >> 2) * 1000000007);
        // uint32_t lowIndex=low>>difflen&(1u<<k_tree)-1;
        // uint32_t highIndex=high>>difflen&(1u<<k_tree)-1;
        // uint32_t doubtFlag = prevalue >> lowIndex + 1;
        // uint32_t lqueryFlag = prevalue >> lowIndex;
        // uint32_t rqueryFlag = prevalue >> highIndex;
        // p = (low>>difflen) + 1 << difflen;
        // for (int i = lowIndex + 1; i < highIndex; i++)
        // {
        //     if (doubtFlag & 1)
        //     {
        //         if(leafFlag || doubt(p,l))
        //             return true;
        //     }
        //     p += 1<<difflen;
        //     doubtFlag >>= 1;
        // }
        // prevalue >>= lowIndex;
        // if (lqueryFlag & 1)
        // {
        //     if (leafFlag || RangeQuery(low, low | (1u<<difflen)-1, p , l))
        //         return true;
        // }
        // if (rqueryFlag & 1)
        // {
        //     if (leafFlag || RangeQuery(high >> difflen << difflen, high, p , l))
        //         return true;
        // }
        // return false;

        // 2叉
        uint64_t diff = low ^ high;
        uint64_t difflen;
        bool leafFlag = false;
        for (difflen = 0; difflen <= L; difflen += 1)
        {
            if(diff==0)
            {
                break;
            }
            diff>>=1;
        }
        if(difflen == 0)
            return QueryPair1(L,low);
        difflen -= 1;
        if(difflen == 0)
            leafFlag = true;
        l = (L - difflen) + 1;
        uint64_t plen = l - 1;
        uint64_t level = (plen - 1) & 3;
        uint64_t mask = 0xF << (level + 2);
        uint32_t prevalue = sbf.querysbm4((difflen + level + 1 == 64 ? 0 : low >> difflen + level + 1) + (plen - level -1 >> 2) * 1000000007);
        uint64_t lqueryFlag = prevalue >> (low >> difflen & (~(1u << level + 1)) | mask );
        uint64_t rqueryFlag = prevalue >> (high >> difflen & (~(1u << level + 1)) | mask );
        // RBMQuery[l-2]++;
        if (lqueryFlag & 1)
        {
            // if((RBMiter=RBMkeySet[l-2].find(low>>L-l+1))==RBMkeySet[l-2].end())
            // {
            //     RBMFPR[l-2]++;
            // }
            // else
            // {
            //     RBMQuery[l-2]--;
            // }
            if (leafFlag || RangeQuery(low, low | (1ll<<difflen)-1, p , l))
                return true;
        }
        // RBMQuery[l-2]++;
        if (rqueryFlag & 1)
        {
            // if((RBMiter=RBMkeySet[l-2].find(low>>L-l+1))==RBMkeySet[l-2].end())
            // {
            //     RBMFPR[l-2]++;
            // }
            // else
            // {
            //     RBMQuery[l-2]--;
            // }
            if (leafFlag || RangeQuery(high >> difflen << difflen, high, p , l))
                return true;
        }
        return false;

        
        // cout << l<<endl;
        // cout<<p<<"_"<< p + (l == 1 ? 0 : 1u << (L - l + 1)) - 1 <<endl;
        // if (p > high || p + (l == 1 ? 0 : 1u << (L - l + 1)) - 1 < low)
        //     return false;
        // if (p >= low && p + (l == 1 ? 0 : 1u << (L - l + 1)) - 1 <= high)
        //     return doubt(p, l);
        // // cout << l<<endl;
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
    }
    // bool doubt(uint32_t p, uint32_t l)
    // {
    //     count++;
    //     // cout<<p<<"-"<<l<<endl;
    //     // cout<<(p>>L-l+1)<<"-"<<l-1<<endl;

    //     // if (!bf[l - 2].query(p>>L-l+1))
    //     // return false;
    //     // return doubt2((p >> L - l + 1), l - 1);
    //     if (!QueryPair1(l - 1, (p >> L - l + 1)))
    //         return false;
    //     // cout<<p<<"==="<<l<<endl;
    //     if (l > L)
    //         return true;
    //     if (doubt(p, l + 1))
    //         return true;
    //     return doubt(p + (1u << (L - l)), l + 1);
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