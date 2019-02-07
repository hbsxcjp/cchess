objects = board_base.o piece.o info.o \
            board.o move.o chessInstance.o main.o 

a.exe: $(objects)
	g++ -Wall -o a.exe $(objects)

main.o: chessInstance.h
	gcc -c -std=c++11 -fexec-charset=gbk -Wall main.cpp
chessInstance.o: chessInstance.h info.h
	gcc -c -std=c++11 -fexec-charset=gbk -Wall chessInstance.cpp
move.o: move.h board.h info.h
	gcc -c -std=c++11 -fexec-charset=gbk -Wall move.cpp
board.o: board.h info.h move.h
	gcc -c -std=c++11 -fexec-charset=gbk -Wall board.cpp
info.o: info.h board_base.h
	gcc -c -std=c++11 -fexec-charset=gbk -Wall info.cpp
piece.o: board.h
	gcc -c -std=c++11 -fexec-charset=gbk -Wall piece.cpp
board_base.o: board_base.h
	gcc -c -std=c++11 -fexec-charset=gbk -Wall board_base.cpp

.PHONY: clean
clean:
	rm a.exe $(objects)
