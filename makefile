GCC = g++ -std=c++0x
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
DEBUG = -Wall

all: ana_maiko2.cpp
	$(GCC) ana_maiko2.cpp $(ROOTFLAGS) $(DEBUG) $(ROOTLIBS) -o ana_maiko2

clean:
	rm -f ana_maiko2 *.o
