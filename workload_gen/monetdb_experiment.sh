#
# a fanthrough of multiple iterations of monetdb experiments
# contact: wilson qin
#

BASE=$1
INCR_POW_TWO=$2

# entries are bytes
VAL_SIZE=512

# remove the previously generated data dirs
rm -r data_ft_range_*
rm -r monetdb_data_ft_range_*



#     //argv[1] : entries_num
#          //argv[2] : entries_length
#               //argv[3] : no_of_queries
#                    //argv[4] : max_range_size
#                         //argv[5] : the ratio between point/range queries
#                              //argv[6] : data_dist_type
#                                   //argv[7] : query_dist_type

BEGIN_RG_SZE=$1
POWS_OF_TWO=$2
EMAIL_ME=${3:-""}

# use this as identifier for the run
RUN_ID=`date`

if [ ! -z "$EMAIL_ME" ]; then
    echo "RUN STARTED: $RUN_ID. Begin at: $BEGIN_RG_SZE, iterations: $POWS_OF_TWO." | mail -s "run on $HOSTNAME" $EMAIL_ME
fi

echo "experiment begin with $BEGIN_RG_SZE,$POWS_OF_TWO"

# args 1,2,3 for workload.cpp
TOT_KEYS=100000000
ENTRY_LEN=${VAL_SIZE}
TOT_QUERIES=1000

# MAX_RG_SZE_ARR=("0.0001" "0.0003" "0.001" "0.003" "0.01" "0.03" "0.1" "0.3")
# MAX_RG_SZE_ARR=("0.00000002" "0.00000004" "0.00000008" "0.00000016" "0.00000032" "0.000001" "0.00001" "0.0001" "0.001" "0.01" "0.1")
MAX_RG_SZE_ARR=(2 4 8 16 32 64 128 256 512 1000 2000 4000 10000 100000 1000000 10000000 30000000 50000000 70000000 90000000)

# currently looped through as arg 5
#MAX_RG_SZE=$CUR_MAX_RG

# args 6,7,8
RATIO_PT_RG=0.0
DATA_DIST=uniform
QUERY_DIST=uniform

# also arg 9 can be a correlation degree
# currently in workload.cpp it has a default value of 1.0 if not provided

# generate all the data directories by range of the queries
# for ((i=0; i<=POWS_OF_TWO; i++)); do
#     MAX_RG_SZE=$(($BEGIN_RG_SZE * 2**$i))
for MAX_RG_SZE in "${MAX_RG_SZE_ARR[@]}"; do
    echo $MAX_RG_SZE
    ./workload ${TOT_KEYS} ${ENTRY_LEN} ${TOT_QUERIES} ${BFS_DIFF_LEVEL} ${MAX_RG_SZE} ${RATIO_PT_RG} ${DATA_DIST} ${QUERY_DIST}

    # data is now in my_data

    mv my_data data_ft_range_${MAX_RG_SZE}



    # use the transform_to_monetdb script to convert each data_ft_range_{RNG_SIZE} directory
    # to its monetdb form equivalent (new directory created if does not exist)
    ./transform_to_monetdb.sh $(pwd)/data_ft_range_${MAX_RG_SZE} $(pwd)/monetdb_data_ft_range_${MAX_RG_SZE} $VAL_SIZE

    ./single_monetdb_experiment.sh monetdb_data_ft_range_${MAX_RG_SZE}

    if [ ! -z "$EMAIL_ME" ]; then
        echo "RUN:$RUN_ID. Finished Iteration with $MAX_RG_SZE" | mail -s "run on $HOSTNAME" $EMAIL_ME
    fi

    # remove the previously generated data dirs
    rm -r data_ft_range_*
    rm -r monetdb_data_ft_range_*
done

if [ ! -z "$EMAIL_ME" ]; then
    echo "RUN ENDED: $RUN_ID. Begin at: $BEGIN_RG_SZE, iterations: $POWS_OF_TWO." | mail -s "run on $HOSTNAME" $EMAIL_ME
fi
