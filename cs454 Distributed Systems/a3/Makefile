default: clean objects library server client binder

all: server client binder

objects: server.c server_functions.c server_function_skels.c server_stuff.c client1.c client2.c
	g++ -c server.c server_functions.c server_function_skels.c server_stuff.c client1.c client2.c -DTESTING -Wno-write-strings

server: server.o server_functions.o server_function_skels.o server_stuff.o
	g++ -L. server.o server_functions.o server_function_skels.o -lrpc -o server

library: server_stuff.o
	ar rc librpc.a server_stuff.o
	ranlib librpc.a

client2: client2.o
	g++ -L. client2.o -lrpc -o client

client: client1.o
	g++ -L. client1.o -lrpc -o client

binder: binder.c
	g++ binder.c -DTESTING -o binder

tests: tests.c
	g++ -o tests tests.c binder.c

rserver: server
	./server
	
clean:
	rm -f *.o server client binder tests *.a
