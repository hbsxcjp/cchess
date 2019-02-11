objects = board_base.o piece.o info.o \
            board.o move.o chessInstance.o main.o 

vpath %.h head
vpath %.cpp src
vpath %.o obj

a.exe: $(objects)
	g++ -Wall -o a.exe $(objects)

main.o: chessInstance.h main.cpp
	gcc -c -o obj/main.o -std=c++11 -fexec-charset=gbk -iquote head -Wall src/main.cpp
chessInstance.o: chessInstance.h info.h chessInstance.cpp
	gcc -c -o obj/chessInstance.o -std=c++11 -fexec-charset=gbk -iquote head -Wall src/chessInstance.cpp
move.o: move.h board.h info.h move.cpp
	gcc -c -o obj/move.o -std=c++11 -fexec-charset=gbk -iquote head -Wall src/move.cpp
board.o: board.h info.h move.h board.cpp
	gcc -c -o obj/board.o -std=c++11 -fexec-charset=gbk -iquote head -Wall src/board.cpp
info.o: info.h board_base.h info.cpp
	gcc -c -o obj/info.o -std=c++11 -fexec-charset=gbk -iquote head -Wall src/info.cpp
piece.o: board.h piece.cpp
	gcc -c -o obj/piece.o -std=c++11 -fexec-charset=gbk -iquote head -Wall src/piece.cpp
board_base.o: board_base.h board_base.cpp
	gcc -c -o obj/board_base.o -std=c++11 -fexec-charset=gbk -iquote head -Wall src/board_base.cpp


.PHONY: clean
clean:
	rm a.exe $(objects)
