# CS 111 Lab 1 Makefile

CC = gcc
#CFLAGS = -g -Wall -Wextra -Werror
CFLAGS = -g -Wall -Wextra
DIR = lab1-$(USER)

SIMPSH_SOURCES = \
  main.c \
  openfile.c \
  executecmd.c
SIMPSH_OBJECTS = $(subst .c,.o,$(SIMPSH_SOURCES))

DIST_SOURCES = $(SIMPSH_SOURCES) Makefile README test.sh

simpsh: $(SIMPSH_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(SIMPSH_OBJECTS)

dist: $(DIR).tar.gz

$(DIR).tar.gz: $(DIST_SOURCES)
	rm -rf $(DIR)
	tar -czf $@ --transform='s,^,$(DIR)/,' $(DIST_SOURCES)

check: 
	./test.sh

clean:
	rm -rf *~ *.o *.tar.gz simpsh $(DIR) a b c d
