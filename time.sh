#!/bin/bash

#Example 1
touch z
for i in {1..10}
do
    echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" >> z
    done

touch y

time cat z | tr A-Z a-z | tr -sd 'b' 'a' > y

cat y
cat z
rm y
touch e
touch y
echo "Here"
cat y
echo "Here"
./simpsh --rdonly z --pipe --pipe --wronly y --wronly e --command 0 2 6 cat - --command 1 4 6 tr A-Z a-z --command 3 5 6 tr -sd 'b' 'd' --profile


