nvcc: nvcc.c

test: nvcc
	bash ./test.sh
	./nvcc -test

clean:
	rm -rf nvcc *.o *~ tmp* *#*
