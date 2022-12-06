PROG_NAME=lab4
HEADER_NAME=myapp
CPP=$PROG_NAME.cc
HEADER=$HEADER_NAME.h
NS3_PATH=/home/adhoc/ns-allinone-3.24/ns-3.24

CURRENT_PATH=$(pwd)

cp $CPP $NS3_PATH/scratch/$CPP
cp $HEADER $NS3_PATH/scratch/$HEADER
cd $NS3_PATH
./waf --run $PROG_NAME

cp *.pcap $CURRENT_PATH/pcap/