CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
SRC=$(filter-out $(wildcard ref*.cpp), $(wildcard *.cpp))

CC=g++
INCLUDES=
CFLAGS=-fdiagnostics-color=always -std=c++2a -g -pthread
LIB=-lm -L../unittest-cpp-master/UnitTest++ -lUnitTest++


all: test

test: test.cpp modules.cpp utility.cpp motor.cpp
	$(CC) $(CFLAGS)  -DUNIT_TESTING $(INCLUDES) -o $@ $^ $(LIB)

clean:
	-rm -f test salinity

