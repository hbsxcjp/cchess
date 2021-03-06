objects = obj/tools.o obj/piece.o obj/seat.o obj/board.o obj/instance.o obj/main.o \
            obj/jsoncpp.o 

vpath %.h src/head src/json
vpath %.cpp src
vpath %.o obj

a.exe: $(objects)
	g++ -Wall -o a.exe $(objects)

obj/main.o: main.cpp
	gcc -c -o obj/main.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/main.cpp
obj/instance.o: instance.cpp
	gcc -c -o obj/instance.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/instance.cpp
obj/board.o: board.cpp
	gcc -c -o obj/board.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/board.cpp
obj/seat.o: seat.cpp
	gcc -c -o obj/seat.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/seat.cpp
obj/piece.o: piece.cpp
	gcc -c -o obj/piece.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/piece.cpp
obj/tools.o: tools.cpp
	gcc -c -o obj/tools.o -std=c++11 -fexec-charset=gbk -iquote src/head -Wall src/tools.cpp
obj/jsoncpp.o: jsoncpp.cpp
	gcc -c -o obj/jsoncpp.o -std=c++11 -fexec-charset=gbk -iquote src/json -Wall src/jsoncpp.cpp


.PHONY: clean
clean:
	rm a.exe $(objects)
