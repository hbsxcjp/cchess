@echo off
cls
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall info.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall board_base.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall piece.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall board.cpp
gcc -c -std=c++11 -fexec-charset=gbk -Wall move.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall chessInstance.cpp
rem gcc -c -std=c++11 -fexec-charset=gbk -Wall main.cpp
rem g++ -Wall -o a.exe main.o move.o
rem gcc -c -std=c++11 -Wall -fexec-charset=gbk info.cpp board_base.cpp piece.cpp board.cpp move.cpp chessInstance.cpp main.cpp 
rem -fexec-charset=gbk  -finput-charset=UTF-8
g++ -Wall -o a.exe main.o chessInstance.o board_base.o move.o board.o  piece.o  info.o 
 