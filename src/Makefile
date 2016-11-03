app_hss.fdx: app_hss.o
	make -C dict_s6a
	make -C app_s6a
	make -C ps_server
	gcc -shared app_hss.o -o app_hss.fdx -L. -lapp_s6a -lpsserver -liplistener
app_hss.o: app_hss.c
	gcc -fPIC -c -Wall -Wextra app_hss.c -lfdproto -lfdcore -o app_hss.o
install:
	make install -C dict_s6a
	sudo cp *.fdx /usr/lib/freeDiameter
clean:
	make clean -C dict_s6a
	make clean -C app_s6a
	make clean -C ps_server
	rm -rf *.o *.fdx
