.PHONY: all

all: tor

tor: tor.cpp la.cpp
	g++ -std=c++26 -ggdb -Wall -Wextra -pedantic -o tor tor.cpp la.cpp -lraylib -lX11
