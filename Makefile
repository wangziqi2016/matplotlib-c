
CFLAGS=-I/usr/include/python2.7/ -lpython2.7

.phony: all test

all: test

test: test-matplotlib

test-matplotlib: matplotlib.o test-matplotlib.c
	gcc test-matplotlib.c -c -o test-matplotlib.o -g -Wall -Wextra $(CFLAGS)
	gcc test-matplotlib.o matplotlib.o -o test-matplotlib -g -Wall -Wextra $(CFLAGS)

matplotlib.o: matplotlib.c matplotlib.h
	gcc matplotlib.c -c -o matplotlib.o -g -Wall -Wextra $(CFLAGS)