CFLAGS=-Wall -std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

nvcc: $(OBJS)
	$(CC) -o nvcc $(OBJS) $(LDFLAGS)

$(OBJS): nvcc.h

test: nvcc
	bash ./test.sh
	./nvcc -test

clean:
	rm -rf nvcc *.o *~ tmp* *#*
