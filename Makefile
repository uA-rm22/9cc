9cc: 9cc.c
	gcc -o 9cc 9cc.c

test: 9cc
	chmod 755 test.sh
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*