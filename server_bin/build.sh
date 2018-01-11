#!/usr/bin/env bash

COMPILEVER=$1

date
if [ -z $COMPILEVER ];
then
    echo -e "Error need a Param  --> Usage ./build_cs.sh  [debug] or [release] or [release_log] -----------------------"
else
#-----------begin --------------------------------
sh ../common/build.sh $COMPILEVER
sh ../network/build.sh $COMPILEVER
sh ../public/build.sh $COMPILEVER
sh ../server_comet/build.sh $COMPILEVER
sh ../server_logic/build.sh $COMPILEVER
sh ../server_route/build.sh $COMPILEVER
#-----------end--------------------------------
fi

