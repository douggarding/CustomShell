CFLAGS = -std=c++14 -Wall -g -fsanitize=address,undefined

all: shell

shell: main.cpp shelpers.o
	clang++ -o shell $(CFLAGS) $^

shelpers.o: shelpers.cpp shelpers.hpp
	clang++ -c $(CFLAGS) $^

clean: 
	rm *.o