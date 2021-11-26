// #include "sBF.h"

using namespace std;

// extern int testt;
extern stack<pair<uint64_t,uint64_t> >psr;
extern stack<pair<uint64_t,uint64_t> >psd;
class RSBF_LP
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
            cout << "RSBF_LP memory initialization fail 1.\n";
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
        while(!psr.empty())
        {
            psr.pop();
        }
        psr.push(make_pair(p,l));
        while(!psr.empty())
        {
            p = psr.top().first;
            l = psr.top().second;
            psr.pop();
            if (p > high || p + (l == 1 ? 0 : 1ll << (L - l * k_tree + k_tree)) - 1 < low)
                continue;
            if (p >= low && p + (l == 1 ? 0 : 1ll << (L - l * k_tree + k_tree)) - 1 <= high)
            {
                if(doubt(p, l))
                {
                    return true;
                }
                else
                {
                    continue;
                }       
            }
            uint32_t prevalue = sbf.querysbm4((l == 1 ? 0 : p >> L - l * k_tree +  k_tree) + (l - 1) * 1000000007);
            for (int i = (1ll << k_tree) - 1; i>=0; i--)
            {
                if (prevalue & (1ll << i))
                {
                    psr.push(make_pair(p + i * (1ll << (L - l * k_tree)),l+1));
                }
            }
        }
        return false;
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
            if (!check(p, l))
            {
                continue;
            }
            if (l * k_tree > L)
            {
                return true;
            }
            for (int i = (1ll << k_tree) - 1; i>=0; i--)
            {
                psr.push(make_pair(p + i * (1ll << (L - l * k_tree)),l+1));
            }
        }
        return false;
    }
    inline bool check(const uint64_t p, const uint64_t l)
    {
        uint64_t key = (l <= 2 ? 0 : p >> L - l * k_tree + 2 * k_tree) + (l - 2) * 1000000007;
        uint64_t mask = (1ll << k_tree) - 1;
        return (sbf.querysbm4(key) >> (p >> L - l * k_tree + k_tree & mask)) & 1;
    }
};