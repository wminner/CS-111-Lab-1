#!/bin/bash

#TEST 1
# touch z
# for i in {1..10}
# do
#     echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" >> z
# done

# for i in {1..5}
# do
# 	time cat z | tr A-Z a-z | tr -sd 'b' 'a' > y & 2>c
#	rm y c
# 	/usr/bin/time -f "Page Faults:\t%F\nMax resident set size:\t%M\n\t%I\n\t%O\n\t%R\nVoluntary context switches:\t%c\nInvoluntary context switches:\t%w\n" cat z | tr A-Z a-z | tr -sd 'b' 'a' > y & 2>c
# 	rm y c
# done

# User CPU Time:\t%U\nSystem CPU Time:\t%S\nMax resident set size:\t%M\n
#cat y
#cat z
# rm y

# touch e
# touch y
# for i in {1..5}
# do
# 	./simpsh --profile --rdonly z --pipe --pipe --wronly y --wronly e --command 0 2 6 cat --command 1 4 6 tr A-Z a-z --command 3 5 6 tr -sd 'b' 'd' --close 2 --close 4 --wait
# done

# rm e y
#rm z


#TEST 2

# for i in {1..5}
# do
# 	touch a
# 	./simpsh --profile --rdonly a --creat --wronly b --creat --wronly c --pipe --pipe --command 0 4 2 echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" --command 3 6 2 tr A-Z a-z --command 5 1 2 cat --close 4 --close 6 --wait
# 	rm a b c
# done

# for i in {1..5}
# do
# 	time echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" | tr A-Z a-z | cat > b & 2>c
# 	rm b c
# 	/usr/bin/time -f "Page Faults:\t%F\nMax resident set size:\t%M\n\t%I\n\t%O\n\t%R\nVoluntary context switches:\t%c\nInvoluntary context switches:\t%w\n" echo "ABCDEFGHIJJKLMNOPQRSTUVWXYZ" | tr A-Z a-z | cat > b & 2>c
# 	rm b c
# done

#TEST 3

# for i in {1..1}
# do
	time head -c 1MB /dev/urandom | tr -s A-Z a-z | cat > b
	rm b

	/usr/bin/time -f "Page Faults:\t%F\nMax resident set size:\t%M\n\t%I\n\t%O\n\t%R\nVoluntary context switches:\t%c\nInvoluntary context switches:\t%w\n" head -c 1MB /dev/urandom | tr -s A-Z a-z | cat > b
	rm b
# done

# for i in {1..1}
# do
	# touch a
	# ./simpsh --profile --rdonly a --creat --wronly b --creat --wronly c --pipe --pipe --command 0 4 2 head -c 1MB /dev/urandom --command 3 6 2 tr -s A-Z a-z --command 5 1 2 cat --close 4 --close 6 --wait
	# rm a b c
# done











