all: test1 maker inverter

test1: main.cpp
	g++ -c main.cpp
	g++ -o test1 main.o -lgdal

maker: maker.cpp
	g++ -c maker.cpp
	g++ -o maker maker.o -lgdal

inverter: inverter.cpp
	g++ -c inverter.cpp
	g++ -c ThreadController.cpp
	g++ -c ThreadHolder.cpp
	g++ -o inverter inverter.o ThreadController.o ThreadHolder.o -std=c++11 -lgdal -Os -pthread

clean:
	rm -f main.o
	rm -f test1
	rm -f maker.o
	rm -f maker
	rm -f inverter.o
	rm -f inverter
	rm -f ThreadController.o
	rm -f ThreadHolder.o
