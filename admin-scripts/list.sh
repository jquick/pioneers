#!/bin/dash
#
# Lists all running games:
# PID Port Game
ps -edaf | awk '$8 ~ "pioneers-server-console" { printf "%5d %s %s\n", $2+0, $21, $11 }' | sort -k 2
