SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	gcc -o 9cc $(OBJS)

$(OBJS): 9cc.h

test: 9cc
	chmod 755 test.sh
	./test.sh

gitPush: $(OBJS) Makefile test.sh
	git add $(SRCS) Makefile test.sh 
	git commit
	git push origin master

clean:
	rm -f 9cc *.o *~ tmp*