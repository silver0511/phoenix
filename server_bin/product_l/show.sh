#!/bin/bash
## This is ProcessGroud Program
ps aux | grep '\<tcpcomet.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<wscomet.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<httpcomet.l.1.1.100\>'  |grep -v grep

ps aux | grep '\<regserver.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<userserver.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<scserver.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<gcserver.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<groupserver.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<fdserver.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<ecserver.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<offcialserver.l.1.1.100\>'  |grep -v grep

ps aux | grep '\<scroute.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<gcroute.l.1.1.100\>'  |grep -v grep
ps aux | grep '\<pcroute.l.1.1.100\>'  |grep -v grep