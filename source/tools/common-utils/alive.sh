#!/bin/bash

NETWORK=$1

for HOST in $(seq 1 254)
do
    ping -c1 -w1 $NETWORK.$HOST &>/dev/null && result=0 || result=1
        if [ "$result" == 0 ];then
            echo -e "\033[32;1m$NETWORK.$HOST is up \033[0m"
            echo "$NETWORK.$HOST" >> /tmp/up.txt
        else
            echo -e "\033[;31m$NETWORK.$HOST is down \033[0m"
            echo "$NETWORK.$HOST" >> /tmp/down.txt
        fi
done

