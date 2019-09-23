SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(SRCS)
	gcc -o 9cc $(SRCS)

$(OBJS): 9cc.h

test: 9cc
	chmod 755 test.sh
	./test.sh

gitPush: $(SRCS) Makefile test.sh
	git add $(SRCS) Makefile test.sh 
	git commit
	git push origin master

clean:
	rm -f 9cc *.o *~ tmp*