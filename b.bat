@echo off
rem cls
gcc -c -std=c++11 -fexec-charset=gbk -Wall info.cpp
gcc -c -std=c++11 -fexec-charset=gbk -Wall piece.cpp
gcc -c -std=c++11 -fexec-charset=gbk -Wall board_base.cpp
gcc -c -std=c++11 -fexec-charset=gbk -Wall board.cpp
gcc -c -std=c++11 -fexec-charset=gbk -Wall main.cpp
g++ -o a.exe -Wall main.o board.o piece.o board_base.o info.o 
 