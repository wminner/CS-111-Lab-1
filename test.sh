#!/bin/bash
# If diffs come up clean, then tests have passed
# There will be some output from 'make clean', 'make', and '--verbose'

make clean
make

# Test 1 - cat and verbose/brief
echo "This is file a" > a
echo "This is file b" > b
echo "This is file c" > c
printf "This is file a\nThis is file c\n" > d

./simpsh --rdonly a --wronly b --wronly c --verbose --command 0 1 2 cat a --brief --command 0 1 2 cat c

diff -u b d

if [ $? -eq 0 ]
then
   echo "Test 1/3 passed"
else
    echo "Test 1/3 failed"
fi

# Test 2 - sort
echo "This is file b" > b
printf "4\n3\n2\n1\n" > a
sort a | cat - c > d

./simpsh --rdonly a --wronly b --wronly c --command 0 1 2 sort --command 0 1 2 cat c 

diff -u b d

if [ $? -eq 0 ]
then
    echo "Test 2/3 passed"
else
    echo "Test 2/3 failed"
fi

# Test 3 - tr
echo "This is file b" > b
printf "ABC XYZ 123 789" > a
cat a | tr A-Z a-z > d

./simpsh --rdonly a --wronly b --wronly c --command 0 1 2 tr A-Z a-z

diff -u b d

if [ $? -eq 0 ]
then
    echo "Test 3/3 passed"
else
    echo "Test 3/3 failed"
fi

