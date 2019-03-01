objects = obj/jsoncpp.o obj/tools.o obj/board_base.o obj/piece.o \
            obj/board.o obj/move.o obj/chessInstance.o obj/main.o 

vpath %.h src/head src/json
vpath %.cpp src
vpath %.o obj

a.exe: $(objects)
	g++ -Wall -o a.exe $(objects)

obj/main.o: chessInstance.h main.cpp
	gcc -c -o obj/main.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/main.cpp
obj/chessInstance.o: chessInstance.h chessInstance.cpp
	gcc -c -o obj/chessInstance.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/chessInstance.cpp
obj/move.o: move.h board.h move.cpp
	gcc -c -o obj/move.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/move.cpp
obj/board.o: board.h move.h board.cpp
	gcc -c -o obj/board.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/board.cpp
obj/piece.o: board.h piece.cpp
	gcc -c -o obj/piece.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/piece.cpp
obj/board_base.o: board_base.h board_base.cpp
	gcc -c -o obj/board_base.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/board_base.cpp
obj/tools.o: tools.h tools.cpp
	gcc -c -o obj/tools.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/tools.cpp
obj/jsoncpp.o: json.h json-forwards.h jsoncpp.cpp
	gcc -c -o obj/jsoncpp.o -std=c++11 -fexec-charset=gbk -iquote src/json -Wall src/jsoncpp.cpp


.PHONY: clean
clean:
	rm a.exe $(objects)
