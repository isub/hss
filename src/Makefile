app_hss.fdx: dict_cxdx.o app_hss.o app_hss_server.o
	g++ -shared -o app_hss.fdx  dict_cxdx.o app_hss.o app_hss_server.o
dict_cxdx.o: dict_cxdx.c
	gcc -fPIC -c -Wall -Wextra dict_cxdx.c -lfdproto -lfdcore
app_hss.o: app_hss.c
	gcc -fPIC -c -Wall -Wextra app_hss.c -lfdproto -lfdcore
app_hss_server.o: app_hss_server.c
	gcc -fPIC -c -Wall -Wextra app_hss_server.c -lfdproto -lfdcore
install:
	sudo cp app_hss.fdx /usr/lib/freeDiameter
clean:
	rm -rf *.o *.fdx
