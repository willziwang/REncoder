#include <bits/stdc++.h>
#include <immintrin.h>
#include "BOBHash32.h"
#include "MurmurHash3.h"
#include "BF.h"
#include "sBF.h"
#include "sBF_SIMD.h"

#include "rosetta_RN.h"
#include "rosetta_RP.h"
#include "rosetta_LN.h"
#include "rosetta_LP.h"

#include "rbm-basic.h"
#include "rbm-basic_RN.h"
#include "rbm-basic_RN_SIMD.h"
#include "rbm-basic_RP.h"
#include "rbm-basic_RP_SIMD.h"
#include "rbm-basic_LN.h"
#include "rbm-basic_LN_SIMD.h"
#include "rbm-basic_LP.h"
#include "rbm-basic_LP_SIMD.h"
#include "rbm-basic_LN_DR.h"
#include "rsbf.h"
#include "rsbf_RN.h"
#include "rsbf_RP.h"
#include "rsbf_LN.h"
#include "rsbf_LP.h"
#include "rsbf2.h"
#include "disk.h"

using namespace std;
long long cache_hit = 0;
long long query_count = 0;
long long bfQuery = 0;
stack<pair<uint64_t, uint64_t>> psr;
stack<pair<uint64_t, uint64_t>> psd;
long long lndoubt = 0;
long long rndoubt = 0;
long long doubt_count[33];
long long true_querys[33];
double miss_rates[33];
string filename;
// extern int levelQuery[64];
// extern int levelFPR[64];
// extern int RBMQuery[64];
// extern int RBMFPR[64];
Bloomfilter bf;
// RSBF_2 rsbf2;
// Rosetta_RN rosetta_rn;
// Rosetta_RP rosetta_rp;
// Rosetta_LN rosetta_ln;
// Rosetta_LP rosetta_lp;
// RBM rbm;
// RBM_RN_SIMD rbm_rn_simd;
// RBM_RP_SIMD rbm_rp_simd;
// RBM_LN_SIMD rbm_ln_simd;
RBM_LP_SIMD rbm_lp_simd;
// Bloomfilter bf;
// RBM_RN rbm_rn;
// RBM_RP rbm_rp;
// RBM_LN rbm_ln;
RBM_LP rbm_lp;
// RSBF rsbf[5];
// RSBF_RN rsbf_rn[5];
// RSBF_RP rsbf_rp[5];
// RSBF_LN rsbf_ln[5];
// RSBF_LP rsbf_lp[5];
const int MAX_RANGE_SIZE = 1,
          BIT_PER_KEY_ROSETTA = 120, BIT_PER_KEY_RBM = 24;
int BPK;
int QL;
int IF_SELF_ADAPT;
int STEP;
int MAX_ITEM_NUM;
int rangeQueryNum;
vector<uint64_t> keys;
struct Lookup
{
    uint64_t key;
    uint64_t value;
};
vector<Lookup> Lookups;
vector<uint64_t> querys;
vector<uint64_t> uppers;
set<uint64_t> key_set;
map<uint64_t, bool> query_map[MAX_RANGE_SIZE + 1];
vector<DiskBlock<uint64_t>> files;
vector<uint64_t> keybuffer;
vector<uint64_t> maxkeys;
ofstream testData;
ofstream resultData;
ifstream intestData;
int pr = 0, testt = 0;
uint64_t rand64()
{
    return rand() ^ ((uint64_t)rand() << 15) ^ ((uint64_t)rand() << 30) ^ ((uint64_t)rand() << 45) ^ ((uint64_t)rand() << 60);
}

