 # CS 111 Lab 1 Makefile

CC = gcc
CFLAGS = -g -Wall -Wextra -Werror
DISTDIR = lab1-$(USER)

SIMPSH_SOURCES = \
  main.c
SIMPSH_OBJECTS = $(subst .c,.o,$(SIMPSH_SOURCES))

DIST_SOURCES = \
  $(SIMPSH_SOURCES) Makefile README

simpsh: $(SIMPSH_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(SIMPSH_OBJECTS)

dist: $(DISTDIR).tar.gz

$(DISTDIR).tar.gz: $(DIST_SOURCES)
	rm -rf $(DISTDIR)
	tar -czf $@ $(DIST_SOURCES)

check: 

clean:
	rm -rf *~ *.o *.tar.gz simpsh $(DISTDIR)
