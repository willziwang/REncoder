// #include "sBF.h"

using namespace std;

// extern int testt;

class RSBF_RN
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
            cout << "RSBF_RN memory initialization fail 1.\n";
            exit(-1);
        }
        sbf.init(_memory, hash_num);
    }
    void Insert4(uint64_t p)
    {
        for (int i = k_tree; i <= L; i += k_tree)
        {
            sbf.insertsbm4((p >> k_tree) + ((L - i) / k_tree) * 1000000007, 1ll << ((p & (1ll << k_tree) - 1)));
            p >>= k_tree;
        }
    }
    bool RangeQuery(uint64_t low, uint64_t high, uint64_t p = 0, uint64_t l = 1)
    { // todo 全范围查询？
        if (p > high || p + (l == 1 ? 0 : 1ll << (L - l * k_tree + k_tree)) - 1 < low)
            return false;
        if (p >= low && p + (l == 1 ? 0 : 1ll << (L - l * k_tree + k_tree)) - 1 <= high)
            return doubt(p, l);
        for (int i = 0; i < (1ll << k_tree) - 1; i++)
        {
            if (RangeQuery(low, high, p + i * (1ll << (L - l * k_tree)), l + 1))
                return true;
        }
        return RangeQuery(low, high, p + ((1ll << k_tree) - 1) * (1ll << (L - l * k_tree)), l + 1);
    }
    bool doubt(uint64_t p, uint64_t l)
    {
        count++;
        if (!check(p, l))
            return false;
        if (l * k_tree > L)
            return true;
        for (int i = 0; i < (1ll << k_tree) - 1; i++)
        {
            if (doubt(p + i * (1ll << (L - l * k_tree)), l + 1))
                return true;
        }
        return doubt(p + ((1ll << k_tree) - 1) * (1ll << (L - l * k_tree)), l + 1);
    }
    inline bool check(const uint64_t p, const uint64_t l)
    {
        uint64_t key = (l <= 2 ? 0 : p >> L - l * k_tree + 2 * k_tree) + (l - 2) * 1000000007;
        uint64_t mask = (1ll << k_tree) - 1;
        return (sbf.querysbm4(key) >> (p >> L - l * k_tree + k_tree & mask)) & 1;
    }
};