SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
ARG="0;"

9cc: $(SRCS)
		gcc -g -O0 -o 9cc $(SRCS)

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

debug:
	gcc -g -O0 -o 9cc $(SRCS)
	gdb --args 9cc "$(ARG)"
