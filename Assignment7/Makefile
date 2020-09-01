librsocket.a: rsocket.o
	ar -rcs librsocket.a rsocket.o
rsocket.o: rsocket.c rsocket.h
	cc -Wall -c rsocket.c
clean:
	rm librsocket.a rsocket.o user1 user2
test: user1.c user2.c rsocket.h librsocket.a
	gcc user1.c -o user1 -L. -lrsocket
	gcc user2.c -o user2 -L. -lrsocket
