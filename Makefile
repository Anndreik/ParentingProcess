HEADERS =

default: parentingprocess

parentingprocess.o: parentingprocess.c $(HEADERS)
	gcc -c -D_REENTRANT parentingprocess.c -lpthread -o parentingprocess.o -lm

parentingprocess: parentingprocess.o
	gcc -D_REENTRANT parentingprocess.o -lpthread -o parentingprocess -lm

clean:
	-rm -f parentingprocess.o
	-rm -f parentingprocess
