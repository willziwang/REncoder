//Bloom filter

#include <iostream>
using namespace std;
extern long long bfQuery;
extern long long query_count;
class Bloomfilter
{
public:
    static const uint64_t
            MAX_BIT_NUM = 1ll << 29,
            MAX_HASH_NUM = 20;
    uint64_t
            hash_num, //number of hash function
            counter_num,
            bit_per_row;
    uint64_t memory;

    BOBHash32 hash[MAX_HASH_NUM];
    vector<size_t> seeds;
    uint32_t array[MAX_HASH_NUM][MAX_BIT_NUM/32];

    void init(uint64_t _memory, //bit
              int _hash_num)
    {
        memory = _memory,
        hash_num = _hash_num,
        counter_num = ((memory / hash_num) >> 5);
        bit_per_row = counter_num << 5;

        if(bit_per_row > MAX_BIT_NUM){
            cout<<"BF memory initialization fail 1.\n";
            exit(-1);
        }
        if(hash_num > MAX_HASH_NUM){
            cout<<"BF memory initialization fail 2.\n";
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

    void insert(uint64_t x)
    {
        for (int i = 0; i < hash_num; i++)
        {
            // uint32_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
            uint64_t pos = 0;
            MurmurHash3_x86_32((char *)&x, 8, seeds[i], &pos);
            pos %= bit_per_row;
            array[i][pos >> 5] |= (1u << (pos & 31));
        }
    }

    bool query(uint64_t x)
    {
        bfQuery++;
        // query_count++;
        for (int i = 0; i < hash_num; i++)
        {
            // uint32_t pos = hash[i].run((char *)&x, sizeof(uint64_t)) % bit_per_row;
            uint64_t pos = 0;
            MurmurHash3_x86_32((char *)&x, 8, seeds[i], &pos);
            pos %= bit_per_row;
            if (!((array[i][pos >> 5] >> (pos & 31)) & 1))
                return false;
        }
        return true;
    }
};
