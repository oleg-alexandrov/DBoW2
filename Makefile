CC=g++ -std=c++11
CFLAGS=-DNDEBUG -O3 -Wall -IDBoW2 $(FLAGS)

LFLAGS=-LDBoW2 -lDBoW2 -LDLib/lib -lDVision -lDUtilsCV -lDUtils \
 -lstdc++ $(LINK_FLAGS)

DEPS=lib/libDBoW2.so dlib
TARGET=demo
TARGET_SURF=demo_surf

all: $(TARGET) $(TARGET_SURF)

$(TARGET): $(TARGET).o $(DEPS)
	$(CC) $(TARGET).o $(LFLAGS) -o $@

$(TARGET_SURF): $(TARGET_SURF).o $(DEPS)
	$(CC) $(TARGET_SURF).o $(LFLAGS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

dlib:
	make -C DLib

# bug here, does not get recompiled!
lib/libDBoW2.so:
	make -C DBoW2 && mkdir -p ./lib/ && cp DBoW2/libDBoW2.so ./lib/

clean:
	rm -f *.o $(TARGET) $(TARGET_SURF); rm -f ./lib/*.so; \
	make -C DBoW2 clean; make -C DLib clean

install: $(TARGET) $(TARGET_SURF)
	make -C DBoW2 install && cp DBoW2/libDBoW2.so ./lib/

uninstall:
	make -C DBoW2 uninstall
