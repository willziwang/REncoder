// #include "sBF.h"

using namespace std;

extern int testt;
// int RBMQuery[64]={0};
// int RBMFPR[64]={0};
// set<uint64_t> RBMkeySet[64];
// set<uint64_t>::iterator RBMiter;
extern stack<pair<uint64_t, uint64_t>> psr;
extern stack<pair<uint64_t, uint64_t>> psd;
class RBM_LP_SIMD
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
    int query_level;
    void init(uint64_t _memory,
              int _hash_num,
              uint64_t _L,
              int _query_level = 64)
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
        sbf_simd.init(_memory, hash_num);
    }
    void setQueryLevel(int _query_level)
    {
        query_level = _query_level;
    }
    bool QueryPair1(uint64_t plen, uint64_t p)
    {
        if (L - plen >= query_level)
        {
            return true;
        }
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
    int Insert4(vector<uint64_t> keys, int step)
    {
        int num = keys.size();
        double onerate = 0;
        double pre_onerate = 0;
        int start_level = 1;
        while (true)
        {
            query_level = start_level + step;
            for (int i = 0; i < num; i++)
            {
                Insert4(keys[i], start_level);
            }
            for (int i = 0; i < sbf_simd.counter_num; i++)
            {
                uint64_t array = sbf_simd.array[0][i];
                for (int j = 0; j < 8; j++)
                {
                    onerate += (array & 1);
                    array >>= 1;
                }
            }
            onerate /= sbf_simd.bit_per_row;
            if (0.5 - onerate < 0.065 || onerate >= 0.5 || query_level==65)
            {
                break;
            }
            start_level = query_level;
            pre_onerate = onerate;
            onerate = 0;
        }
        query_level -= 1;
        return query_level;
    }
    void Insert4(uint64_t p, uint64_t start_level)
    {
        uint64_t rbm[8] = {0};
        uint64_t p0 = p;
        p >>= (start_level - 1) / 8 * 8;
        int si = (start_level - 1) / 8 * 8 + 8;
        int ei = (query_level - 2) / 8 * 8 + 8;
        for (int i = si; i <= ei; i += 8)
        {
            int sl = start_level - i + 8 > 0 ? start_level - i + 8 : 1;
            int el = query_level - 1 - i + 8;
            memset(rbm, 0, sizeof(rbm));
            uint64_t u = p & 0x000000FFU;
            rbm[7 - u / 64] |= sl <= 1 && el >= 1 ? (1ul << u) : 0;
            u >>= 1;
            rbm[3 - u / 64] |= sl <= 2 && el >= 2 ? (1ul << u) : 0;
            u >>= 1;
            rbm[1] |= sl <= 3 && el >= 3 ? (1ul << u) : 0;
            u >>= 1;
            u |= 0xFFFFFFC0U;
            rbm[0] |= sl <= 4 && el >= 4 ? (1ul << u) : 0;
            rbm[0] |= sl <= 5 && el >= 5 ? (1ul << (u >> 1)) : 0;
            rbm[0] |= sl <= 6 && el >= 6 ? (1ul << (u >> 2)) : 0;
            rbm[0] |= sl <= 7 && el >= 7 ? (1ul << (u >> 3)) : 0;
            rbm[0] |= sl <= 8 && el >= 8 ? (1ul << (u >> 4)) : 0;
            sbf_simd.insertsbm4((p >> 8) + (L - i >> 3) * 1000000007, rbm);
            p >>= 8;
        }
    }
    void Insert4(uint64_t p)
    {
        uint64_t level = query_level;
        uint64_t rbm[8] = {0};
        uint64_t p0 = p;
        for (int i = 8; i <= L; i += 8)
        {
            // uint64_t u = (p & 0x000000FFU) | 0xFFFFFE00U;
            memset(rbm, 0, sizeof(rbm));
            uint64_t u = p & 0x000000FFU;
            if (level <= 8)
            {
                rbm[7 - u / 64] |= level >= 1 ? (1ul << u) : 0;
                u >>= 1;
                rbm[3 - u / 64] |= level >= 2 ? (1ul << u) : 0;
                u >>= 1;
                rbm[1] |= level >= 3 ? (1ul << u) : 0;
                u >>= 1;
                u |= 0xFFFFFFC0U;
                rbm[0] |= level >= 4 ? (1ul << u) : 0;
                rbm[0] |= level >= 5 ? (1ul << (u >> 1)) : 0;
                rbm[0] |= level >= 6 ? (1ul << (u >> 2)) : 0;
                rbm[0] |= level >= 7 ? (1ul << (u >> 3)) : 0;
                rbm[0] |= level >= 8 ? (1ul << (u >> 4)) : 0;
                sbf_simd.insertsbm4((p >> 8) + (L - i >> 3) * 1000000007, rbm);
                break;
            }
            else
            {
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
            }
            // RBMkeySet[67-i].insert(p);
            // RBMkeySet[66-i].insert(p>>1);
            // RBMkeySet[65-i].insert(p>>2);
            // RBMkeySet[64-i].insert(p>>3);
            level -= 8;
            sbf_simd.insertsbm4((p >> 8) + (L - i >> 3) * 1000000007, rbm);
            p >>= 8;
        }
    }

    bool RangeQuery(uint64_t low, uint64_t high, uint64_t p = 0, uint64_t l = 1)
    {
        if (low == high)
        {
            for (int i = 2; i <= L + 1; i++)
            {
                if (!QueryPair1(i - 1, (low >> L - i + 1)))
                {
                    return false;
                }
            }
            return true;
        }
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