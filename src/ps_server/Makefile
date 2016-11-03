libpsserver.a: ps_server.o
	ar rc ../libpsserver.a ps_server.o
ps_server.o: ps_server.c
	gcc -fPIC -c -Wall -Wextra ps_server.c -lfdproto -lfdcore -o ps_server.o -I../../../utils/src
install:
	sudo cp *.fdx /usr/lib/freeDiameter
clean:
	rm -rf *.o
