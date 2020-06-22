#!/bin/sh

indent -kr -l210 -bli0 -di1 -i2 -bbo -nip -lp -nbad -nsai -bl -bls -nut  ./*.c
gcc ./voltage.c -O1 -llxi -Wall -D_GNU_SOURCE -o ./2450_cal_voltage

