#!/bin/bash
# contact: Wilson Qin

# the directory of data already generated. to transform. abide by readme.txt
DATADIR=$1
# where to output
OUTPUT_DATADIR=$2
# length of a value in bytes. default is 64 len char string
VALS_SIZE=${3:-64}

IN_KEYS="${DATADIR}/data.txt"
IN_RNG_START="${DATADIR}/txn.txt"
IN_RNG_END="${DATADIR}/upper_bound.txt"

# CREATE OUR DEFAULT DUMMY VALUE STRING FOR WORKLOAD
# dynamically set based on VALS_SIZE
DEFAULT_VAL=$(head -c ${VALS_SIZE} < /dev/zero | tr '\0' '\141')

### Check if a directory does not exist ###
if [ ! -d $OUTPUT_DATADIR ]
then
    echo "making output dir because does not exist: ${OUTPUT_DATADIR}"
    mkdir -p ${OUTPUT_DATADIR}
fi

# make the csv for the monetdb table
# spre=$(echo "k,v" > ${OUTPUT_DATADIR}/table.csv)
s=$(awk -v val="${DEFAULT_VAL}" -vOFS='' '{print $0,",",val}' ${IN_KEYS} > ${OUTPUT_DATADIR}/table.csv)
# [ -d $spre ]
[ -d $s ]  || (echo "error: trouble creating the monet CSV data at:  ${OUTPUT_DATADIR}/table.csv" && exit)

echo "success: created monet CSV data file"


# the table name we want to create and load in monet
TABLE_NAME="tbl"

OUT_LOAD=${OUTPUT_DATADIR}/load.sql

# build up the load sql script
s=$(echo "DROP TABLE IF EXISTS ${TABLE_NAME} ; " > ${OUT_LOAD})
# note this got changed to int for `k` BIGINT -- 64 bit
# used to be HUGEINT (128 bit signed)
s2=$(echo "CREATE TABLE ${TABLE_NAME} (k BIGINT, v STRING) ;" >> ${OUT_LOAD})
s3=$(echo "COPY INTO tbl FROM '${OUTPUT_DATADIR}/table.csv' USING DELIMITERS ',' ; \q" >> ${OUT_LOAD})
stot=$s && $s2 && $s3

[ -d $stot ] || (echo "error: trouble creating the monet Load SQL at:  ${OUT_LOAD}" && exit)

echo "success: created monet Load SQL file"

# make the range queries for the SQL queries
s=$(paste ${IN_RNG_START} ${IN_RNG_END} > ${OUTPUT_DATADIR}/queries.tmp.txt)

[ -d $s ] || (echo "error: trouble creating the temporary queries translation file" && exit)

# use awk with field separator single-space, then produce a low range to high range sql query per line of the input range file tmp
# then output as a sql script
#s=$(awk -F " " '{print $0, "SELECT k,v FROM \"${TABLE_NAME}\" WHERE k > ", $0, " AND k < ", $1, " ;"}' ${OUTPUT_DATADIR}/queries.tmp.txt > ${OUTPUT_DATADIR}/queries.sql)
#s=$(awk -v t="${TABLE_NAME}" -F " " '{print "SELECT count(k)  FROM ", t, " WHERE k > ", $1, " AND k < ", $2, " ;"}' ${OUTPUT_DATADIR}/queries.tmp.txt > ${OUTPUT_DATADIR}/queries.sql)
s=$(awk -v t="${TABLE_NAME}" -F " " '{print "SELECT sum(k)  FROM ", t, " WHERE k > ", $1, " AND k < ", $2, " ;"}' ${OUTPUT_DATADIR}/queries.tmp.txt > ${OUTPUT_DATADIR}/queries.sql)


[ -d $s ] || (echo "error: trouble creating the queries.sql translated file" && exit)

s=$(rm ${OUTPUT_DATADIR}/queries.tmp.txt)
[ -d $s ] || echo "error: trouble deleting the temporary queries translation file. non-fatal, this script will continue."

echo "script finished."

