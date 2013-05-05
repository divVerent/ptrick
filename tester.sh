#!/bin/sh

set -ex

gcc ptrick.c -o ptrick -Wall -Wextra
gcc race1.c -o race1 -Wall -Wextra
gcc race2.c -o race2 -static -Wall -Wextra

set +x -- starting switcher
while :; do
	ln -f race1 race
	ln -f race2 race
done & switcherpid=$!
set -x

n=0
nmax=1000
set +x -- running loop $nmax times
while [ $n -lt $nmax ]; do
	./ptrick ./race 2>&1 | xargs echo || true # condense output to a single line
	n=$(($n+1))
done | sort | uniq -c
set -x

kill $switcherpid
