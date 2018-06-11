# Makefile 
SRC = src/main.cpp  src/parser.cpp src/rectangle.cpp src/statistic.cpp 
LAYOUT = src/layout_io.cpp src/layout_fill.cpp src/layout_rule.cpp src/layout_reorder.cpp src/layout_bin.cpp
all:
	clang++ -std=c++11 -Wall $(SRC) $(LAYOUT) -O3 -o main
g++:
	g++ -std=c++11 -Wall $(SRC) $(LAYOUT) -O3 -o main
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
	rm main *.cut statistics.txt
# Use address sanitizer to check memory
mem_check:
	clang++ -std=c++11 -Wall $(SRC) $(LAYOUT) -O3 -o main -fsanitize=address
