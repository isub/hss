libapp_s6a.a: app_s6a.o app_s6a_dict_cache.o app_s6a_client.o app_s6a_client_signal.o
	ar rc ../libapp_s6a.a app_s6a.o app_s6a_dict_cache.o app_s6a_client.o app_s6a_client_signal.o
app_s6a.o: app_s6a.c
	gcc -fPIC -c -Wall -Wextra app_s6a.c -lfdproto -lfdcore -o app_s6a.o
app_s6a_dict_cache.o: app_s6a_dict_cache.c
	gcc -fPIC -c -Wall -Wextra app_s6a_dict_cache.c -lfdproto -lfdcore -o app_s6a_dict_cache.o
app_s6a_client.o: app_s6a_client.c
	gcc -fPIC -c -Wall -Wextra app_s6a_client.c -lfdproto -lfdcore -o app_s6a_client.o
app_s6a_client_signal.o: app_s6a_client_signal.c
	gcc -fPIC -c -Wall -Wextra app_s6a_client_signal.c -lfdproto -lfdcore -o app_s6a_client_signal.o
install:
	sudo cp *.fdx /usr/lib/freeDiameter
clean:
	rm -rf *.o
