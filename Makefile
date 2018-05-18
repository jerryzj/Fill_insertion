# Makefile 
# main.cpp part.hpp utility.hpp part.cpp utility.cpp
#SRC = src
all:
	clang++ -std=c++11 -Wall src/main.cpp -o main
g++:
	g++ -std=c++11 -Wall src/main.cpp -o main
test1:
	./main ./circuit1/circuit1.conf
clean:
<<<<<<< HEAD
	rm main src/*.hpp.gch
=======
	rm *.out src/*.hpp.gch
>>>>>>> 0783171aebd22c39c3ac812beac1dabddf840180
