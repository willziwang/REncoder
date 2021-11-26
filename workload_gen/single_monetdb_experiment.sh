#
# a single iteration of a monetdb experiment
# contact: wilson qin
#

DBNAME="diffident"
EXP_PATH=$1

OPTIONS="-a"

# monetdb create $DBNAME
# monetdb release $DBNAME

# load.sql
LOAD_TS_START=`date +%s`
mclient -d $DBNAME $OPTIONS < $1/load.sql
LOAD_TS_END=`date +%s`
l_diff="$(($LOAD_TS_END-$LOAD_TS_START))"

Q_TS_START=`date +%s`
mclient -d $DBNAME $OPTIONS < $1/queries.sql >> /dev/null
Q_TS_END=`date +%s`
q_diff="$(($Q_TS_END-$Q_TS_START))"

echo "$LOAD_TS_START,$LOAD_TS_END,$l_diff,$Q_TS_START,$Q_TS_END,$q_diff"

# cleanup of monetdb db, with force flag
# monetdb destroy $DBNAME -f