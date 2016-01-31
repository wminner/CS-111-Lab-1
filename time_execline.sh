#!/bin/bash

# Test 1

touch z
for i in {1..10}
do
    echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" >> z
done

TESTNUM=0
for i in {1..5}
do
    TESTNUM=$((TESTNUM+1))
    printf "********** TEST 1-%d **********\n" $TESTNUM
    time ./execline1
    rm y c
    /usr/bin/time -f "Max Resident Set Size:\t%M\nBlock Input:\t%I\nBlock Output:\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary Context Switches:\t%w\nInvoluntary Context Switches:\t%c\n" ./execline1
    rm y c
done


# Test 2

TESTNUM=0
for i in {1..5}
do
    TESTNUM=$((TESTNUM+1))
    printf "********** TEST 2-%d **********\n" $TESTNUM
    time ./execline2
    rm b c
    /usr/bin/time -f "Max Resident Set Size:\t%M\nBlock Input:\t%I\nBlock Output:\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary Context Switches:\t%w\nInvoluntary Context Switches:\t%c\n" ./execline2
    rm b c
done


# Test 3

TESTNUM=0
for i in {1..5}
do
    TESTNUM=$((TESTNUM+1))
    printf "********** TEST 3-%d **********\n" $TESTNUM
    time ./execline3
    rm b
    /usr/bin/time -f "Max Resident Set Size:\t%M\nBlock Input:\t%I\nBlock Output:\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary Context Switches:\t%w\nInvoluntary Context Switches:\t%c\n" ./execline3
    rm b
done
