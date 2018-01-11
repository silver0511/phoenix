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
BuildBin  'regserver' '../server_logic/regserver/linux_prj'
BuildBin  'userserver' '../server_logic/userserver/linux_prj'
BuildBin  'scserver' '../server_logic/scserver/linux_prj'
BuildBin  'gcserver' '../server_logic/gcserver/linux_prj'
BuildBin  'groupserver' '../server_logic/groupserver/linux_prj'
BuildBin  'fdserver' '../server_logic/fdserver/linux_prj'
BuildBin  'ecserver' '../server_logic/ecserver/linux_prj'
BuildBin  'offcialserver' '../server_logic/offcialserver/linux_prj'
#-----------end--------------------------------

#end display info
ENDTIME=`date +%s`
duration=$(($ENDTIME-$ALLTIMEBEGIN))


echo  "==================logic server build duration" $duration "seconds=================="
for((i=0; i<$PRJCOUNT;   i++))
do
  echo  ${PRJLIST[$i]} durate time ${DURATIONTIMELIST[$i]} seconds
done
echo  "==================logic server build duration" $duration "seconds=================="
fi
