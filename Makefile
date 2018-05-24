# Makefile 
# main.cpp part.hpp utility.hpp part.cpp utility.cpp
#SRC = src
all:
	clang++ -std=c++11 -Wall src/main.cpp src/parser.cpp src/layout.cpp -o main
g++:
	g++ -std=c++11 -Wall src/main.cpp src/parser.cpp src/layout.cpp -o main
test1:
	./main ./circuit1/circuit1.conf
test2:
	./main ./circuit2/circuit2.config
test3:
	./main ./circuit3/circuit3.config
test4:
	./main ./circuit4/circuit4.config
test5:
	./main ./circuit5/circuit5.config
clean:
	rm main
# Use address sanitizer to check memory
mem_check:
	clang++ -std=c++11 -Wall src/main.cpp src/parser.cpp src/layout.cpp -o main -fsanitize=address
