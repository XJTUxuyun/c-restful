CC = gcc -g -std=c99

CFLAGS= -I /usr/include/libxml2

LIBS= -L. libr3.a -lpcre -lpthread -lxml2 -lrt

OBJECTS= main.o sched.o restful.o http_parser.o mission.o

.PHONY:	wc	clean

dp:	$(OBJECTS)
	$(CC)	-o wc $(OBJECTS) $(LIBS)

clean:
	-rm *.o
	-rm wc