uint32_t rand32()
{
    return rand() ^ ((uint64_t)rand() << 15) ^ ((uint64_t)rand() << 30);
}
void genNormalKey()
{
    int repeat = 0;
    double mean = 1ul << 63;
    double stddev = 1ul << 42; //34for30M
    default_random_engine generator(990808);
    normal_distribution<double> distribution(mean, stddev);
    for (int i = 0; i < MAX_ITEM_NUM; i++)
    {
        keys[i] = distribution(generator);
        if (key_set.find(keys[i]) != key_set.end())
        {
            repeat++;
        }
        key_set.insert(keys[i]);
    }
    cout << repeat << endl;
}
void genRandomKey()
{
    // int repeat = 0;
    // // testData.open("testkey.txt");
    // for (int i = 0; i < MAX_ITEM_NUM; i++)
    // {
    //     keys[i] = rand64();
    //     // testData << hex << keys[i] << endl;
    //     if (key_set.find(keys[i]) != key_set.end())
    //     {
    //         repeat++;
    //     }
    //     key_set.insert(keys[i]);
    // }
    // cout << repeat << endl;
    // // testData.close();

    // intestData.open("./workloads_SuRF/load_randint");
    // testData.open("./my_data_Rosetta/data.txt");
    // uint64_t key;
    // for (int i = 0; i < MAX_ITEM_NUM * 2; i++)
    // {
    //     intestData >> key;
    //     keys[i] = key;
    // }
    // random_shuffle(keys.begin(), keys.end());
    // sort(keys.begin(), keys.begin()+MAX_ITEM_NUM);
    // for (int i = 0; i < MAX_ITEM_NUM; i++)
    // {
    //     key_set.insert(keys[i]);
    //     testData << keys[i] << endl;
    // }
    // cout << keys[0] << endl;
    // intestData.close();
    // testData.close();

    intestData.open("../test_data/data.txt");
    uint64_t key;
    int count = 0;
    for (int i = 0; i < MAX_ITEM_NUM; i++)
    {
        intestData >> key;
        if (count == 0)
        {
            cout << key << endl;
            count = 1;
        }
        keys[i] = key;
        key_set.insert(keys[i]);
#ifdef USE_DISK
        keybuffer.push_back(keys[i]);
        if (keybuffer.size() == MAX_BUFFER_SIZE)
        {
            files.emplace_back(keybuffer, "./disk_experiment_binary." + to_string(files.size()), 64);
            maxkeys.push_back(key);
            keybuffer.clear();
        }
#endif
    }
    cout << keys[MAX_ITEM_NUM - 1] << endl;
#ifdef USE_DISK
    files.emplace_back(keybuffer, "./disk_experiment_binary." + to_string(files.size()), 64);
    maxkeys.push_back(key);
    keybuffer.clear();
#endif
    intestData.close();
    // for (uint64_t maxkey : maxkeys)
    // {
    //     cout << maxkey << endl;
    // }
    // ifstream in(filename, ios::binary);
    // if (!in.is_open())
    // {
    //     cerr << "unable to open file" << endl;
    //     exit(EXIT_FAILURE);
    // }
    // uint64_t size;
    // in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    // in.read(reinterpret_cast<char *>(keys.data()), size * sizeof(uint64_t));
    // for (int i = 0; i < MAX_ITEM_NUM; i++)
    // {
    //     key_set.insert(keys[i]);
    //     keybuffer.push_back(keys[i]);
    //     if (keybuffer.size() == MAX_BUFFER_SIZE)
    //     {
    //         files.emplace_back(keybuffer, "./disk_experiment_binary." + to_string(files.size()), 8);
    //         keybuffer.clear();
    //     }
    // }
    // files.emplace_back(keybuffer, "./disk_experiment_binary." + to_string(files.size()), 8);
    // keybuffer.clear();
}
void genQuery()
{
    // for (int i = 0; i < rangeQueryNum; i++)
    //     querys[i] = rand64();

    intestData.open("../test_data/txn.txt");
    uint64_t query;
    for (int i = 0; i < rangeQueryNum; i++)
    {
        intestData >> query;
        querys[i] = query;
    }
    intestData.close();
    intestData.open("../test_data/upper_bound.txt");
    uint64_t upper;
    for (int i = 0; i < rangeQueryNum; i++)
    {
        intestData >> upper;
        uppers[i] = upper - 1;
    }
    cout << querys[0] << " " << uppers[0] << endl;
    intestData.close();

    // intestData.open("./workloads_SuRF/txn_randint_uniform");
    // testData.close();
    // testData.open("./my_data_Rosetta/txn.txt");
    // uint64_t query;
    // for (int i = 0; i < rangeQueryNum; i++)
    // {
    //     intestData >> query;
    //     querys[i] = query;
    // }
    // sort(querys, querys + rangeQueryNum);
    // for (int i = 0; i < rangeQueryNum; i++)
    // {
    //     testData << querys[i] << endl;
    // }
    // cout << querys[0] << endl;
    // testData.close();
    // testData.open("./my_data_Rosetta/upper_bound.txt");
    // for (int i = 0; i < rangeQueryNum; i++)
    // {
    //     uppers[i] = querys[i] + (rand() % 32);
    //     testData << uppers[i] + 1 << endl;
    // }
    // cout << uppers[0] + 1 << endl;
    // testData.close();
    // intestData.close();

    // ifstream in(filename + "_equality_lookups_10M", ios::binary);
    // if (!in.is_open())
    // {
    //     cerr << "unable to open " << filename << endl;
    //     exit(EXIT_FAILURE);
    // }
    // uint64_t size;
    // in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    // in.read(reinterpret_cast<char *>(Lookups.data()), size * sizeof(Lookups));
    // for (int i = 0; i < rangeQueryNum; i++)
    // {
    //     querys[i] = Lookups[i].key + 1;
    // }
}
void genQueryMap()
{
    srand(990808);
    genRandomKey();
    testData.open(filename + "_testquery");
    genQuery();
    set<uint64_t>::iterator iter;
    for (int i = 0; i < rangeQueryNum; i++)
    {
        testData << querys[i] << " ";
        iter = key_set.lower_bound(querys[i]);
        for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        {
            // if (iter != key_set.end() && (querys[i] + (uint64_t)(1ll << rsize) - 1 < querys[i] || *iter <= querys[i] + (1ll << rsize) - 1))
            if (iter != key_set.end() && (*iter) <= uppers[i])
            // if (iter != key_set.end() && (*iter) <= querys[i] + (1ll << rsize) - 1)
            {
                testData << 1 << " ";
                query_map[rsize].insert(map<uint64_t, bool>::value_type(querys[i], true));
            }
            else
            {
                testData << 0 << " ";
                query_map[rsize].insert(map<uint64_t, bool>::value_type(querys[i], false));
            }
        }
        testData << endl;
        // if (i % 1000 == 0)
        // {
        //     cout << i << endl;
        // }
    }
    testData.close();
    // intestData.open("testquery.txt");
    // for (int i = 0; i < rangeQueryNum; i++)
    // {
    //     int flag;
    //     intestData >> querys[i];
    //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
    //     {
    //         intestData >> flag;
    //         if (flag == 1)
    //         {
    //             query_map[rsize].insert(map<uint64_t, bool>::value_type(querys[i], true));
    //         }
    //         else
    //         {
    //             query_map[rsize].insert(map<uint64_t, bool>::value_type(querys[i], false));
    //         }
    //     }
    //     if (i % 1000 == 0)
    //     {
    //         cout << i << endl;
    //     }
    // }
    // intestData.close();
}
uint64_t queryLevel()
{
    uint64_t maxrsize = 0;
    for (uint32_t i = 0; i < rangeQueryNum; i++)
    {
        if ((size_t)rand() % 10000 < 10000 * 10000 / rangeQueryNum)
        {
            // uint64_t l = querys[i], r = (querys[i] + (1ll << MAX_RANGE_SIZE) - 1 > querys[i] ? querys[i] + (1ll << MAX_RANGE_SIZE) - 1 : -1);
            uint64_t l = querys[i], r = uppers[i];
            if (r - l > maxrsize)
            {
                maxrsize = r - l;
            }
        }
    }
    uint64_t level = log(maxrsize + 1) / log(2) + 1;
    cout << "query level = " << level << endl;
    return level;
}
void test()
{
    Lookups.clear();
    key_set.clear();
    for (int i = 0; i <= MAX_RANGE_SIZE; i++)
    {
        query_map[i].clear();
    }
    for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
    {
        resultData << ","
                   << "2^" + to_string(rsize);
    }
    resultData << endl;
    genQueryMap();
    timespec time1, time2;
    long long resns, kk, kkk = 1, QueryTestNum = 0;
    double th;
    // cout << "test" << filename << endl;
    cout << "Insert " << MAX_ITEM_NUM << "items" << endl;
    uint64_t query_level = queryLevel();
    uint64_t memory = (uint64_t)MAX_ITEM_NUM * BPK;
    for (int ql = QL; ql <= QL; ql += 1)
    {
        // hash_num = (size_t)(round(log(2) * BIT_PER_KEY_RBM));
        cout << "set_level = " << ql << endl;
        int hash_num = 3;
        cout << "hash_num = " << hash_num << endl;
        cout << "BPK = " << BPK << endl;
        rbm_lp_simd.init(memory, hash_num, 64, ql);
        // rbm_lp.init(memory, hash_num, 64, ql);
        // rbm_ln.init(memory, hash_num, 64);
        // rbm_ln_simd.init(memory, hash_num, 64);
        // bf.init(memory, 18);
        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rosetta_rn.Insert(keys[i]);
        //     }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("Rosetta_RN\tInsert throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rosetta_rp.Insert(keys[i]);
        //     }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("Rosetta_RP\tInsert throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rosetta_ln.Insert(keys[i]);
        //     }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("Rosetta_LN\tInsert throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rosetta_lp.Insert(keys[i]);
        //     }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("Rosetta_LP\tInsert throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rbm.Insert4(keys[i]);
        //     }
        // }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RBM\tInsert4 throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rbm_rn.Insert4(keys[i]);
        //         // if (i == 49999999)
        //         // {
        //         //     int count=0;
        //         //     double onerate = 0;
        //         //     for (uint64_t i = 0; i < rbm_rn.sbf.counter_num; i++)
        //         //     {
        //         //         double onerate_per_counter = 0;
        //         //         uint64_t z = rbm_rn.sbf.array[0][i];
        //         //         for (int j = 0; j < 8; j++)
        //         //         {
        //         //             onerate_per_counter += (z & 1);
        //         //             z >>= 1;
        //         //         }
        //         //         onerate += onerate_per_counter;
        //         //         if(onerate_per_counter==0)
        //         //         {
        //         //             printf("RBM_RN\tOneRate %ld is %lf\n", i, onerate_per_counter);
        //         //             count++;
        //         //         }
        //         //     }
        //         //     printf("RBM_RN\tZero count is %d\n",count);
        //         //     printf("RBM_RN\tOneRate %d is %lf\n", i, onerate);
        //         // }
        //     }
        // }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RBM_RN\tInsert4 throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rbm_rn_simd.Insert4(keys[i]);
        //     }
        // }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RBM_RN_SIMD\tInsert4 throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rbm_rp.Insert4(keys[i]);
        //     }
        // }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RBM_RP\tInsert4 throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rbm_rp_simd.Insert4(keys[i]);
        //     }
        // }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RBM_RP_SIMD\tInsert4 throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rbm_ln.Insert4(keys[i]);
        //     }
        // }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RBM_LN\tInsert4 throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rbm_ln_simd.Insert4(keys[i]);
        //     }
        // }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RBM_LN_SIMD\tInsert4 throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         rbm_lp.Insert4(keys[i]);
        //     }
        // }

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RBM_LP\tInsert4 throughput is %lf mips\n", th);

        clock_gettime(CLOCK_MONOTONIC, &time1);
        kk = kkk;
        for (int k = 1; k <= kk; k++)
        {
            if (IF_SELF_ADAPT)
            {
                int true_level = rbm_lp_simd.Insert4(keys, STEP);
                cout << "true_level: " << true_level << endl;
            }
            else
            {
                for (int i = 0; i < MAX_ITEM_NUM; i++)
                {
                    rbm_lp_simd.Insert4(keys[i]);
                }
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        printf("RBM_LP_SIMD\tInsert throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        // {
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //     {
        //         bf.insert(keys[i]);
        //     }
        // }
        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("BF\tInsert4 throughput is %lf mips\n", th);

        // clock_gettime(CLOCK_MONOTONIC, &time1);
        // kk = kkk;
        // for (int k = 1; k <= kk; k++)
        //     for (int i = 0; i < MAX_ITEM_NUM; i++)
        //         rsbf2.Insert4(keys[i]);

        // clock_gettime(CLOCK_MONOTONIC, &time2);
        // resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        // th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        // printf("RSBF_2_old\tInsert4 throughput is %lf mips\n", th);

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     for (int k = 1; k <= kk; k++)
        //         for (int i = 0; i < MAX_ITEM_NUM; i++)
        //             rsbf[k_tree].Insert4(keys[i]);

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        //     printf("RSBF_%d\tInsert4 throughput is %lf mips\n", 1 << k_tree, th);
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     for (int k = 1; k <= kk; k++)
        //         for (int i = 0; i < MAX_ITEM_NUM; i++)
        //             rsbf_rn[k_tree].Insert4(keys[i]);

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        //     printf("RSBF_RN_%d\tInsert4 throughput is %lf mips\n", 1 << k_tree, th);
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     for (int k = 1; k <= kk; k++)
        //         for (int i = 0; i < MAX_ITEM_NUM; i++)
        //             rsbf_rp[k_tree].Insert4(keys[i]);

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        //     printf("RSBF_RP_%d\tInsert4 throughput is %lf mips\n", 1 << k_tree, th);
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     for (int k = 1; k <= kk; k++)
        //         for (int i = 0; i < MAX_ITEM_NUM; i++)
        //             rsbf_ln[k_tree].Insert4(keys[i]);

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        //     printf("RSBF_LN_%d\tInsert4 throughput is %lf mips\n", 1 << k_tree, th);
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     for (int k = 1; k <= kk; k++)
        //         for (int i = 0; i < MAX_ITEM_NUM; i++)
        //             rsbf_lp[k_tree].Insert4(keys[i]);

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * MAX_ITEM_NUM * kk / resns;

        //     printf("RSBF_LP_%d\tInsert4 throughput is %lf mips\n", 1 << k_tree, th);
        // }

        // resultData << "Rosetta_RN";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             QueryTestNum++;
        //             rosetta_rn.RangeQuery(l, r);
        //         }

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("Rosetta_RN\ttrue query is %lld\n",bfQuery);
        //     true_querys[rsize] = bfQuery;
        //     bfQuery =0;
        //     resultData << "," << th;
        //     printf("Rosetta_RN\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "Rosetta_RN_truequery";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;

        // resultData << "Rosetta_RP";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             QueryTestNum++;
        //             rosetta_rp.RangeQuery(l, r);
        //         }

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("Rosetta_RP\ttrue query is %lld\n",bfQuery);
        //     true_querys[rsize] = bfQuery;
        //     bfQuery =0;
        //     resultData << "," << th;
        //     printf("Rosetta_RP\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "Rosetta_RP_truequery";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;

        // resultData << "Rosetta_LN";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     bfQuery =0;
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //             QueryTestNum++;
        //             rosetta_ln.RangeQuery(l, r);
        //         }

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("Rosetta_LN\ttrue query is %lld\n",bfQuery);
        //     true_querys[rsize] = bfQuery;
        //     resultData << "," << th;
        //     printf("Rosetta_LN\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "Rosetta_LN_truequery";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;

        // resultData << "Rosetta_LP";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     bfQuery = 0;
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //             // uint64_t l = querys[i], r = uppers[i];
        //             uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //             QueryTestNum++;
        //             rosetta_lp.RangeQuery(l, r);
        //         }

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("Rosetta_LP\ttrue query is %lld\n", bfQuery);
        //     true_querys[rsize] = bfQuery;
        //     resultData << "," << th;
        //     printf("Rosetta_LP\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "Rosetta_LP_truequery";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;

        // resultData << "Rosetta_LP_DISK";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     double fp=0, empty=0;
        //     bool positive, full;
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //             QueryTestNum++;
        //             if (rosetta_lp.RangeQuery(l, r))
        //             {
        //                 positive = true;
        //                 for (auto &file : files)
        //                 {
        //                     if (file.IsRangeFull(l, r))
        //                     {
        //                         full = true;
        //                         break;
        //                     }
        //                 }
        //                 if (positive && !full)
        //                 {
        //                     ++fp;
        //                 }

        //                 if (!full)
        //                 { // filterAns will also be false
        //                     ++empty;
        //                 }
        //             }
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     resultData << "," << th;
        //     printf("Rosetta_LP\tquery2^%d throughput_DISK is %lf mips\n", rsize, th);
        //     printf("Rosetta_LP\tquery2^%d fpr is %lf mips\n", rsize, fp / empty);
        // }
        // resultData << endl;
        // resultData << "RBM";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             QueryTestNum++;
        //             rbm.RangeQuery(l, r);
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("RBM\tmiss rate is %lf\n",(double)(query_count-cache_hit)/query_count);
        //     miss_rates[rsize]=(double)(query_count-cache_hit)/query_count;
        //     printf("RBM\ttrue query is %lld\n",query_count-cache_hit);
        //     true_querys[rsize]=query_count-cache_hit;
        //     cache_hit = 0;
        //     query_count = 0;
        //     resultData << "," << th;
        //     printf("RBM\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "RBM_missrate";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << miss_rates[i];
        // }
        // resultData << endl;
        // resultData << "RBM_truequery";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;

        // resultData << "RBM_RN";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //             QueryTestNum++;
        //             rbm_rn.RangeQuery(l, r);
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("RBM_RN\tmiss rate is %lf\n", (double)(query_count - cache_hit) / query_count);
        //     miss_rates[rsize] = (double)(query_count - cache_hit) / query_count;
        //     printf("RBM_RN\ttrue query is %lld\n", query_count - cache_hit);
        //     true_querys[rsize] = query_count - cache_hit;
        //     cache_hit = 0;
        //     query_count = 0;
        //     resultData << "," << th;
        //     printf("RBM_RN\tquery2^%d doubt_count is %lld\n", rsize, rndoubt);
        //     doubt_count[rsize] = rndoubt;
        //     rndoubt = 0;
        //     printf("RBM_RN\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "RBM_RN_missrate";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << miss_rates[i];
        // }
        // resultData << endl;
        // resultData << "RBM_RN_truequery";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;
        // resultData << "RBM_RN_doubt_count";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << doubt_count[i];
        // }
        // resultData << endl;

        // resultData << "RBM_RN_SIMD";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //             QueryTestNum++;
        //             rbm_rn_simd.RangeQuery(l, r);
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("RBM_RN_SIMD\tmiss rate is %lf\n", (double)(query_count - cache_hit) / query_count);
        //     miss_rates[rsize] = (double)(query_count - cache_hit) / query_count;
        //     printf("RBM_RN_SIMD\ttrue query is %lld\n", query_count - cache_hit);
        //     true_querys[rsize] = query_count - cache_hit;
        //     cache_hit = 0;
        //     query_count = 0;
        //     resultData << "," << th;
        //     printf("RBM_RN_SIMD\tquery2^%d doubt_count is %lld\n", rsize, rndoubt);
        //     doubt_count[rsize] = rndoubt;
        //     rndoubt = 0;
        //     printf("RBM_RN_SIMD\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "RBM_RN_SIMD_missrate";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << miss_rates[i];
        // }
        // resultData << endl;
        // resultData << "RBM_RN_SIMD_truequery";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;
        // resultData << "RBM_RN_SIMD_doubt_count";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << doubt_count[i];
        // }
        // resultData << endl;

        // resultData << "RBM_RP";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //             QueryTestNum++;
        //             rbm_rp.RangeQuery(l, r);
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("RBM_RP\tmiss rate is %lf\n", (double)(query_count - cache_hit) / query_count);
        //     miss_rates[rsize] = (double)(query_count - cache_hit) / query_count;
        //     printf("RBM_RP\ttrue query is %lld\n", query_count - cache_hit);
        //     true_querys[rsize] = query_count - cache_hit;
        //     cache_hit = 0;
        //     query_count = 0;
        //     resultData << "," << th;
        //     printf("RBM_RP\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "RBM_RP_missrate";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << miss_rates[i];
        // }
        // resultData << endl;
        // resultData << "RBM_RP_truequery";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;

        // resultData << "RBM_RP_SIMD";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //             QueryTestNum++;
        //             rbm_rp_simd.RangeQuery(l, r);
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("RBM_RP_SIMD\tmiss rate is %lf\n", (double)(query_count - cache_hit) / query_count);
        //     miss_rates[rsize] = (double)(query_count - cache_hit) / query_count;
        //     printf("RBM_RP_SIMD\ttrue query is %lld\n", query_count - cache_hit);
        //     true_querys[rsize] = query_count - cache_hit;
        //     cache_hit = 0;
        //     query_count = 0;
        //     resultData << "," << th;
        //     printf("RBM_RP_SIMD\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "RBM_RP_SIMD_missrate";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << miss_rates[i];
        // }
        // resultData << endl;
        // resultData << "RBM_RP_SIMD_truequery";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;

        // resultData << "RBM_LN";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     cache_hit = 0;
        //     query_count = 0;
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //             QueryTestNum++;
        //             rbm_ln.RangeQuery(l, r);
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("RBM_LN\tmiss rate is %lf\n",(double)(query_count-cache_hit)/query_count);
        //     miss_rates[rsize]=(double)(query_count-cache_hit)/query_count;
        //     printf("RBM_LN\ttrue query is %lld\n",query_count-cache_hit);
        //     true_querys[rsize]=query_count-cache_hit;
        //     resultData << "," << th;
        //     printf("RBM_LN\tquery2^%d doubt_count is %lld\n", rsize, lndoubt);
        //     doubt_count[rsize]=lndoubt;
        //     lndoubt = 0;
        //     printf("RBM_LN\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "RBM_LN_missrate";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << miss_rates[i];
        // }
        // resultData << endl;
        // resultData << "RBM_LN_truequery";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;
        // resultData << "RBM_LN_doubt_count";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << doubt_count[i];
        // }
        // resultData << endl;

        // resultData << "RBM_LN_SIMD";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     cache_hit = 0;
        //     query_count = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //             QueryTestNum++;
        //             rbm_ln_simd.RangeQuery(l, r);
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("RBM_LN_SIMD\tmiss rate is %lf\n",(double)(query_count-cache_hit)/query_count);
        //     miss_rates[rsize]=(double)(query_count-cache_hit)/query_count;
        //     printf("RBM_LN_SIMD\ttrue query is %lld\n",query_count-cache_hit);
        //     true_querys[rsize]=query_count-cache_hit;
        //     resultData << "," << th;
        //     printf("RBM_LN_SIMD\tquery2^%d doubt_count is %lld\n", rsize, lndoubt);
        //     doubt_count[rsize]=lndoubt;
        //     lndoubt = 0;
        //     printf("RBM_LN_SIMD\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "RBM_LN_SIMD_missrate";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << miss_rates[i];
        // }
        // resultData << endl;
        // resultData << "RBM_LN_SIMD_truequery";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;
        // resultData << "RBM_LN_SIMD_doubt_count";
        // for(int i=0;i<=MAX_RANGE_SIZE;i++)
        // {
        //     resultData << "," << doubt_count[i];
        // }
        // resultData << endl;

        // resultData << "RBM_LP";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     cache_hit = 0;
        //     query_count = 0;
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) - 1 >= querys[i] ? querys[i] + (1ll << rsize) - 1 : -1);
        //             uint64_t l = querys[i], r = uppers[i];
        //             // uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //             QueryTestNum++;
        //             rbm_lp.RangeQuery(l, r);
        //         }
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("RBM_LP\tmiss rate is %lf\n", (double)(query_count - cache_hit) / query_count);
        //     miss_rates[rsize] = (double)(query_count - cache_hit) / query_count;
        //     printf("RBM_LP\ttrue query is %lld\n", query_count - cache_hit);
        //     true_querys[rsize] = query_count - cache_hit;
        //     resultData << "," << th;
        //     printf("RBM_LP\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;
        // resultData << "RBM_LP_missrate";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << miss_rates[i];
        // }
        // resultData << endl;
        // resultData << "RBM_LP_truequery";
        // for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        // {
        //     resultData << "," << true_querys[i];
        // }
        // resultData << endl;

        resultData << "RBM_LP_SIMD";
        for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        {
            cache_hit = 0;
            query_count = 0;
            clock_gettime(CLOCK_MONOTONIC, &time1);
            kk = kkk;
            QueryTestNum = 0;
            int ss = 0;
            for (int k = 1; k <= kk; k++)
                for (uint32_t i = 0; i < rangeQueryNum; i++)
                {
                    // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) - 1 >= querys[i] ? querys[i] + (1ll << rsize) - 1 : -1);
                    uint64_t l = querys[i], r = uppers[i];
                    // uint64_t l = querys[i], r = querys[i] + (1ll << rsize) - 1;
                    QueryTestNum++;
                    ss ^= rbm_lp_simd.RangeQuery(l, r);
                }
            // cout << ss << endl;
            clock_gettime(CLOCK_MONOTONIC, &time2);
            resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
            th = (double)1000.0 * QueryTestNum / resns;
            printf("RBM_LP_SIMD\tmiss rate is %lf\n", (double)(query_count - cache_hit) / query_count);
            miss_rates[rsize] = (double)(query_count - cache_hit) / query_count;
            printf("RBM_LP_SIMD\ttrue query is %lld\n", query_count - cache_hit);
            true_querys[rsize] = query_count - cache_hit;
            resultData << "," << th;
            printf("RBM_LP_SIMD\tquery throughput is %lf mips\n", th);
        }
        resultData << endl;
        resultData << "RBM_LP_SIMD_missrate";
        for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        {
            resultData << "," << miss_rates[i];
        }
        resultData << endl;
        resultData << "RBM_LP_SIMD_truequery";
        for (int i = 0; i <= MAX_RANGE_SIZE; i++)
        {
            resultData << "," << true_querys[i];
        }
        resultData << endl;

        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     cache_hit = 0;
        //     query_count = 0;
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     int ss=0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 0; i < rangeQueryNum; i++)
        //         {
        //             // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) - 1 >= querys[i] ? querys[i] + (1ll << rsize) - 1 : -1);
        //             uint64_t l = querys[i], r = uppers[i];
        //             QueryTestNum++;
        //             ss ^= bf.query(l);
        //         }
        //     cout <<ss<<endl;
        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     printf("BF\ttrue query is %lld\n", query_count);
        //     printf("BF\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }

#ifdef USE_DISK
        resultData << "RBM_LP_SIMD_DISK";
        for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        {
            double fp = 0, empty = 0;
            bool positive, full;
            clock_gettime(CLOCK_MONOTONIC, &time1);
            kk = kkk;
            QueryTestNum = 0;
            for (int k = 1; k <= kk; k++)
                for (uint32_t i = 0; i < rangeQueryNum; i++)
                {
                    positive = false, full = false;
                    // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
                    uint64_t l = querys[i], r = uppers[i];
                    QueryTestNum++;
                    positive = rbm_lp_simd.RangeQuery(l, r);
                    if (positive)
                    {
                        auto iter = lower_bound(maxkeys.begin(), maxkeys.end(), l);
                        int start = iter - maxkeys.begin();
                        iter = lower_bound(maxkeys.begin(), maxkeys.end(), r);
                        int end = iter - maxkeys.begin();
                        for (int fn = start; fn <= end; fn++)
                        {
                            if (files[fn].IsRangeFull(l, r))
                            {
                                full = true;
                                break;
                            }
                        }
                        // for (auto &file : files)
                        // {
                        //     if (file.IsRangeFull(l, r))
                        //     {
                        //         full = true;
                        //         break;
                        //     }
                        // }
                    }
                    if (positive && !full)
                    {
                        ++fp;
                    }

                    if (!full)
                    { // filterAns will also be false
                        ++empty;
                    }
                }
            clock_gettime(CLOCK_MONOTONIC, &time2);
            resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
            th = (double)1000.0 * QueryTestNum / resns;
            resultData << "," << th;
            printf("RBM_LP_SIMD\tquery throughput_DISK is %lf mips\n", th);
            printf("RBM_LP_SIMD\tquery FP_DISK is %lf\n", fp);
        }
        resultData << endl;

        resultData << "RBM_LP_SIMD_SET";
        for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        {
            double fp = 0, empty = 0;
            bool positive, full;
            clock_gettime(CLOCK_MONOTONIC, &time1);
            kk = kkk;
            QueryTestNum = 0;
            for (int k = 1; k <= kk; k++)
                for (uint32_t i = 0; i < rangeQueryNum; i++)
                {
                    positive = false, full = false;
                    // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
                    uint64_t l = querys[i], r = uppers[i];
                    QueryTestNum++;
                    positive = rbm_lp_simd.RangeQuery(l, r);
                    if (positive)
                    {
                        // auto iter = key_set.lower_bound(l);
                        // if (iter != key_set.end() && (*iter) <= r)
                        // {
                        //     full = true;
                        // }
                        size_t start;
                        auto iter = lower_bound(maxkeys.begin(), maxkeys.end(), l);
                        if (iter == maxkeys.begin())
                        {
                            start = 0;
                        }
                        else
                        {
                            iter--;
                            start = *iter;
                        }
                        auto iter2 = key_set.lower_bound(start);
                        while (iter2 != key_set.end())
                        {
                            if ((*iter2) >= l)
                            {
                                if ((*iter2) <= r)
                                {
                                    full = true;
                                }
                                break;
                            }
                            iter2++;
                        }
                    }
                    if (positive && !full)
                    {
                        ++fp;
                    }

                    if (!full)
                    { // filterAns will also be false
                        ++empty;
                    }
                }
            clock_gettime(CLOCK_MONOTONIC, &time2);
            resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
            th = (double)1000.0 * QueryTestNum / resns;
            resultData << "," << th;
            printf("RBM_LP_SIMD\tquery throughput_SET is %lf mips\n", th);
            printf("RBM_LP_SIMD\tquery FP_SET is %lf\n", fp);
        }
#endif
        // resultData << endl;
        // resultData << "RSBF_2_old";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     clock_gettime(CLOCK_MONOTONIC, &time1);
        //     kk = kkk;
        //     QueryTestNum = 0;
        //     for (int k = 1; k <= kk; k++)
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             QueryTestNum++;
        //             rsbf2.RangeQuery(l, r);
        //         }

        //     clock_gettime(CLOCK_MONOTONIC, &time2);
        //     resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //     th = (double)1000.0 * QueryTestNum / resns;
        //     // printf("RSBF_2_old\tmiss rate is %lf\n",(double)(query_count-cache_hit)/query_count);
        //     cache_hit = 0;
        //     query_count = 0;
        //     resultData << "," << th;
        //     printf("RSBF_2_old\tquery2^%d throughput is %lf mips\n", rsize, th);
        // }
        // resultData << endl;

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         clock_gettime(CLOCK_MONOTONIC, &time1);
        //         kk = kkk;
        //         QueryTestNum = 0;
        //         for (int k = 1; k <= kk; k++)
        //             for (uint32_t i = 1; i < rangeQueryNum; i++)
        //             {
        //                 uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //                 QueryTestNum++;
        //                 rsbf[k_tree].RangeQuery(l, r);
        //             }

        //         clock_gettime(CLOCK_MONOTONIC, &time2);
        //         resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //         th = (double)1000.0 * QueryTestNum / resns;
        //         // printf("RSBF_%d\tmiss rate is %lf\n",1 << k_tree,(double)(query_count-cache_hit)/query_count);
        //         cache_hit = 0;
        //         query_count = 0;
        //         resultData << "," << th;
        //         printf("RSBF_%d\tquery2^%d throughput is %lf mips\n", 1 << k_tree, rsize, th);
        //     }
        //     resultData << endl;
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_RN_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         clock_gettime(CLOCK_MONOTONIC, &time1);
        //         kk = kkk;
        //         QueryTestNum = 0;
        //         for (int k = 1; k <= kk; k++)
        //             for (uint32_t i = 1; i < rangeQueryNum; i++)
        //             {
        //                 uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //                 QueryTestNum++;
        //                 rsbf_rn[k_tree].RangeQuery(l, r);
        //             }

        //         clock_gettime(CLOCK_MONOTONIC, &time2);
        //         resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //         th = (double)1000.0 * QueryTestNum / resns;
        //         // printf("RSBF_%d\tmiss rate is %lf\n",1 << k_tree,(double)(query_count-cache_hit)/query_count);
        //         cache_hit = 0;
        //         query_count = 0;
        //         resultData << "," << th;
        //         printf("RSBF_RN_%d\tquery2^%d throughput is %lf mips\n", 1 << k_tree, rsize, th);
        //     }
        //     resultData << endl;
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_RP_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         clock_gettime(CLOCK_MONOTONIC, &time1);
        //         kk = kkk;
        //         QueryTestNum = 0;
        //         for (int k = 1; k <= kk; k++)
        //             for (uint32_t i = 1; i < rangeQueryNum; i++)
        //             {
        //                 uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //                 QueryTestNum++;
        //                 rsbf_rp[k_tree].RangeQuery(l, r);
        //             }

        //         clock_gettime(CLOCK_MONOTONIC, &time2);
        //         resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //         th = (double)1000.0 * QueryTestNum / resns;
        //         // printf("RSBF_%d\tmiss rate is %lf\n",1 << k_tree,(double)(query_count-cache_hit)/query_count);
        //         cache_hit = 0;
        //         query_count = 0;
        //         resultData << "," << th;
        //         printf("RSBF_RP_%d\tquery2^%d throughput is %lf mips\n", 1 << k_tree, rsize, th);
        //     }
        //     resultData << endl;
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_LN_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         clock_gettime(CLOCK_MONOTONIC, &time1);
        //         kk = kkk;
        //         QueryTestNum = 0;
        //         for (int k = 1; k <= kk; k++)
        //             for (uint32_t i = 1; i < rangeQueryNum; i++)
        //             {
        //                 uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //                 QueryTestNum++;
        //                 rsbf_ln[k_tree].RangeQuery(l, r);
        //             }

        //         clock_gettime(CLOCK_MONOTONIC, &time2);
        //         resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //         th = (double)1000.0 * QueryTestNum / resns;
        //         // printf("RSBF_%d\tmiss rate is %lf\n",1 << k_tree,(double)(query_count-cache_hit)/query_count);
        //         cache_hit = 0;
        //         query_count = 0;
        //         resultData << "," << th;
        //         printf("RSBF_LN_%d\tquery2^%d throughput is %lf mips\n", 1 << k_tree, rsize, th);
        //     }
        //     resultData << endl;
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_LP_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         clock_gettime(CLOCK_MONOTONIC, &time1);
        //         kk = kkk;
        //         QueryTestNum = 0;
        //         for (int k = 1; k <= kk; k++)
        //             for (uint32_t i = 1; i < rangeQueryNum; i++)
        //             {
        //                 uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //                 QueryTestNum++;
        //                 rsbf_lp[k_tree].RangeQuery(l, r);
        //             }

        //         clock_gettime(CLOCK_MONOTONIC, &time2);
        //         resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        //         th = (double)1000.0 * QueryTestNum / resns;
        //         // printf("RSBF_%d\tmiss rate is %lf\n",1 << k_tree,(double)(query_count-cache_hit)/query_count);
        //         cache_hit = 0;
        //         query_count = 0;
        //         resultData << "," << th;
        //         printf("RSBF_LP_%d\tquery2^%d throughput is %lf mips\n", 1 << k_tree, rsize, th);
        //     }
        //     resultData << endl;
        // }

        double FPR = 0, FPRQSUM = 0;
        // resultData << endl;
        // resultData << "Rosetta_RN";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rosetta_rn.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rosetta_rn.RangeQuery(l, r))
        //         {
        //             if (i < 100)
        //             {
        //                 cout << i << ",";
        //             }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("Rosetta_RN\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "Rosetta_RP";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rosetta_rp.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rosetta_rp.RangeQuery(l, r))
        //         {
        //             if (i < 100)
        //             {
        //                 cout << i << ",";
        //             }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("Rosetta_RP\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "Rosetta_LN";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rosetta_ln.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rosetta_ln.RangeQuery(l, r))
        //         {
        //             // if (i < 100)
        //             // {
        //             //     cout << i << ",";
        //             // }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("Rosetta_LN\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "Rosetta_LP";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) - 1 >= querys[i] ? querys[i] + (1ll << rsize) - 1 : -1);
        //         // uint64_t l = querys[i], r = uppers[i];
        //         uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rosetta_lp.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rosetta_lp.RangeQuery(l, r))
        //         {
        //             // if (i < 100)
        //             // {
        //             //     cout << i << ",";
        //             // }
        //             FPR++;
        //         }
        //     }
        //     // cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("Rosetta_LP\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "RBM";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rbm.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rbm.RangeQuery(l, r))
        //         {
        //             if (i < 100)
        //             {
        //                 cout << i << ",";
        //             }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RBM\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "RBM_RN";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rbm_rn.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error in " << i;
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rbm_rn.RangeQuery(l, r))
        //         {
        //             if (i < 100)
        //             {
        //                 cout << i << ",";
        //             }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RBM_RN\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "RBM_RN_SIMD";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rbm_rn_simd.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error in " << i;
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rbm_rn_simd.RangeQuery(l, r))
        //         {
        //             if (i < 100)
        //             {
        //                 cout << i << ",";
        //             }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RBM_RN_SIMD\trange2^%d FPR is %lf,sum = %lf,fpr = %lf\n", rsize, FPR / FPRQSUM,FPRQSUM,FPR);
        // }
        // resultData << endl;

        // resultData << "RBM_RP";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rbm_rp.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error in " << i;
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rbm_rp.RangeQuery(l, r))
        //         {
        //             if (i < 100)
        //             {
        //                 cout << i << ",";
        //             }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RBM_RP\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "RBM_RP_SIMD";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rbm_rp_simd.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rbm_rp_simd.RangeQuery(l, r))
        //         {
        //             if (i < 100)
        //             {
        //                 cout << i << ",";
        //             }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RBM_RP_SIMD\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "RBM_LN";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rbm_ln.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rbm_ln.RangeQuery(l, r))
        //         {
        //             // if (i < 100)
        //             // {
        //             //     cout << i << ",";
        //             // }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RBM_LN\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "RBM_LN_SIMD";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rbm_ln_simd.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rbm_ln_simd.RangeQuery(l, r))
        //         {
        //             // if (i < 100)
        //             // {
        //             //     cout << i << ",";
        //             // }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RBM_LN_SIMD\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // resultData << "RBM_LP";
        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         uint64_t l = querys[i], r = uppers[i];
        //         // uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rbm_lp.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rbm_lp.RangeQuery(l, r))
        //         {
        //             // if (i < 100)
        //             // {
        //             //     cout << i << ",";
        //             // }
        //             FPR++;
        //         }
        //     }
        //     // cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RBM_LP\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        resultData << "RBM_LP_SIMD";
        for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        {
            FPR = 0, FPRQSUM = 0;
            for (uint32_t i = 0; i < rangeQueryNum; i++)
            {
                // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) - 1 >= querys[i] ? querys[i] + (1ll << rsize) - 1 : -1);
                uint64_t l = querys[i], r = uppers[i];
                // uint64_t l = querys[i], r = querys[i] + (1ll << rsize)-1;
                if (query_map[rsize][querys[i]])
                {
                    if (!rbm_lp_simd.RangeQuery(l, r))
                    {
                        cout << "range Query error";
                        exit(-1);
                    }
                    continue;
                }
                FPRQSUM++;
                if (rbm_lp_simd.RangeQuery(l, r))
                {
                    // if (i < 100)
                    // {
                    //     cout << i << ",";
                    // }
                    FPR++;
                }
            }
            // cout << endl;
            resultData << "," << FPR / FPRQSUM;
            printf("RBM_LP_SIMD\trange FPR is %lf sum = %lf,fpr = %lf\n", FPR / FPRQSUM, FPRQSUM, FPR);
        }
        resultData << endl;

        // for (int rsize = 1; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 0; i < rangeQueryNum; i++)
        //     {
        //         // uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) - 1 >= querys[i] ? querys[i] + (1ll << rsize) - 1 : -1);
        //         uint64_t l = querys[i], r = uppers[i];
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!bf.query(l))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (bf.query(l))
        //         {
        //             // if (i < 100)
        //             // {
        //             //     cout << i << ",";
        //             // }
        //             FPR++;
        //         }
        //     }
        //     // cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("BF\trange2^%d FPR is %lf sum = %lf,fpr = %lf\n", rsize, FPR / FPRQSUM, FPRQSUM, FPR);
        // }
        // resultData << "RSBF_2_old";
        // for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        // {
        //     FPR = 0, FPRQSUM = 0;
        //     for (uint32_t i = 1; i < rangeQueryNum; i++)
        //     {
        //         uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1 >= querys[i] ? querys[i] + (1ll << rsize) -1 : -1);
        //         if (query_map[rsize][querys[i]])
        //         {
        //             if (!rsbf2.RangeQuery(l, r))
        //             {
        //                 cout << "range Query error";
        //                 exit(-1);
        //             }
        //             continue;
        //         }
        //         FPRQSUM++;
        //         if (rsbf2.RangeQuery(l, r))
        //         {
        //             if (i < 100)
        //             {
        //                 cout << i << ",";
        //             }
        //             FPR++;
        //         }
        //     }
        //     cout << endl;
        //     resultData << "," << FPR / FPRQSUM;
        //     printf("RSBF_2_old\trange2^%d FPR is %lf\n", rsize, FPR / FPRQSUM);
        // }
        // resultData << endl;

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         FPR = 0, FPRQSUM = 0;
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             if (query_map[rsize][querys[i]])
        //             {
        //                 if (!rsbf[k_tree].RangeQuery(l, r))
        //                 {
        //                     cout << "range Query error";
        //                     exit(-1);
        //                 }
        //                 continue;
        //             }
        //             FPRQSUM++;
        //             if (rsbf[k_tree].RangeQuery(l, r))
        //             {
        //                 FPR++;
        //             }
        //         }
        //         resultData << "," << FPR / FPRQSUM;
        //         printf("RSBF_%d\trange2^%d FPR is %lf\n", 1 << k_tree, rsize, FPR / FPRQSUM);
        //     }
        //     resultData << endl;
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_RN_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         FPR = 0, FPRQSUM = 0;
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             if (query_map[rsize][querys[i]])
        //             {
        //                 if (!rsbf_rn[k_tree].RangeQuery(l, r))
        //                 {
        //                     cout << "range Query error";
        //                     exit(-1);
        //                 }
        //                 continue;
        //             }
        //             FPRQSUM++;
        //             if (rsbf_rn[k_tree].RangeQuery(l, r))
        //             {
        //                 FPR++;
        //             }
        //         }
        //         resultData << "," << FPR / FPRQSUM;
        //         printf("RSBF_RN_%d\trange2^%d FPR is %lf\n", 1 << k_tree, rsize, FPR / FPRQSUM);
        //     }
        //     resultData << endl;
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_RP_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         FPR = 0, FPRQSUM = 0;
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             if (query_map[rsize][querys[i]])
        //             {
        //                 if (!rsbf_rp[k_tree].RangeQuery(l, r))
        //                 {
        //                     cout << "range Query error";
        //                     exit(-1);
        //                 }
        //                 continue;
        //             }
        //             FPRQSUM++;
        //             if (rsbf_rp[k_tree].RangeQuery(l, r))
        //             {
        //                 FPR++;
        //             }
        //         }
        //         resultData << "," << FPR / FPRQSUM;
        //         printf("RSBF_RP_%d\trange2^%d FPR is %lf\n", 1 << k_tree, rsize, FPR / FPRQSUM);
        //     }
        //     resultData << endl;
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_LN_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         FPR = 0, FPRQSUM = 0;
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             if (query_map[rsize][querys[i]])
        //             {
        //                 if (!rsbf_ln[k_tree].RangeQuery(l, r))
        //                 {
        //                     cout << "range Query error";
        //                     exit(-1);
        //                 }
        //                 continue;
        //             }
        //             FPRQSUM++;
        //             if (rsbf_ln[k_tree].RangeQuery(l, r))
        //             {
        //                 FPR++;
        //             }
        //         }
        //         resultData << "," << FPR / FPRQSUM;
        //         printf("RSBF_LN_%d\trange2^%d FPR is %lf\n", 1 << k_tree, rsize, FPR / FPRQSUM);
        //     }
        //     resultData << endl;
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     resultData << "RSBF_LP_" + to_string(1 << k_tree);
        //     for (int rsize = 0; rsize <= MAX_RANGE_SIZE; rsize++)
        //     {
        //         FPR = 0, FPRQSUM = 0;
        //         for (uint32_t i = 1; i < rangeQueryNum; i++)
        //         {
        //             uint64_t l = querys[i], r = (querys[i] + (1ll << rsize) -1>querys[i]?querys[i] + (1ll << rsize) -1:-1);
        //             if (query_map[rsize][querys[i]])
        //             {
        //                 if (!rsbf_lp[k_tree].RangeQuery(l, r))
        //                 {
        //                     cout << "range Query error";
        //                     exit(-1);
        //                 }
        //                 continue;
        //             }
        //             FPRQSUM++;
        //             if (rsbf_lp[k_tree].RangeQuery(l, r))
        //             {
        //                 FPR++;
        //             }
        //         }
        //         resultData << "," << FPR / FPRQSUM;
        //         printf("RSBF_LP_%d\trange2^%d FPR is %lf\n", 1 << k_tree, rsize, FPR / FPRQSUM);
        //     }
        //     resultData << endl;
        // }

        double onerate = 0;
        double levelfpr = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rosetta_rn.bf[l].counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += rosetta_rn.bf[l].array[0][i] & 1;
        //             rosetta_rn.bf[l].array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rosetta_rn.bf[l].bit_per_row;
        //     // levelfpr = (double)levelFPR[l] / levelQuery[l];
        //     // printf("Rosetta\tFPR in level %d is %lf\n", l, levelfpr);
        //     printf("Rosetta_RN\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rosetta_rp.bf[l].counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += rosetta_rp.bf[l].array[0][i] & 1;
        //             rosetta_rp.bf[l].array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rosetta_rp.bf[l].bit_per_row;
        //     // levelfpr = (double)levelFPR[l] / levelQuery[l];
        //     // printf("Rosetta\tFPR in level %d is %lf\n", l, levelfpr);
        //     printf("Rosetta_RP\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rosetta_ln.bf[l].counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += rosetta_ln.bf[l].array[0][i] & 1;
        //             rosetta_ln.bf[l].array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rosetta_ln.bf[l].bit_per_row;
        //     // levelfpr = (double)levelFPR[l] / levelQuery[l];
        //     // printf("Rosetta\tFPR in level %d is %lf\n", l, levelfpr);
        //     printf("Rosetta_LN\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rosetta_lp.bf[l].counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += rosetta_lp.bf[l].array[0][i] & 1;
        //             rosetta_lp.bf[l].array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rosetta_lp.bf[l].bit_per_row;
        //     // levelfpr = (double)levelFPR[l] / levelQuery[l];
        //     // printf("Rosetta\tFPR in level %d is %lf\n", l, levelfpr);
        //     printf("Rosetta_LP\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // // double RBMfpr = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < rbm.sbf.counter_num; i++)
        //         {
        //             for (int j = 0; j < 32; j++)
        //             {
        //                 onerate += (rbm.sbf.array[0][i] & 1);
        //                 rbm.sbf.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= rbm.sbf.bit_per_row;
        //     }
        //     // RBMfpr = (double)RBMFPR[l] / RBMQuery[l];
        //     // printf("RBM\tFPR in level %d is %lf\n", l, RBMfpr);
        //     printf("RBM\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < rbm_rn.sbf.counter_num; i++)
        //         {
        //             for (int j = 0; j < 32; j++)
        //             {
        //                 onerate += (rbm_rn.sbf.array[0][i] & 1);
        //                 rbm_rn.sbf.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= rbm_rn.sbf.bit_per_row;
        //     }
        //     printf("RBM_RN\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < rbm_rn_simd.sbf_simd.counter_num; i++)
        //         {
        //             for (int j = 0; j < 8; j++)
        //             {
        //                 onerate += (rbm_rn_simd.sbf_simd.array[0][i] & 1);
        //                 rbm_rn_simd.sbf_simd.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= rbm_rn_simd.sbf_simd.bit_per_row;
        //     }
        //     printf("RBM_RN_SIMD\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < rbm_rp.sbf.counter_num; i++)
        //         {
        //             for (int j = 0; j < 32; j++)
        //             {
        //                 onerate += (rbm_rp.sbf.array[0][i] & 1);
        //                 rbm_rp.sbf.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= rbm_rp.sbf.bit_per_row;
        //     }
        //     printf("RBM_RP\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < rbm_rp_simd.sbf_simd.counter_num; i++)
        //         {
        //             for (int j = 0; j < 8; j++)
        //             {
        //                 onerate += (rbm_rp_simd.sbf_simd.array[0][i] & 1);
        //                 rbm_rp_simd.sbf_simd.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= rbm_rp_simd.sbf_simd.bit_per_row;
        //     }
        //     printf("RBM_RP_SIMD\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < rbm_ln.sbf.counter_num; i++)
        //         {
        //             for (int j = 0; j < 32; j++)
        //             {
        //                 onerate += (rbm_ln.sbf.array[0][i] & 1);
        //                 rbm_ln.sbf.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= rbm_ln.sbf.bit_per_row;
        //     }
        //     printf("RBM_LN\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 64; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < rbm_ln_simd.sbf_simd.counter_num; i++)
        //         {
        //             for (int j = 0; j < 8; j++)
        //             {
        //                 onerate += (rbm_ln_simd.sbf_simd.array[0][i] & 1);
        //                 rbm_ln_simd.sbf_simd.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= rbm_ln_simd.sbf_simd.bit_per_row;
        //     }
        //     printf("RBM_LN_SIMD\tOneRate in level %d is %lf\n", l, onerate);
        // }

        // onerate = 0;
        // for (int l = 0; l < 1; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < rbm_lp.sbf.counter_num; i++)
        //         {
        //             for (int j = 0; j < 32; j++)
        //             {
        //                 onerate += (rbm_lp.sbf.array[0][i] & 1);
        //                 rbm_lp.sbf.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= rbm_lp.sbf.bit_per_row;
        //     }
        //     printf("RBM_LP\tOneRate in level %d is %lf\n", l, onerate);
        // }

        onerate = 0;
        for (int l = 0; l < 1; l++)
        {
            if (l == 0)
            {
                for (int i = 0; i < rbm_lp_simd.sbf_simd.counter_num; i++)
                {
                    for (int j = 0; j < 8; j++)
                    {
                        onerate += (rbm_lp_simd.sbf_simd.array[0][i] & 1);
                        if(i<800||i>=rbm_lp_simd.sbf_simd.counter_num/2&&i<rbm_lp_simd.sbf_simd.counter_num/2+800||i>=rbm_lp_simd.sbf_simd.counter_num-800)
                        {
                            printf("%d",rbm_lp_simd.sbf_simd.array[0][i] & 1);
                        }
                        rbm_lp_simd.sbf_simd.array[0][i] >>= 1;
                    }
                    if ((i+1)%16==0&&(i<800||i>=rbm_lp_simd.sbf_simd.counter_num/2&&i<rbm_lp_simd.sbf_simd.counter_num/2+800||i>=rbm_lp_simd.sbf_simd.counter_num-800))
                    {
                        printf("\n");
                    }
                }
                onerate /= rbm_lp_simd.sbf_simd.bit_per_row;
            }
            printf("RBM_LP_SIMD\tOneRate in level %d is %lf\n", l, onerate);
        }

        // onerate = 0;
        // for (int l = 0; l < 1; l++)
        // {
        //     if (l == 0)
        //     {
        //         for (int i = 0; i < bf.counter_num; i++)
        //         {
        //             for (int j = 0; j < 32; j++)
        //             {
        //                 onerate += (bf.array[0][i] & 1);
        //                 bf.array[0][i] >>= 1;
        //             }
        //         }
        //         onerate /= bf.bit_per_row;
        //     }
        //     printf("BF\tOneRate in level %d is %lf\n", l, onerate);
        // }
        // onerate = 0;
        // for (int i = 0; i < rsbf2.sbf.counter_num; i++)
        // {
        //     for (int j = 0; j < 32; j++)
        //     {
        //         onerate += (rsbf2.sbf.array[0][i] & 1);
        //         rsbf2.sbf.array[0][i] >>= 1;
        //     }
        // }
        // onerate /= rsbf2.sbf.bit_per_row;

        // printf("RSBF_2_old\tOneRate is %lf\n", onerate);

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rsbf[k_tree].sbf.counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += (rsbf[k_tree].sbf.array[0][i] & 1);
        //             rsbf[k_tree].sbf.array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rsbf[k_tree].sbf.bit_per_row;

        //     printf("RSBF_%d\tOneRate is %lf\n", 1 << k_tree, onerate);
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rsbf_rn[k_tree].sbf.counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += (rsbf_rn[k_tree].sbf.array[0][i] & 1);
        //             rsbf_rn[k_tree].sbf.array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rsbf_rn[k_tree].sbf.bit_per_row;

        //     printf("RSBF_RN_%d\tOneRate is %lf\n", 1 << k_tree, onerate);
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rsbf_rp[k_tree].sbf.counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += (rsbf_rp[k_tree].sbf.array[0][i] & 1);
        //             rsbf_rp[k_tree].sbf.array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rsbf_rp[k_tree].sbf.bit_per_row;

        //     printf("RSBF_RP_%d\tOneRate is %lf\n", 1 << k_tree, onerate);
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rsbf_ln[k_tree].sbf.counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += (rsbf_ln[k_tree].sbf.array[0][i] & 1);
        //             rsbf_ln[k_tree].sbf.array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rsbf_ln[k_tree].sbf.bit_per_row;

        //     printf("RSBF_LN_%d\tOneRate is %lf\n", 1 << k_tree, onerate);
        // }

        // for (int k_tree = 1; k_tree <= 4; k_tree *= 2)
        // {
        //     onerate = 0;
        //     for (int i = 0; i < rsbf_lp[k_tree].sbf.counter_num; i++)
        //     {
        //         for (int j = 0; j < 32; j++)
        //         {
        //             onerate += (rsbf_lp[k_tree].sbf.array[0][i] & 1);
        //             rsbf_lp[k_tree].sbf.array[0][i] >>= 1;
        //         }
        //     }
        //     onerate /= rsbf_lp[k_tree].sbf.bit_per_row;

        //     printf("RSBF_LP_%d\tOneRate is %lf\n", 1 << k_tree, onerate);
        // }
        // cout << testt;
    }
    // cout<<rosetta.RangeQuery(10000,200000)<<endl;
    // cout<<rosetta.RangeQuery(1000,2000)<<endl;
    // cout<<rosetta.RangeQuery(10,100)<<endl;
    // cout<<rosetta.RangeQuery(10,10)<<endl;
    // cout<<rosetta.RangeQuery(11,11)<<endl;
    // cout<<rosetta.RangeQuery(12,12)<<endl;
    // cout<<rosetta.RangeQuery(13,13)<<endl;
    // cout<<rosetta.RangeQuery(14,14)<<endl;
    // cout<<rosetta.RangeQuery(15,15)<<endl;

    // cout<<rbm.RangeQuery(10007,10007)<<endl;
    // cout<<rbm.RangeQuery(10000,200000)<<endl;
    // cout<<rbm.RangeQuery(1000,2000)<<endl;
    // cout<<rbm.RangeQuery(10,100000)<<endl;
    // cout<<rbm.RangeQuery(10,10000)<<endl;
    // cout<<rbm.RangeQuery(11,1100)<<endl;
    // cout<<keys[1]<<endl;
}
int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        cout << "error arg : <key_num> <query_num> <BPK> <IF_SELF_ADAPT> <SELF_ADAPT_STEP/STORE_LEVEL>" << endl;
        return 0;
    }
    MAX_ITEM_NUM = atoi(argv[1]);
    rangeQueryNum = atoi(argv[2]);
    BPK = atoi(argv[3]);
    IF_SELF_ADAPT = atoi(argv[4]);
    if (IF_SELF_ADAPT)
    {
        STEP = atoi(argv[5]);
    }
    else
    {
        QL = atoi(argv[5]);
    }
    keys.resize(MAX_ITEM_NUM);
    Lookups.resize(rangeQueryNum);
    querys.resize(rangeQueryNum);
    uppers.resize(rangeQueryNum);
    // srand((unsigned)time(0));
    // srand(990808);
    // rosetta.init(5 * 1e6 * 22* 3.8, 2, 32);
    // rbm.init(5 * 1e6 * 22, 2, 32);

    // test();
    // random_data : 160，128 for 50K     160，100 for 50M
    // normal_data : 106, 22 for 50M(30M)
    uint64_t memory = (uint64_t)MAX_ITEM_NUM * BIT_PER_KEY_ROSETTA;
    // rosetta_rn.init(memory, 2, 64);
    // rosetta_rp.init(memory, 2, 64);
    // rosetta_ln.init(memory, 2, 64);
    // rosetta_lp.init(memory, 3, 64);
    // rosetta_ln.init(memory, 3, 64);
    memory = (uint64_t)MAX_ITEM_NUM * BPK;
    // rbm.init(memory, 2, 64);
    // rbm_rn.init(memory, 2, 64);
    // rbm_rn_simd.init(memory, 2, 64);
    // rbm_rp.init(memory, 2, 64);
    // rbm_rp_simd.init(memory, 2, 64);
    // rbm_ln.init(memory, 2, 64);
    // rbm_ln_simd.init(memory, 2, 64);
    // rbm_lp.init(memory, 3, 64);
    // rbm_lp_simd.init(memory, 3, 64);

    // rsbf2.init(memory, 2, 64);

    // for (int i = 1; i <= 4; i *= 2)
    // {
    //     rsbf[i].init(memory, 2, i, 64);
    // }
    // for (int i = 1; i <= 4; i *= 2)
    // {
    //     rsbf_rn[i].init(memory, 2, i, 64);
    // }
    // for (int i = 1; i <= 4; i *= 2)
    // {
    //     rsbf_rp[i].init(memory, 2, i, 64);
    // }
    // for (int i = 1; i <= 4; i *= 2)
    // {
    //     rsbf_ln[i].init(memory, 2, i, 64);
    // }
    // for (int i = 1; i <= 4; i *= 2)
    // {
    //     rsbf_lp[i].init(memory, 2, i, 64);
    // }
    resultData.open("res.csv");
    // filename = argv[1];
    resultData << filename << endl;
    test();
    // rbm_lp_simd.init(memory, 2, 64);
    // filename = argv[2];
    // resultData << filename << endl;
    // test();
    // rbm_lp_simd.init(memory, 2, 64);
    // filename = argv[3];
    // resultData << filename << endl;
    // test();
    // rbm_lp_simd.init(memory, 2, 64);
    // filename = argv[4];
    // resultData << filename << endl;
    // test();
    // resultData.close();
    // cout << endl
    //      << endl;
    // rosetta.init(5 * 1e6 * 22 * 8, 3, 32);
    // rbm.init(5 * 1e6 * 22 * 2, 2, 32);
    // for (int i = 1; i <= 4; i *= 2)
    // {
    //     rsbf[i].init(5 * 1e6 * 22 * 2, 2, i, 32);
    // }

    // test();

    // cout << endl
    //      << endl;
    // rosetta.init(5 * 1e6 * 22 * 8, 3, 32);
    // rbm.init(5 * 1e6 * 22 * 2, 2, 32);
    // for (int i = 1; i <= 4; i *= 2)
    // {
    //     rsbf[i].init(5 * 1e6 * 22 * 2, 2, i, 32);
    // }

    // test();

    // bf.init(1<<22, 4);
    // bf.insert(10);
    // bf.insert(100);
    // cout<<bf.query(100)<<endl;
    // cout<<bf.query(1000)<<endl;
    // rosetta.Insert(4228885784UL);
    // rosetta.Insert(10);
    // cout<<rosetta.RangeQuery(10,10)<<endl;
    // cout<<rosetta.RangeQuery(1000,2000)<<endl;
    // cout<<rosetta.RangeQuery(10,100)<<endl;
    // rbm.Insert(130);
    // rbm.Insert(10);
    // cout<<rbm.RangeQuery(101,131)<<endl;
    // cout<<rbm.RangeQuery(1000,2000)<<endl;
    // cout<<rbm.RangeQuery(10,100)<<endl;

    return 0;
}