#!/bin/bash
while true; do
  for I in {100..105};do
    ping -c 2 -w 2 192.168.1.$I &>/dev/null
    if [ $? -eq 0 ];then
      echo -e "\033[32;40m 192.168.1.$I is UP.\033[0m"
    else
      echo -e "\033[32;40m 192.168.1.$I is DOWN.\033[0m"
    fi
  done
    break
done
