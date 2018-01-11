#!/usr/bin/env bash
PRJCOUNT=0
COMPILEVER=$1

PRJLIST[0]=''
DURATIONTIMELIST[0]=0

function BuildBin()
{
echo $1 Prject Building
cd $2
echo $COMPILEVER
pwd
BEGINTIME=`date +%s`
make clean
make COMP_VER=$COMPILEVER
make install COMP_VER=$COMPILEVER
cd -
ENDTIME=`date +%s`
DURATIONTIMELIST[$PRJCOUNT]=$(($ENDTIME-$BEGINTIME))
PRJLIST[$PRJCOUNT]=$1

echo $1 Buld Duration ${DURATIONTIMELIST[$PRJCOUNT]} sec
PRJCOUNT=$PRJCOUNT+1
} 

#start display info
ALLTIMEBEGIN=`date +%s`
echo Start build
date 
if [ -z $COMPILEVER ];
then
    echo -e "Error need a Param  --> Usage ./build_cs.sh  [debug] or [release] or [release_log] -----------------------"
else
#-----------begin --------------------------------
BuildBin  'tcpcomet' '../server_comet/tcpcomet/linux_prj'
BuildBin  'wscomet' '../server_comet/wscomet/linux_prj'
BuildBin  'httpcomet' '../server_comet/httpcomet/linux_prj'
#-----------end--------------------------------

#end display info
ENDTIME=`date +%s`
duration=$(($ENDTIME-$ALLTIMEBEGIN))


echo  "==================comet server build duration" $duration "seconds=================="
for((i=0; i<$PRJCOUNT;   i++))  
do  
  echo  ${PRJLIST[$i]} durate time ${DURATIONTIMELIST[$i]} seconds
done
echo  "==================comet server build duration" $duration "seconds=================="
fi

