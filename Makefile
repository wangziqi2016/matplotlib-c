
CFLAGS=-I/usr/include/python2.7/ -lpython2.7

.phony: all test

all: test

test: test-matplotlib test-primitives

test-matplotlib: matplotlib.o test-matplotlib.c
	gcc test-matplotlib.c -c -o test-matplotlib.o -g -Wall -Wextra $(CFLAGS)
	gcc test-matplotlib.o matplotlib.o -o test-matplotlib -g -Wall -Wextra $(CFLAGS)

test-primitives: primitives.o test-primitives.c
	gcc test-primitives.c -c -o test-primitives.o -g -Wall -Wextra $(CFLAGS)
	gcc test-primitives.o primitives.o -o test-primitives -g -Wall -Wextra $(CFLAGS)

matplotlib.o: matplotlib.c matplotlib.h
	gcc matplotlib.c -c -o matplotlib.o -g -Wall -Wextra $(CFLAGS)

primitives.o: primitives.c primitives.h
	gcc primitives.c -c -o primitives.o -g -Wall -Wextra $(CFLAGS)