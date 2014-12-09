CC=g++
CFLAGS=-DNDEBUG -O3 -Wall -IDBoW2 $(FLAGS)

LFLAGS=-LDBoW2 -lDBoW2 -LDLib/lib -lDVision -lDUtilsCV -lDUtils \
 -lstdc++ $(LINK_FLAGS)

DEPS=lib/libDBoW2.so dlib
TARGET=demo

all: $(TARGET)

$(TARGET): $(TARGET).o $(DEPS)
	$(CC) $(TARGET).o $(LFLAGS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

dlib:
	make -C DLib

lib/libDBoW2.so:
	make -C DBoW2 && mkdir -p ./lib/ && cp DBoW2/libDBoW2.so ./lib/

clean:
	rm -f *.o $(TARGET); rm -f ./lib/*.so; \
	make -C DBoW2 clean; make -C DLib clean

install: $(TARGET)
	make -C DBoW2 install && cp DBoW2/libDBoW2.so ./lib/

uninstall:
	make -C DBoW2 uninstall
