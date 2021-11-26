// #include "sBF.h"

using namespace std;

extern int testt;
// int RBMQuery[64]={0};
// int RBMFPR[64]={0};
// set<uint64_t> RBMkeySet[64];
// set<uint64_t>::iterator RBMiter;
extern stack<pair<uint64_t, uint64_t>> psr;
extern stack<pair<uint64_t, uint64_t>> psd;
class RBM_LP
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
    uint64_t query_level;
    void init(uint64_t _memory,
              int _hash_num,
              uint64_t _L,
              uint64_t _query_level = 64)
    {
        count = 0;
        memory = _memory,
        hash_num = _hash_num,
        L = _L;
        query_level = _query_level;
        if (MAX_BF_NUM < _L)
        {
            cout << "RBM_LN memory initialization fail 1.\n";
            exit(-1);
        }
        sbf.init(_memory, hash_num);
    }
    void setQueryLevel(uint64_t _query_level)
    {
        query_level = _query_level;
    }
    bool QueryPair1(uint64_t plen, uint64_t p)
    {
        if (L - plen >= query_level)
        {
            return true;
        }
        uint64_t level = (plen - 1) & 3;
        uint64_t mask = 0xF << (level + 2);
        uint32_t res = (sbf.querysbm4((p >> level + 1) + (plen - level - 1 >> 2) * 1000000007) >> (p & (~(1ll << level + 1)) | mask)) & 1;
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
        uint64_t level = query_level;
        uint32_t rbm = 0;
        uint64_t p0 = p;
        for (int i = 4; i <= L; i += 4)
        {
            rbm = 0;
            uint64_t u = (p & 0x0000000FU) | 0xFFFFFFE0U;
            if (level <= 4)
            {
                rbm |= level >= 1 ? (1u << u) : 0;
                rbm |= level >= 2 ? (1u << (u >> 1)) : 0;
                rbm |= level >= 3 ? (1u << (u >> 2)) : 0;
                rbm |= level >= 4 ? (1u << (u >> 3)) : 0;
                // RBMkeySet[67-i].insert(p);
                // RBMkeySet[66-i].insert(p>>1);
                // RBMkeySet[65-i].insert(p>>2);
                // RBMkeySet[64-i].insert(p>>3);
                sbf.insertsbm4((p >> 4) + (L - i >> 2) * 1000000007, rbm);
                break;
            }
            else
            {
                rbm |= (1u << u);
                rbm |= (1u << (u >> 1));
                rbm |= (1u << (u >> 2));
                rbm |= (1u << (u >> 3));
            }
            level -= 4;
            sbf.insertsbm4((p >> 4) + (L - i >> 2) * 1000000007, rbm);
            p >>= 4;
        }
    }

    bool RangeQuery(uint64_t low, uint64_t high, uint64_t p = 0, uint64_t l = 1)
    {
        bool leftBorder = low == 0;
        bool rightBorder = high + 1 == 0;
        if (!leftBorder)
        {
            low -= 1;
        }
        if (!rightBorder)
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
        for (int i = 2; i <= l; i++)
        {
            if (!QueryPair1(i - 1, (low >> L - i + 1)))
            {
                return false;
            }
        }
        if (leftBorder && rightBorder)
        {
            if (doubt(0, l + 1))
            {
                return true;
            }
            return doubt(1ll << (L - l), l + 1);
        }
        else if (rightBorder)
        {
            for (int i = l; i <= L; i++)
            {
                if (i != l && !QueryPair1(i - 1, low >> L - i + 1))
                {
                    return false;
                }
                if (!(low >> L - i & 1))
                {
                    p = (i == 1 ? 0 : low >> L - i + 1 << L - i + 1) + (1ll << L - i);
                    if (doubt(p, i + 1))
                    {
                        return true;
                    }
                }
                if (low + 1 >> L - i != low >> L - i)
                {
                    break;
                }
            }
        }
        else if (leftBorder)
        {
            for (int i = l; i <= L; i++)
            {
                if (i != l && !QueryPair1(i - 1, high >> L - i + 1))
                {
                    return false;
                }
                if (high >> L - i & 1)
                {
                    p = i == 1 ? 0 : high >> L - i + 1 << L - i + 1;
                    if (doubt(p, i + 1))
                    {
                        return true;
                    }
                }
                if (high >> L - i << L - i == high)
                {
                    break;
                }
            }
        }
        else
        {
            for (int i = l + 1; i <= L; i++)
            {
                if (!QueryPair1(i - 1, low >> L - i + 1))
                {
                    break;
                }
                if (!(low >> L - i & 1))
                {
                    p = (i == 1 ? 0 : low >> L - i + 1 << L - i + 1) + (1ll << L - i);
                    if (doubt(p, i + 1))
                    {
                        return true;
                    }
                }
                if (low + 1 >> L - i != low >> L - i)
                {
                    break;
                }
            }
            for (int i = l + 1; i <= L; i++)
            {
                if (!QueryPair1(i - 1, high >> L - i + 1))
                {
                    break;
                }
                if (high >> L - i & 1)
                {
                    p = i == 1 ? 0 : high >> L - i + 1 << L - i + 1;
                    if (doubt(p, i + 1))
                    {
                        return true;
                    }
                }
                if (high >> L - i << L - i == high)
                {
                    break;
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
        //     uint64_t level = (l - 1) & 3;
        //     uint64_t mask = 0xF << (level + 2);
        //     uint32_t prevalue = sbf.querysbm4((L - l + level + 1 == 64 ? 0 : p >> L - l  + level + 1) + (l - level -1 >> 2) * 1000000007);
        //     uint64_t lqueryFlag = prevalue >> (p >> L - l & (~(1ll << level + 1)) | mask );
        //     uint64_t rqueryFlag = prevalue >> (p + (1ll << (L - l)) >> L - l & (~(1ll << level + 1)) | mask );
        //     if(rqueryFlag & 1)
        //     {
        //         psr.push(make_pair(p + (1ll << (L - l)),l+1));
        //     }
        //     if(lqueryFlag & 1)
        //     {
        //         psr.push(make_pair(p,l+1));
        //     }
        // }
        // return false;
    }
    bool doubt(uint64_t p, uint64_t l)
    {
        while (!psd.empty())
        {
            psd.pop();
        }
        psd.push(make_pair(p, l));
        while (!psd.empty())
        {
            p = psd.top().first;
            l = psd.top().second;
            psd.pop();
            if (!QueryPair1(l - 1, (p >> L - l + 1)))
            {
                continue;
            }
            if (l > L)
            {
                return true;
            }
            psd.push(make_pair(p + (1ll << (L - l)), l + 1));
            psd.push(make_pair(p, l + 1));
        }
        return false;
    }
};