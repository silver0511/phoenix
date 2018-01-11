#!/bin/bash
#判断参数个数
if [ $# -eq 0 ]
then
echo "Usage $0 [start|restart|stop]"
exit 1
fi
#如果传入参数，把传入参数做为java应用启动时参数
CMD_ARGS="";
if [ $# -gt 0 ]
then
COUNT=0
for i in $@; do
        if [ $COUNT -gt 0 ]
        then
        CMD_ARGS="$CMD_ARGS $i"
        fi
        let COUNT+=1
done
fi
if [ "$CMD_ARGS" = "" ]
then
CMD_ARGS="-Xms3G -Xmx5G -Xss256k"
fi


case $1 in
        start)
        ps=`ps -fe |grep jar|grep im_server|grep -v grep|tr -s ' '|awk -F' ' '{print $2}'`
        if [ -n "$ps" ]
        then
        	echo "dbserver process is starting pid=$ps , please first stop!"
        else
        	echo "CMD_ARGS=$CMD_ARGS"
        	echo "dbserver process is starting"
        	nohup java $CMD_ARGS -jar im_server-1.0-SNAPSHOT.jar &
        	sleep 2
        	ps=`ps -fe |grep jar|grep im_server|grep -v grep|tr -s ' '|awk -F' ' '{print $2}'`
        	if [ -n "$ps" ]
        	then
        		echo "pid=$ps , please use"
        		echo "tail -200f /var/log/nim.db.server/server.log"
        		echo "to check log file!"
        	fi
        fi
        ;;
        restart)
        ps=`ps -fe |grep jar|grep im_server|grep -v grep|tr -s ' '|awk -F' ' '{print $2}'`
        if [ -n "$ps" ]
        then
        	kill -9 $ps
        	echo "killed dbserver pid:$ps"
        fi
        echo "CMD_ARGS=$CMD_ARGS"
        echo "dbserver process is starting"
        nohup java $CMD_ARGS -jar im_server-1.0-SNAPSHOT.jar &
        sleep 2
        ps=`ps -fe |grep jar|grep im_server|grep -v grep|tr -s ' '|awk -F' ' '{print $2}'`
        if [ -n "$ps" ]
        then
        	echo "pid=$ps , please use"
        	echo "tail -200f /var/log/nim.db.server/server.log"
        	echo "to check log file!"
        fi
        ;;
        stop)
        ps=`ps -fe |grep jar|grep im_server|grep -v grep|tr -s ' '|awk -F' ' '{print $2}'`
        if [ -n "$ps" ]
        then
        kill -9 $ps
        echo "dbserver process  is stoped with pid=$ps!"
        else
        echo "dbserver process  already stoped!"
        fi
        ;;
        *)
        echo "I do not know where to go..."
        ;;
esac
