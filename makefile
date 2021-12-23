all: client

client: client.cpp
	g++ -g -w -std=c++11 -o client client.cpp

clean:
	rm -rf *.o fifo* client