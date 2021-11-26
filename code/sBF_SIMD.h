//shifting Bloom filter

#include <cstdint>
extern long long cache_hit;
extern long long query_count;
class ShiftingBloomfilter_SIMD
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
    uint64_t *BFcache4_value;
    BOBHash32 hash[MAX_HASH_NUM];
    vector<size_t> seeds;
    uint8_t array[MAX_HASH_NUM][MAX_BIT_NUM / 8 + 64];


    
    void init(uint64_t _memory, //bit
              int _hash_num)
    {
        memory = _memory,
        hash_num = _hash_num,
        counter_num = ((memory / hash_num) >> 3);

        bit_per_row = counter_num << 3;
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
        seeds.resize(hash_num);
        mt19937 gen(1337);
        for (size_t i = 0; i < hash_num; ++i)
        {
            seeds[i] = gen();
        }
        memset(array, 0, sizeof(array));
    }
    void ROL_m256i(__m256i_u *src, int count)
    {
        __m256i_u left[2], right[2];
        left[0] = _mm256_loadu_si256(src);
        left[1] = _mm256_loadu_si256(src + 1);
        right[0] = _mm256_loadu_si256(src);
        right[1] = _mm256_loadu_si256(src + 1);
        int step = count / 64;
        int shift = count % 64;
        for (int i = 0; i < 8; i++)
        {
            if (i + step > 7)
            {
                left[i / 4][i % 4] = 0;
            }
            else
            {
                left[i / 4][i % 4] = (left[(i + step) / 4][(i + step) % 4] << shift) | (shift == 0 || i + step + 1 > 7 ? 0 : (uint64_t)(left[(i + step + 1) / 4][(i + step + 1) % 4]) >> 64 - shift);
            }
        }
        step = (512 - count) / 64;
        shift = (512 - count) % 64;
        for (int i = 7; i >= 0; i--)
        {
            if (i - step < 0)
            {
                right[i / 4][i % 4] = 0;
            }
            else
            {
                right[i / 4][i % 4] = (uint64_t)(right[(i - step) / 4][(i - step) % 4]) >> shift | (shift == 0 || i - step - 1 < 0 ? 0 : right[(i - step - 1) / 4][(i - step - 1) % 4] << 64 - shift);
            }
        }
        src[0] = _mm256_or_si256(left[0], right[0]);
        src[1] = _mm256_or_si256(left[1], right[1]);
    }

    // 64bits
    // void insertsbm4(uint64_t x, uint64_t *sbm)
    // {
    //     // uint64_t z = (((uint64_t)x)<<32)|y;
    //     // cout<<x<<" "<<y<<" "<<z<<endl;
    //     // cout<<"A";
    //     __m256i_u sbm_new[2], sbm_old[2];
    //     for (int i = 0; i < hash_num; i++)
    //     {
    //         sbm_new[0] = _mm256_loadu_si256((__m256i_u *)(sbm));
    //         sbm_new[1] = _mm256_loadu_si256((__m256i_u *)(sbm + 4));
    //         uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
    //         // uint32_t pos = hash[i].run((char *)&z, sizeof(uint64_t)) % bit_per_row;
    //         uint64_t t = pos >> 9;
    //         sbm_old[0] = _mm256_loadu_si256((__m256i_u *)(array[i] + t * 8));
    //         sbm_old[1] = _mm256_loadu_si256((__m256i_u *)(array[i] + t * 8 + 4));
    //         ROL_m256i(sbm_new, pos & 511);
    //         sbm_old[0] = _mm256_or_si256(sbm_new[0], sbm_old[0]);
    //         sbm_old[1] = _mm256_or_si256(sbm_new[1], sbm_old[1]);
    //         _mm256_storeu_si256((__m256i_u *)(array[i] + t * 8), sbm_old[0]);
    //         _mm256_storeu_si256((__m256i_u *)(array[i] + t * 8 + 4), sbm_old[1]);
    //     }
    // }

    // inline uint64_t *querysbm4(uint64_t x)
    // {
    //     static uint64_t ans[8] = {0};
    //     __m256i_u ans1[2], ans2[2];
    //     query_count++;
    //     if (BFcache4_key == x)
    //     {
    //         cache_hit++;
    //         // cout << "cache\t";
    //         return BFcache4_value;
    //     }
    //     for (int i = 0; i < 1; i++)
    //     {
    //         uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
    //         // uint32_t pos = hash[i].run((char *)&z, sizeof(uint64_t)) % bit_per_row;
    //         uint64_t t = pos >> 9;
    //         ans1[0] = _mm256_loadu_si256((__m256i_u *)(array[i] + t * 8));
    //         ans1[1] = _mm256_loadu_si256((__m256i_u *)(array[i] + t * 8 + 4));
    //         ROL_m256i(ans1, 512 - (pos & 511));
    //     }
    //     for (int i = 1; i < hash_num; i++)
    //     {
    //         uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
    //         // uint32_t pos = hash[i].run((char *)&z, sizeof(uint64_t)) % bit_per_row;
    //         uint64_t t = pos >> 9;
    //         ans2[0] = _mm256_loadu_si256((__m256i_u *)(array[i] + t * 8));
    //         ans2[1] = _mm256_loadu_si256((__m256i_u *)(array[i] + t * 8 + 4));
    //         ROL_m256i(ans2, 512 - (pos & 511));
    //         ans1[0] = _mm256_and_si256(ans1[0], ans2[0]);
    //         ans1[1] = _mm256_and_si256(ans1[1], ans2[1]);
    //     }
    //     BFcache4_key = x;
    //     _mm256_storeu_si256((__m256i_u *)(ans), ans1[0]);
    //     _mm256_storeu_si256((__m256i_u *)(ans + 4), ans1[1]);
    //     BFcache4_value = ans;
    //     return ans;
    // }

    void insertsbm4(uint64_t x, uint64_t *sbm)
    {
        // uint64_t z = (((uint64_t)x)<<32)|y;
        // cout<<x<<" "<<y<<" "<<z<<endl;
        // cout<<"A";
        __m512i_u sbm_new, sbm_old;
        sbm_new = _mm512_loadu_si512((__m512i_u *)(sbm));
        for (int i = 0; i < hash_num; i++)
        {
            uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
            // uint64_t pos = 0;
            // MurmurHash3_x86_32((char *)&x, 8, seeds[i], &pos);
            // pos %= bit_per_row;
            uint64_t t = pos >> 3;
            sbm_old = _mm512_loadu_si512((__m512i_u *)(array[i] + t));
            sbm_old = _mm512_or_si512(sbm_new, sbm_old);
            _mm512_storeu_si512((__m512i_u *)(array[i] + t), sbm_old);
        }
    }

    inline uint64_t *querysbm4(uint64_t x)
    {
        static uint64_t ans[8] = {0};
        __m512i_u ans1, ans2;
        query_count++;
        if (BFcache4_key == x)
        {
            cache_hit++;
            // cout << "cache\t";
            return BFcache4_value;
        }
        for (int i = 0; i < 1; i++)
        {
            uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
            // uint64_t pos = 0;
            // MurmurHash3_x86_32((char *)&x, 8, seeds[i], &pos);
            // pos %= bit_per_row;
            uint64_t t = pos >> 3;
            ans1 = _mm512_loadu_si512((__m512i_u *)(array[i] + t));
        }
        for (int i = 1; i < hash_num; i++)
        {
            uint64_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
            // uint64_t pos = 0;
            // MurmurHash3_x86_32((char *)&x, 8, seeds[i], &pos);
            // pos %= bit_per_row;
            uint64_t t = pos >> 3;
            ans2 = _mm512_loadu_si512((__m512i_u *)(array[i] + t));
            ans1 = _mm512_and_si512(ans1, ans2);
        }
        BFcache4_key = x;
        _mm512_storeu_si512((__m512i_u *)(ans), ans1);
        BFcache4_value = ans;
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
