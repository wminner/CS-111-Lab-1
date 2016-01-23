echo "ABCDEFGHIJK" > tempa
touch tempb
touch tempc
touch testb

cat tempa | tr A-Z a-z > testb

./simpsh --verbose --rdonly tempa --wronly tempb --pipe --wronly tempc --command 0 3 4 cat tempa --command 2 1 4 tr A-Z a-z

diff -u tempb testb

if [ $? -eq 0 ]
then
    echo "Pipe test passed"
else
    echo "Pipe test failed!?"
    fi
