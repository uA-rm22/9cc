9cc: 9cc.c
	gcc -o 9cc 9cc.c

test: 9cc
	chmod 755 test.sh
	./test.sh

gitAdd: 9cc Makefile test.sh
	git add 9cc.c Makefile test.sh 
	git commit
	git push origin master
clean:
	rm -f 9cc *.o *~ tmp*