// #include "sBF.h"

using namespace std;

extern int testt;
// int RBMQuery[64]={0};
// int RBMFPR[64]={0};
// set<uint64_t> RBMkeySet[64];
// set<uint64_t>::iterator RBMiter;
class RBM_RP_SIMD
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
    ShiftingBloomfilter_SIMD sbf_simd;
    void init(uint64_t _memory,
              int _hash_num,
              uint64_t _L)
    {
        count = 0;
        memory = _memory,
        hash_num = _hash_num,
        L = _L;
        if (MAX_BF_NUM < _L)
        {
            cout << "RBM_RP memory initialization fail 1.\n";
            exit(-1);
        }
        sbf_simd.init(_memory, hash_num);
    }
    bool QueryPair1(uint64_t plen, uint64_t p)
    {
        uint64_t level = (plen - 1) & 7;
        uint64_t mask = 0xF << (level + 2);
        uint64_t *rbm = sbf_simd.querysbm4((p >> level + 1) + (plen - level - 1 >> 3) * 1000000007);
        uint32_t res;
        if (level < 5)
        {
            res = (rbm[0] >> (p & (~(1ll << level + 1)) | mask)) & 1;
        }
        else
        {
            p = p & ((1u << level + 1) - 1);
            res = (rbm[(1u << level - 4) - 1 - (p >> 6)] >> p) & 1;
        }
        return res;
    }
    void Insert4(uint64_t p)
    {
        uint64_t rbm[8] = {0};
        uint64_t p0 = p;
        for (int i = 8; i <= L; i += 8)
        {
            // uint64_t u = (p & 0x000000FFU) | 0xFFFFFE00U;
            memset(rbm,0,sizeof(rbm));
            uint64_t u = p & 0x000000FFU;
            rbm[7 - u / 64] |= (1ul << u);
            u >>= 1;
            rbm[3 - u / 64] |= (1ul << u);
            u >>= 1;
            rbm[1] |= (1ul << u);
            u >>= 1;
            u |= 0xFFFFFFC0U;
            rbm[0] |= (1ul << u);
            rbm[0] |= (1ul << (u >> 1));
            rbm[0] |= (1ul << (u >> 2));
            rbm[0] |= (1ul << (u >> 3));
            rbm[0] |= (1ul << (u >> 4));
            // RBMkeySet[67-i].insert(p);
            // RBMkeySet[66-i].insert(p>>1);
            // RBMkeySet[65-i].insert(p>>2);
            // RBMkeySet[64-i].insert(p>>3);
            sbf_simd.insertsbm4((p >> 8) + (L - i >> 3) * 1000000007, rbm);
            p >>= 8;
        }
    }

    bool RangeQuery(uint64_t low, uint64_t high, uint64_t p = 0, uint64_t l = 1)
    {
        if (p > high || p + (l == 1 ? 0 : 1ll << (L - l + 1)) - 1 < low)
            return false;
        if (p >= low && p + (l == 1 ? 0 : 1ll << (L - l + 1)) - 1 <= high)
            return doubt(p, l);
        uint64_t level = (l - 1) & 7;
        uint64_t mask = 0xF << (level + 2);
        uint64_t *prevalue = sbf_simd.querysbm4((L - l + level + 1 == 64 ? 0 : p >> L - l + level + 1) + (l - level - 1 >> 3) * 1000000007);
        uint64_t lqueryFlag; 
        uint64_t rqueryFlag;
        if (level < 5)
        {
            lqueryFlag = prevalue[0] >> (p >> L - l & (~(1ll << level + 1)) | mask);
            rqueryFlag = prevalue[0] >> (p + (1ll << (L - l)) >> L - l & (~(1ll << level + 1)) | mask);
        }
        else
        {
            uint64_t z = p >> L - l & ((1u << level + 1) - 1);
            lqueryFlag = (prevalue[(1u << level - 4) - 1 - (z >> 6)] >> z);
            z += 1;
            rqueryFlag = (prevalue[(1u << level - 4) - 1 - (z >> 6)] >> z);
        }
        if ((lqueryFlag & 1) && RangeQuery(low, high, p, l + 1))
        {
            return true;
        }
        else if (rqueryFlag & 1)
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