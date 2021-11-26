//shifting Bloom filter

#include <cstdint>
extern long long cache_hit;
extern long long query_count;
class ShiftingBloomfilter
{
public:
    static const uint64_t
        MAX_BIT_NUM = 1ll << 34,
        MAX_HASH_NUM = 8;
    uint64_t
        hash_num, //number of hash function
        counter_num,
        bit_per_row;
    uint64_t memory;
    uint64_t BFcache4_key = -1;
    uint32_t BFcache4_value;
    BOBHash32 hash[MAX_HASH_NUM];
    uint32_t array[MAX_HASH_NUM][MAX_BIT_NUM / 32];

    void init(uint64_t _memory, //bit
              int _hash_num)
    {
        memory = _memory,
        hash_num = _hash_num,
        counter_num = ((memory / hash_num) >> 5);
        bit_per_row = counter_num << 5;
        // cout<<bit_per_row<<"___"<<counter_num<<"___"<<hash_num<<endl;
        if (bit_per_row > MAX_BIT_NUM)
        {
            cout << "sBF memory initialization fail 1.\n";
            exit(-1);
        }
        if (hash_num > MAX_HASH_NUM)
        {
            cout << "sBF memory initialization fail 2.\n";
            exit(-1);
        }

        for (int i = 0; i < hash_num; i++)
        {
            hash[i].initialize(rand() % MAX_PRIME32);
        }
        memset(array, 0, sizeof(array));
    }

    void insertsbm4(uint64_t x, uint32_t sbm)
    {
        // uint64_t z = (((uint64_t)x)<<32)|y;
        // cout<<x<<" "<<y<<" "<<z<<endl;
        // cout<<"A";
        for (int i = 0; i < hash_num; i++)
        {
            uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
            // uint32_t pos = hash[i].run((char *)&z, sizeof(uint64_t)) % bit_per_row;
            uint64_t t = pos >> 5;
            array[i][t] |= sbm << (pos & 31);
            array[i][t] |= ((pos & 31) == 0 ? 0 : sbm >> (32 - (pos & 31)));
            t = 0;
            // array[i][t] |= sbm;
        }
    }

    inline uint32_t querysbm4(uint64_t x)
    {
        query_count++;
        if (BFcache4_key == x)
        {
            cache_hit++;
            // cout << "cache\t";
            return BFcache4_value;
        }
        uint32_t ans = 0;
        for (int i = 0; i < 1; i++)
        {
            uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
            // uint32_t pos = hash[i].run((char *)&z, sizeof(uint64_t)) % bit_per_row;
            uint64_t t = pos >> 5;
            ans = array[i][t] >> (pos & 31) | ((pos & 31) == 0 ? 0 : array[i][t] << (32 - (pos & 31)));
            // ans = array[i][t];
        }
        for (int i = 1; i < hash_num; i++)
        {
            uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
            // uint32_t pos = hash[i].run((char *)&z, sizeof(uint64_t)) % bit_per_row;
            uint64_t t = pos >> 5;
            ans &= array[i][t] >> (pos & 31) | ((pos & 31) == 0 ? 0 : array[i][t] << (32 - (pos & 31)));
            // ans &= array[i][t];
        }
        BFcache4_key = x, BFcache4_value = ans;
        return ans;
    }


    void insertsbm(uint32_t x, uint32_t *sbm, uint32_t intnum = 16)
    {
        for (int i = 0; i < hash_num; i++)
        {
            uint32_t pos = hash[i].run((char *)&x, sizeof(uint32_t)) % bit_per_row;
            int t = pos >> 5;
            for (int j = 0; j < intnum; j++)
            {
                array[i][t] |= *sbm;
                t++;
                if (t == counter_num)
                    t = 0;
                sbm++;
            }
        }
    }

    void querysbm(uint32_t x, uint32_t *sbm, uint32_t intnum = 16)
    {
        uint32_t ans = 0;
        for (int i = 0; i < 1; i++)
        {
            uint32_t pos = hash[i].run((char *)&x, sizeof(uint32_t)) % bit_per_row;
            int t = pos >> 5;
            for (int j = 0; j < intnum; j++)
            {
                *sbm = array[i][t];
                t++;
                if (t == counter_num)
                    t = 0;
                sbm++;
            }
        }
        for (int i = 1; i < hash_num; i++)
        {
            uint32_t pos = hash[i].run((char *)&x, sizeof(uint32_t)) % bit_per_row;
            int t = pos >> 5;
            for (int j = 0; j < intnum; j++)
            {
                *sbm &= array[i][t];
                t++;
                if (t == counter_num)
                    t = 0;
                sbm++;
            }
        }
    }
};
