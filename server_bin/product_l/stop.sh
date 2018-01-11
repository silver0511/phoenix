#!/bin/bash
## This is ProcessGroud Program
kill -9 `ps aux | grep '\<tcpcomet.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<wscomet.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<httpcomet.l.1.1.100\>' | grep -v grep | awk {'print $2'}`

kill -9 `ps aux | grep '\<regserver.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<userserver.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<scserver.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<gcserver.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<groupserver.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<fdserver.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<ecserver.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<offcialserver.l.1.1.100\>' | grep -v grep | awk {'print $2'}`

kill -9 `ps aux | grep '\<gcroute.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
for pidid in `ps aux | grep '\<scroute.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
do
    kill -9 $pidid
done

for pidid in `ps aux | grep '\<pcroute.l.1.1.100\>' | grep -v grep | awk {'print $2'}`
do
    kill -9 $pidid
done
