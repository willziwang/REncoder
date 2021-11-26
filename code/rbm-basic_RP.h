// #include "sBF.h"

using namespace std;

extern int testt;
// int RBMQuery[64]={0};
// int RBMFPR[64]={0};
// set<uint64_t> RBMkeySet[64];
// set<uint64_t>::iterator RBMiter;
class RBM_RP
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
            cout << "RBM_RP memory initialization fail 1.\n";
            exit(-1);
        }
        sbf.init(_memory, hash_num);
    }
    bool QueryPair1(uint64_t plen, uint64_t p)
    {

        uint64_t level = (plen - 1) & 3;
        uint64_t mask = 0xF << (level + 2);
        uint32_t res = (sbf.querysbm4((p >> level + 1) + (plen - level -1 >> 2) * 1000000007) >>  (p & (~(1ll << level + 1)) | mask ) ) & 1;
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
    }
    void Insert4(uint64_t p)
    {
        uint32_t rbm = 0;
        uint64_t p0 = p;
        for (int i = 4; i <= L; i += 4)
        {

            uint64_t u = (p & 0x0000000FU) | 0xFFFFFFE0U;
            rbm = 0;
            rbm |= (1u << u);
            rbm |= (1u << (u >> 1));
            rbm |= (1u << (u >> 2));
            rbm |= (1u << (u >> 3));
            // RBMkeySet[67-i].insert(p);
            // RBMkeySet[66-i].insert(p>>1);
            // RBMkeySet[65-i].insert(p>>2);
            // RBMkeySet[64-i].insert(p>>3);
            sbf.insertsbm4((p >> 4) + (L - i >> 2) * 1000000007, rbm);
            testt+=rbm;
            p >>= 4;
        }
    }

    bool RangeQuery(uint64_t low, uint64_t high, uint64_t p = 0, uint64_t l = 1)
    {
        if (p > high || p + (l == 1 ? 0 : 1ll << (L - l + 1)) - 1 < low)
            return false;
        if (p >= low && p + (l == 1 ? 0 : 1ll << (L - l + 1)) - 1 <= high)
            return doubt(p, l);
        uint64_t level = (l - 1) & 3;
        uint64_t mask = 0xF << (level + 2);
        uint32_t prevalue = sbf.querysbm4((L - l + level + 1 == 64 ? 0 : p >> L - l  + level + 1) + (l - level -1 >> 2) * 1000000007);
        uint64_t lqueryFlag = prevalue >> (p >> L - l & (~(1ll << level + 1)) | mask );
        uint64_t rqueryFlag = prevalue >> (p + (1ll << (L - l)) >> L - l & (~(1ll << level + 1)) | mask );
        if ((lqueryFlag & 1)&&RangeQuery(low, high, p, l + 1))
        {
            return true;
        }
        else if(rqueryFlag & 1)
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
        if (!QueryPair1(l - 1, (p >> L - l + 1)))
            return false;
        if (l > L)
            return true;
        if (doubt(p, l + 1))
            return true;
        return doubt(p + (1ll << (L - l)), l + 1);
    }
};