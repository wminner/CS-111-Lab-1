#!/bin/bash
NUMTESTS=1
RUNSIMPSH=0
RUNBASH=1
# TEST 1
printf "*************************TEST 1*************************\n"

#		BASH
if [ $RUNBASH -eq 1 ]; then
	touch z
	for i in {1..10}
	do
	    echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" >> z
	done

	for i in {1..$NUMTESTS}
	do
		time cat z | tr A-Z a-z | tr -sd 'b' 'a' > y
		rm y
		/usr/bin/time -f "Page Faults:\t%F\nMax resident set size:\t%M\nBlock Input\t%I\nBlock Output\t%O\nPage Reclaims:\t%R\nVoluntary context switches:\t%w\nInvoluntary context switches:\t%c\n" cat z | tr A-Z a-z | tr -sd 'b' 'a' > y
		rm y
	done
fi

#		SIMPSH
if [ $RUNSIMPSH -eq 1 ]; then
	touch e
	touch y
	for i in {1..$NUMTESTS}
	do
		./simpsh --profile --rdonly z --pipe --pipe --wronly y --wronly e --command 0 2 6 cat --command 1 4 6 tr A-Z a-z --command 3 5 6 tr -sd 'b' 'd' --close 2 --close 4 --wait
	done

	rm e y
	rm z
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
	for i in {1..$NUMTESTS}
	do
		time cat a | tr A-Z a-z | sort > b
		rm b
		/usr/bin/time -f "Page Faults:\t%F\nMax resident set size:\t%M\nBlock Input\t%I\nBlock Output\t%O\nPage Reclaims:\t%R\nVoluntary context switches:\t%w\nInvoluntary context switches:\t%c\n" cat a | tr A-Z a-z | cat > b
		rm b
	done
fi

#		SIMPSH
if [ $RUNSIMPSH -eq 1 ]; then
	for i in {1..$NUMTESTS}
	do
		touch a
		./simpsh --profile --rdonly a --creat --wronly b --creat --wronly c --pipe --pipe --command 0 4 2 cat --command 3 6 2 tr A-Z a-z --command 5 1 2 sort --close 4 --close 6 --wait
		# rm  b
	done
fi
rm a

#TEST 3
printf "*************************TEST 3*************************\n"
#		BASH
if [ $RUNBASH -eq 1 ]; then
	for i in {1..$NUMTESTS}
	do
		time head -c 1MB /dev/urandom | tr -s A-Z a-z | cat > b
		rm b

		/usr/bin/time -f "Page Faults:\t%F\nMax resident set size:\t%M\nBlock Input\t%I\nBlock Output\t%O\nPage Reclaims:\t%R\nVoluntary context switches:\t%w\nInvoluntary context switches:\t%c\n" head -c 1MB /dev/urandom | tr -s A-Z a-z | cat > b
		rm b
	done
fi
#		SIMPSH
if [ $RUNSIMPSH -eq 1 ]; then
	for i in {1..NUMTESTS}
	do
		touch a
		./simpsh --profile --rdonly a --creat --wronly b --creat --wronly c --pipe --pipe --command 0 4 2 head -c 1MB /dev/urandom --command 3 6 2 tr -s A-Z a-z --command 5 1 2 cat --close 4 --close 6 --wait
		rm a b c
	done
fi