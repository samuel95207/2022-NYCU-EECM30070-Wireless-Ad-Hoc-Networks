PROG_NAME=Lab3-1
CPP=$PROG_NAME.cc
NS3_PATH=/home/adhoc/ns-allinone-3.24/ns-3.24

cp $CPP $NS3_PATH/scratch/$CPP
cd $NS3_PATH
./waf --run $PROG_NAME