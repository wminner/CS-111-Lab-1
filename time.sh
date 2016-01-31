#!/bin/bash

make clean
make

NUMTESTS=5
RUNSIMPSH=1
RUNBASH=1
RUNEXECLINE=1

TOTALDIFFS=4
GOODDIFFS=0

# Save stdout and stderr, then redirect to time_results file
exec 3>&1 4>&2 >time_results 2>&1

# TEST 1
printf "*************************TEST 1*************************\n"

#		BASH
if [ $RUNBASH -eq 1 ]; then
	touch z
	for i in {1..10}
	do
	    echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" >> z
	done

	for i in `seq 1 $NUMTESTS`;
	do
                printf "********** BASH TEST 1-%d **********\n" $i
		time cat z | tr A-Z a-z | tr -sd 'b' 'a' > y
		rm y
		/usr/bin/time -f "Max resident set size:\t%M\nBlock Input\t%I\nBlock Output\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary context switches:\t%w\nInvoluntary context switches:\t%c\n" cat z | tr A-Z a-z | tr -sd 'b' 'a' > y
		#rm y  # y used later to diff
	done
fi

#		SIMPSH
if [ $RUNSIMPSH -eq 1 ]; then
	touch e
	touch x
	for i in `seq 1 $NUMTESTS`;
	do
	        printf "********** SIMPSH TEST 1-%d **********\n" $i
		./simpsh --profile --rdonly z --pipe --pipe --wronly x --wronly e --command 0 2 6 cat --command 1 4 6 tr A-Z a-z --command 3 5 6 tr -sd 'b' 'd' --close 2 --close 4 --wait
		printf "\n"
		#rm x  # x used later to diff
	done

	diff -u x y
	if [ $? -eq 0 ]
	then
	    GOODDIFFS=$((GOODDIFFS+1))
	    printf "DIFF RESULTS: No output differences\n"
	    printf "\n"
	fi
	rm e x
	rm z
fi

#               EXECLINE

if [ $RUNEXECLINE -eq 1 ]
then  
    touch z
    for i in {1..10}
    do
	echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" >> z
    done

    for i in `seq 1 $NUMTESTS`;
    do
	printf "********** EXECLINE TEST 1-%d **********\n" $i
	time ./execline1
	rm w
	/usr/bin/time -f "Max Resident Set Size:\t%M\nBlock Input:\t%I\nBlock Output:\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary Context Switches:\t%w\nInvoluntary Context Switches:\t%c\n" ./execline1
	#rm w  # w used later to diff
    done

    diff -u w y
    if [ $? -eq 0 ]
    then
	GOODDIFFS=$((GOODDIFFS+1))
	printf "DIFF RESULTS: No output differences\n"
	printf "\n"
    fi
    rm z y w
fi


#TEST 2
printf "*************************TEST 2*************************\n"

echo "Z
B
D
DW
W
d
g
h
a
e
h
j
u
a
3
46
7
8
F
H
f
G" > a

#		BASH
if [ $RUNBASH -eq 1 ]; then
    for i in `seq 1 $NUMTESTS`;
    do
	printf "********** BASH TEST 2-%d **********\n" $i
	time cat a | tr A-Z a-z | sort > b
	rm b
	/usr/bin/time -f "Max resident set size:\t%M\nBlock Input\t%I\nBlock Output\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary context switches:\t%w\nInvoluntary context switches:\t%c\n" cat a | tr A-Z a-z | sort > b
	#rm b  # b used later to diff
    done
fi

#		SIMPSH
if [ $RUNSIMPSH -eq 1 ]; then
    for i in `seq 1 $NUMTESTS`;
    do
	printf "********** SIMPSH TEST 2-%d **********\n" $i
	touch a
	./simpsh --profile --rdonly a --creat --wronly d --creat --wronly c --pipe --pipe --command 0 4 2 cat --command 3 6 2 tr A-Z a-z --command 5 1 2 sort --close 4 --close 6 --wait
	#rm d  # d used later to diff
	printf "\n"
    done
    
    diff -u b d
    if [ $? -eq 0 ]
    then
	GOODDIFFS=$((GOODDIFFS+1))
	printf "DIFF RESULTS: No output differences\n"
	printf "\n"
    fi
    rm d
fi

#               EXECLINE
if [ $RUNEXECLINE -eq 1 ]
then
    for i in `seq 1 $NUMTESTS`;
    do
	printf "********** EXECLINE TEST 2-%d **********\n" $i
	time ./execline2
	rm f
	/usr/bin/time -f "Max Resident Set Size:\t%M\nBlock Input:\t%I\nBlock Output:\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary Context Switches:\t%w\\nInvoluntary Context Switches:\t%c\n" ./execline2
	#rm f  # f used later to diff
    done
    
    diff -u b f
    if [ $? -eq 0 ]
    then
	GOODDIFFS=$((GOODDIFFS+1))
	printf "DIFF RESULTS: No output differences\n"
	printf "\n"
    fi
    rm a b f
fi


#TEST 3
printf "*************************TEST 3*************************\n"
#		BASH
if [ $RUNBASH -eq 1 ]; then
    for i in `seq 1 $NUMTESTS`;
    do
	printf "********** BASH TEST 3-%d **********\n" $i
	time head -c 1MB /dev/urandom | tr -s A-Z a-z | cat > b
	rm b

	/usr/bin/time -f "Max resident set size:\t%M\nBlock Input\t%I\nBlock Output\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary context switches:\t%w\nInvoluntary context switches:\t%c\n" head -c 1MB /dev/urandom | tr -s A-Z a-z | cat > b
	#rm b  # b used later to diff
    done
fi

#		SIMPSH
if [ $RUNSIMPSH -eq 1 ]; then
    for i in `seq 1 $NUMTESTS`;
    do
	printf "********** SIMPSH TEST 3-%d **********\n" $i
	touch a
	./simpsh --profile --rdonly a --creat --wronly f --creat --wronly c --pipe --pipe --command 0 4 2 head -c 1MB /dev/urandom --command 3 6 2 tr -s A-Z a-z --command 5 1 2 cat --close 4 --close 6 --wait
	rm a c
	printf "\n"
    done

    # Cannot diff binary files...
    # diff -u b f
    # if [ $? -eq 0 ]
    # then
    # 	GOODDIFFS=$((GOODDIFFS+1))
    # 	printf "DIFF RESULTS: No output differences\n"
    # 	printf "\n"
    # fi
    rm f
fi

#               EXECLINE

if [ $RUNEXECLINE -eq 1 ]
then
    for i in `seq 1 $NUMTESTS`;
    do
	printf "********** EXECLINE TEST 3-%d **********\n" $i
	time ./execline3
	rm g
	/usr/bin/time -f "Max Resident Set Size:\t%M\nBlock Input:\t%I\nBlock Output:\t%O\nPage Reclaims:\t%R\nPage Faults:\t%F\nVoluntary Context Switches:\t%w\nInvoluntary Context Switches:\t%c\n" ./execline3
	#rm g  # g used later to diff
    done

    # Cannot diff binary files...
    # diff -u b g
    # if [ $? -eq 0 ]
    # then
    # 	GOODDIFFS=$((GOODDIFFS+1))
    # 	printf "DIFF RESULTS: No output differences\n"
    # 	printf "\n"
    # fi
    rm b g
fi

# Restore stdout and stderr
exec 1>&3 2>&4

if [ $GOODDIFFS -eq $TOTALDIFFS ]
then
    printf "All diffs came out clean!\n"
else
    printf "Some diffs came out dirty? See \'time_results\' for more information.\n"
fi

printf "Performance data in file \'time_results\'.\n"



