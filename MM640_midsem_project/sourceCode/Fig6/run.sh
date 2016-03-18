#!/bin/bash
#echo 'Compiling the main program......'

./generateData.sh

gcc main.c -lgsl -lgslcblas -lfftw3 -lm
#echo 'Compilation complete......'
#echo 'Executing the main program......'
./a.out
#echo 'Execution complete......'
#echo 'Ploting the data......'
gnuplot --persist <plotFig6.gp
