all: test1 maker inverter

test1: main.cpp
	g++ -c main.cpp
	g++ -o test1 main.o -lgdal

maker: maker.cpp
	g++ -c maker.cpp
	g++ -o maker maker.o -lgdal

inverter: inverter.cpp
	g++ -c inverter.cpp
	g++ -o inverter inverter.o -lgdal

clean:
	rm -f main.o
	rm -f test1
	rm -f maker.o
	rm -f maker
	rm -f inverter.o
	rm -f inverter
