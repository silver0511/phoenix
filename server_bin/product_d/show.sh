#!/bin/bash
## This is ProcessGroud Program
ps aux | grep '\<tcpcomet.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<wscomet.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<httpcomet.d.1.1.100\>'  |grep -v grep

ps aux | grep '\<regserver.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<userserver.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<scserver.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<gcserver.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<groupserver.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<fdserver.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<ecserver.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<offcialserver.d.1.1.100\>'  |grep -v grep

ps aux | grep '\<scroute.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<gcroute.d.1.1.100\>'  |grep -v grep
ps aux | grep '\<pcroute.d.1.1.100\>'  |grep -v grep