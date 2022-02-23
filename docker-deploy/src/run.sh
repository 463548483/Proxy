#!/bin/bash
make clean

make

while true
do 
	./proxy 12345
	sleep 1 
done