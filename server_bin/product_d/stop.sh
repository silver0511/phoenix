#!/bin/bash
## This is ProcessGroud Program
kill -9 `ps aux | grep '\<tcpcomet.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<wscomet.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<httpcomet.d.1.1.100\>' | grep -v grep | awk {'print $2'}`

kill -9 `ps aux | grep '\<regserver.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<userserver.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<scrserver.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<gcserver.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<fdserver.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<ecserver.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<groupserver.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
kill -9 `ps aux | grep '\<offcialserver.d.1.1.100\>' | grep -v grep | awk {'print $2'}`

kill -9 `ps aux | grep '\<gcroute.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
for pidid in `ps aux | grep '\<scroute.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
do
    kill -9 $pidid
done

for pidid in `ps aux | grep '\<pcroute.d.1.1.100\>' | grep -v grep | awk {'print $2'}`
do
    kill -9 $pidid
done

