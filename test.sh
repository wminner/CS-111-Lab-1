#!/bin/bash
# If diffs come up clean, then tests have passed, and this will be outputed on
# screen. There will be some output from 'make clean', 'make', and '--abort'.

make clean
make

TESTNUM=10
PASSNUM=0

# Test 1 - cat
echo "This is file a" > a
echo "This is file b" > b
echo "This is file c" > c
printf "This is file a\nThis is file c\n" > d

./simpsh --rdonly a --wronly b --wronly c --command 0 1 2 cat a --command 0 1 2 cat c --test

diff -u b d

if [ $? -eq 0 ]
then
    echo "Test 1/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 1/$TESTNUM failed!?"
fi

# Test 2 - sort (and single dash options)
printf "4\n 3\n2\n1\n" > a
echo "This is file b" > b
echo "This is file c" > c
sort a -b -r | cat - c > d
printf "0 sort -b -r \n0 cat c \n" > f

(./simpsh --rdonly a --wronly b --wronly c --command 0 1 2 sort -b -r --command 0 1 2 cat c --wait)> e

diff -u b d
DIFF1=$?

diff -u e f
DIFF2=$?

rm e f

if [ $DIFF1 -eq 0 -a $DIFF2 -eq 0 ]
then
    echo "Test 2/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 2/$TESTNUM failed!?"
fi

# Test 3 - tr
printf "ABC XYZ 123 789" > a
echo "This is file b" > b
echo "This is file c" > c
cat a | tr A-Z a-z > d

./simpsh --rdonly a --wronly b --wronly c --command 0 1 2 tr A-Z a-z --test

diff -u b d

if [ $? -eq 0 ]
then
    echo "Test 3/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 3/$TESTNUM failed!?"
fi

# Test 4 - oflags
echo "This is file a" > a
echo "This is file c" > c
rm b
cat a | cat - a > d

./simpsh --rdonly a --creat --append --wronly b --wronly c --command 0 1 2 cat a --command 0 1 2 cat a --test

diff -u b d

if [ $? -eq 0 ]
then
    echo "Test 4/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 4/$TESTNUM failed!?"
fi

# Test 5 - rdwr
echo "This is file a" > a
echo "This is file b" > b

cat a | cat - a > d

./simpsh --append --rdwr a --rdwr b --wronly c --command 0 1 2 cat a --command 0 0 2 cat b --test

diff -u a d

if [ $? -eq 0 ]
then
    echo "Test 5/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 5/$TESTNUM failed!?"
fi

# Test 6 - stderror and exit status
printf "cat: e: No such file or directory\ntr: range-endpoints of \'A--\' are in reverse collating sequence order\n" > d

./simpsh --rdonly a --wronly b --rdwr c --command 0 1 2 cat e - --command 0 1 2 tr A--Z a-z --test

EXIT_VAL=$?

diff -u c d

if [ $EXIT_VAL -eq 1 -a $? -eq 0 ]
then
    echo "Test 6/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 6/$TESTNUM failed!?"
fi

# Test 7 - catch signal
echo "11 caught" > d

(./simpsh --rdonly a --wronly b --wronly c --catch 11 --catch 4 --abort) 2> b

EXIT_VAL=$?

diff -u b d

if [ $EXIT_VAL -eq 11 -a $? -eq 0 ]
then
    echo "Test 7/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 7/$TESTNUM failed!?"
fi

# Test 8 - default signal
./simpsh --rdonly a --wronly b --wronly c --command 0 1 2 cat a --catch 11 --catch 4 --default 4 --default 11 --abort

if [ $? -eq 139 ]
then
    echo "Test 8/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 8/$TESTNUM failed!?"
fi

# Test 9 - ignore signal
./simpsh --rdonly a --wronly b --wronly c --catch 11 --ignore 11 --abort

if [ $? -eq 0 ]
then
    echo "Test 9/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 9/$TESTNUM failed!?"
fi

# Test 10 - verbose and brief
echo "This is file c" > c
printf "-" > d
printf "-" >> d
printf "creat --rdonly a \n--trunc --wronly b \n--wronly c \n--command 0 1 2 cat a \n" >> d
(./simpsh --verbose --creat --rdonly a --trunc --wronly b --wronly c --command 0 1 2 cat a --brief --command 0 1 2 cat c --test)> c

diff -u c d

if [ $? -eq 0 ]
then
    echo "Test 10/$TESTNUM passed"
    PASSNUM=$((PASSNUM+1))
else
    echo "Test 10/$TESTNUM failed!?"
fi

if [ $PASSNUM -eq $TESTNUM ]
then
    echo "ALL TESTS PASSED!!!"
else
    echo "SOME TESTS FAILED???"
fi

