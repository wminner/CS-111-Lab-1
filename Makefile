# CS 111 Lab 1 Makefile

CC = gcc
CFLAGS = -O4 -g -Wall -Wextra -Werror -Wno-unused
#CFLAGS = -g -Wall -Wextra
DIR = lab1-$(USER)

SIMPSH_SOURCES = \
  main.c \
  openfile.c

SIMPSH_OBJECTS = $(subst .c,.o,$(SIMPSH_SOURCES))

DIST_SOURCES = $(SIMPSH_SOURCES) Makefile README test.sh time.sh execline1 execline2 execline3 checkdist

simpsh: $(SIMPSH_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(SIMPSH_OBJECTS)

dist: $(DIR).tar.gz

$(DIR).tar.gz: $(DIST_SOURCES)
	rm -rf $(DIR)
	tar -czf $@.tmp --transform='s,^,$(DIR)/,' $(DIST_SOURCES)
	./checkdist $(DIR)
	mv $@.tmp $@

check: test.sh
	./test.sh

clean:
	rm -rf *~ *.o *.tar.gz simpsh $(DIR) a b c d e x y z time_results *.tmp
time:
	./time.sh
