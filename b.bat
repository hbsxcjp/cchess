@echo off
rem cls
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall info.cpp
gcc -c -std=c++11 -fexec-charset=gbk -Wall board_base.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall piece.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall board.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall chessInstance.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall move.cpp main.cpp
gcc -c -std=c++11 -fexec-charset=gbk -Wall move.cpp
rem g++ -Wall -o a.exe main.o move.o
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall info.cpp board_base.cpp piece.cpp board.cpp move.cpp chessInstance.cpp main.cpp
g++ -Wall -o a.exe main.o move.o board.o  piece.o board_base.o
rem info.o   main.cpp
 