#!/bin/bash
# If diffs come up clean, then tests have passed, and this will be outputed on
# screen. There will be some output from 'make clean', 'make', and '--verbose'.

make clean
make

TESTNUM=5

# Test 1 - cat and verbose/brief
echo "This is file a" > a
echo "This is file b" > b
echo "This is file c" > c
printf "This is file a\nThis is file c\n" > d

./simpsh --rdonly a --wronly b --wronly c --verbose --command 0 1 2 cat a --brief --command 0 1 2 cat c --wait

diff -u b d

if [ $? -eq 0 ]
then
   echo "Test 1/$TESTNUM passed"
else
    echo "Test 1/$TESTNUM failed"
fi

# Test 2 - sort (and single dash options)
printf "4\n 3\n2\n1\n" > a
echo "This is file b" > b
echo "This is file c" > c
sort a -b -r | cat - c > d

./simpsh --rdonly a --wronly b --wronly c --command 0 1 2 sort -b -r --command 0 1 2 cat c --wait

diff -u b d

if [ $? -eq 0 ]
then
    echo "Test 2/$TESTNUM passed"
else
    echo "Test 2/$TESTNUM failed"
fi

# Test 3 - tr
printf "ABC XYZ 123 789" > a
echo "This is file b" > b
echo "This is file c" > c
cat a | tr A-Z a-z > d

./simpsh --rdonly a --wronly b --wronly c --command 0 1 2 tr A-Z a-z --wait

diff -u b d

if [ $? -eq 0 ]
then
    echo "Test 3/$TESTNUM passed"
else
    echo "Test 3/$TESTNUM failed"
fi

# Test 4 - oflags
echo "This is file a" > a
echo "This is file c" > c
rm b
cat a | cat - a > d

./simpsh --rdonly a --creat --append --wronly b --wronly c --command 0 1 2 cat a --command 0 1 2 cat a --wait

diff -u b d

if [ $? -eq 0 ]
then
    echo "Test 4/$TESTNUM passed"
else
    echo "Test 4/$TESTNUM failed"
fi

# Test 5 - rdwr
echo "This is file a" > a
echo "This is file b" > b

cat a | cat - a > d

./simpsh --append --rdwr a --rdwr b --wronly c --command 0 1 2 cat a --command 0 0 2 cat b --wait

diff -u a d

if [ $? -eq 0 ]
then
    echo "Test 5/$TESTNUM passed"
else
    echo "Test 5/$TESTNUM failed"
fi
