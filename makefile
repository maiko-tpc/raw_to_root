GCC = g++ -std=c++0x
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
DEBUG = -Wall

all: ana_maiko2 driftv_ana
ana_maiko2:ana_maiko2.cpp
	$(GCC) ana_maiko2.cpp $(ROOTFLAGS) $(DEBUG) $(ROOTLIBS) -o ana_maiko2

driftv_ana: driftv_ana.cpp
	$(GCC) driftv_ana.cpp $(ROOTFLAGS) $(DEBUG) -DCOMPILE_STANDALONE $(ROOTLIBS) -o driftv_ana
clean:
	rm -f ana_maiko2 *.o driftv_ana
