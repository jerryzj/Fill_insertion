# Makefile 
# main.cpp part.hpp utility.hpp part.cpp utility.cpp
#SRC = src
all:
	clang++ -std=c++11 -Wall src/main.cpp -o main
g++:
	g++ -std=c++11 -Wall src/main.cpp
test1:
	./main ./circuit1/circuit1.conf
clean:
	rm *.out src/*.hpp.gch