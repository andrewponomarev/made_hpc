CC=clang++
CFLAGS=-Xpreprocessor -fopenmp
LOPT=-lomp
#-Xpreprocessor -fopenmp  -lomp -o hell
#clang++ -Xpreprocessor -fopenmp main.cpp matrix.cpp -lomp -o task1

SOURCES = main.cpp matrix.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = main

all: main

main: main.o matrix.o
	$(CC) $(CFLAGS) main.o matrix.o $(LOPT) -o task1

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

matrix.o: matrix.cpp
	$(CC) $(CFLAGS) -c matrix.cpp

clean:
	rm -rf *.o *.a task1