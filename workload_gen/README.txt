1. To generate workload, just run `make`.
2. Make will create the executable `workload`.
3. If you run it by `./workload`, you'll see a folder called my_data has been created.
4. Inside my_data,
	a) data.txt contains the keys of the data. note there are no values of the data explicitly generated here.
	b) txn.txt contains the keys indicating the start of the range. LOWER END
	c) upper_bound.txt contains the keys indicating the end of the range. HIGHER END
5. It is done this way to interface well with SuRF.

Monetdb Experiment Instructions and pre-reqs

For debian/ubuntu install check: https://www.monetdb.org/downloads/deb/

To facilitate no password prompts for monetdb experiment trials, you'll need to create a ~/.monetdb file
with following lines. Where monetdb is the monetdb user and password you want to use.
Within the brackets below are the default:
user=<monetdb>
password=<monetdb>
language=sql

To run Monetdb transform:
1. Make a regular generated workload dataset. We will refer to that's folder as $GEN_DATA_FOLDER
2. run `./transform_to_monetdb.sh $GEN_DATA_FOLDER $OUTPUT_FOLDER_PATH` .
this will take an already generated kv data set and convert to monet style sql.

To run a monetdb query:
1. monetdbd create PATH_TO_DB
2. monetdbd start PATH_TO_DB
3. monetdb create DBNAME
4. monetdb release DBNAME
5. mclient -u monetdb -d DBNAME
6. default password usually monetdb
7. to quit \q

To run monetdb experiments, we have two shell scripts
monetdb_experiments.sh the top level setup&runner, and a single trial runner single_experiment_monetdb.sh. 
Note data generation happens per trial