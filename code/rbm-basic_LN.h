// #include "sBF.h"

using namespace std;

extern int testt;
extern long long lndoubt;
// int RBMQuery[64]={0};
// int RBMFPR[64]={0};
// set<uint64_t> RBMkeySet[64];
// set<uint64_t>::iterator RBMiter;
extern stack<pair<uint64_t,uint64_t> >psr;
extern stack<pair<uint64_t,uint64_t> >psd;
class RBM_LN
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
        count = 0;
        memory = _memory,
        hash_num = _hash_num,
        L = _L;
        if (MAX_BF_NUM < _L)
        {
            cout << "RBM_LN memory initialization fail 1.\n";
            exit(-1);
        }
        sbf.init(_memory, hash_num);
    }
    bool QueryPair1(uint64_t plen, uint64_t p)
    {

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
            testt += rbm;
            p >>= 4;
        }
    }

    bool RangeQuery(uint64_t low, uint64_t high, uint64_t p = 0, uint64_t l = 1)
    {
        bool leafFlag = false;
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
            // for (int i = l + 1; i <= L; i++)
            // {
            //     if (!(low >> L - i & 1))
            //     {
            //         p = (i == 1 ? 0 : low >> L - i + 1 << L - i + 1) + (1ll << L - i);
            //         if (doubt(p, i + 1))
            //         {
            //             return true;
            //         }
            //     }
            // }
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
        lndoubt++;
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