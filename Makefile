GCC = g++
CFLAGS = -O3 -g -I/home/sghctoma/tools/ati-stream-sdk-v2.01-lnx64/include/
OBJECTS = Timer.o OCLCrack.o HashStore.o main.o
LIBS = -L/home/sghctoma/tools/ati-stream-sdk-v2.01-lnx64/lib/x86_64/ -lOpenCL

all: oclcrack
oclcrack: $(OBJECTS)
	$(GCC) -o oclcrack $(OBJECTS) $(LIBS)

Timer.o: Timer.h Timer.cpp
	$(GCC) $(CFLAGS) -c Timer.cpp

OCLCrack.o: OCLCrack.h OCLCrack.cpp MD5_cl.h
	$(GCC) $(CFLAGS) -c OCLCrack.cpp

HashStore.o: HashStore.h HashStore.cpp
	$(GCC) $(CFLAGS) -c HashStore.cpp

main.o: main.cpp
	$(GCC) $(CFLAGS) -c main.cpp

MD5_cl.h: MD5.cl
	./genkernelstring.sh

clean:
	rm -f oclcrack $(OBJECTS)
