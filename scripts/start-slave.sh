#!/bin/bash

workDir="$1"
binDir="$2"
testCase="$3"
masterHost="$4"
masterPort="$5"
slaveID="$6"
nodeName="$7"
slaveNum="$8"
totalSlaveNum="$9"

cd "$workDir"

for ((i=0; i<$slaveNum; i++)); do
  slaveName="slave-${nodeName}-${slaveID}"
  (nohup $binDir/slave --config $testCase --master_host $masterHost --master_port $masterPort --slave_id $slaveID --slave_name $slaveName --slave_num $totalSlaveNum > /dev/null) &
  echo -e "start $slaveName"
  slaveID=`expr $slaveID + 1`
done