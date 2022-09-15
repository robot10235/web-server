http_conn: http_conn.o main.o
	g++ -pthread -o http_conn main.o http_conn.o 
http_conn.o: locker.h http_conn.h http_conn.cpp
	g++ -c http_conn.cpp
main.o: main.cpp http_conn.h locker.h threadpool.h
	g++ -c main.cpp
clean:
	rm http_conn http_conn.o main.o
