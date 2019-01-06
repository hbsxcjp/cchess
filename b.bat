@echo off
rem cls
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall info.cpp board_base.cpp piece.cpp board.cpp main.cpp
gcc -c -std=c++11 -fexec-charset=gbk -Wall board.cpp piece.cpp
g++ -Wall -o a.exe main.o board.o  piece.o board_base.o
rem info.o   main.cpp
 