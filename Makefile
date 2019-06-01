nvcc: nvcc.c

test: nvcc
	bash ./test.sh

clean:
	rm -rf nvcc *.o *~ tmp* *#*
