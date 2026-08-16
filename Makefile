.PHONY: all

all: font tor

font: charmap-oldschool_white.png
	ld -r -b binary -o charmap-oldschool_white.o charmap-oldschool_white.png 

tor: tor.cpp la.cpp charmap-oldschool_white.o
	g++ -std=c++23 -ggdb -Wall -Wextra -pedantic -o tor tor.cpp la.cpp charmap-oldschool_white.o -lraylib -lGL -lX11 -lpthread -lm
