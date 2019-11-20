SUDOKU_SOURCE = sudoku.cpp sudoku_search.cpp general_search.cpp queues.cpp
sudoku: $(SUDOKU_SOURCE) sudoku.h
	g++ -std=c++11 -g -o $@ $(SUDOKU_SOURCE) -O3 -march=native
