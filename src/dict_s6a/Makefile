dict_s6a.fdx: dict_s6a.o
	gcc -shared -o dict_s6a.fdx dict_s6a.o
dict_s6a.o: dict_s6a.c
	gcc -fPIC -c -Wall -Wextra dict_s6a.c -lfdproto -lfdcore
install:
	sudo cp *.fdx /usr/lib/freeDiameter
clean:
	rm -rf *.o *.fdx
