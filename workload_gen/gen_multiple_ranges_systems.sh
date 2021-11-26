#     //argv[1] : entries_num
#          //argv[2] : entries_length
#               //argv[3] : no_of_queries
#                    //argv[4] : max_range_size
#                         //argv[5] : the ratio between point/range queries
#                              //argv[6] : data_dist_type
#                                   //argv[7] : query_dist_type

BEGIN_RG_SZE=$1
POWS_OF_TWO=$2

# args 1,2,3 for workload.cpp
TOT_KEYS=10000000
ENTRY_LEN=512
TOT_QUERIES=100000

# currently looped through as arg 5
#MAX_RG_SZE=$CUR_MAX_RG

# args 6,7,8
RATIO_PT_RG=0.0
DATA_DIST=uniform
QUERY_DIST=uniform

# also arg 9 can be a correlation degree
# currently in workload.cpp it has a default value of 1.0 if not provided

for ((i=0; i<=POWS_OF_TWO; i++)); do
    MAX_RG_SZE=$(($BEGIN_RG_SZE * 2**$i))
    echo $MAX_RG_SZE
    ./workload ${TOT_KEYS} ${ENTRY_LEN} ${TOT_QUERIES} ${BFS_DIFF_LEVEL} ${MAX_RG_SZE} ${RATIO_PT_RG} ${DATA_DIST} ${QUERY_DIST}

    # data is now in my_data

    mv my_data data_ft_range_${MAX_RG_SZE}
done
