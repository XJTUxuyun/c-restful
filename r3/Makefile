CC = gcc -std=c99 -g

APPLIB=libr3.so.1 libr3.a

OBJECTS= edge.o match_entry.o memory.o node.o slug.o str.o token.o

all: $(APPLIB)

edge.o: edge.c
	$(CC) -fPIC -c -o edge.o edge.c 

match_entry.o: match_entry.c
	$(CC) -fPIC -c -o match_entry.o match_entry.c

memory.o: memory.c
	$(CC) -fPIC -c -o memory.o memory.c  

node.o: node.c
	$(CC) -fPIC -c -o node.o node.c  

slug.o: slug.c
	$(CC) -fPIC -c -o slug.o slug.c  

str.o: slug.c
	$(CC) -fPIC -c -o str.o str.c  

token.o: token.c
	$(CC) -fPIC -c -o token.o token.c  

libr3.a: $(OBJECTS)
	ar rc ../libr3.a $(OBJECTS)


libr3.so.1:	$(OBJECTS)
	$(CC)	-shared -o libr3.so.1 $(OBJECTS)

clean:
	-rm *.o
	-rm libr3.so.1


