CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall

SRC   := main.cpp algorithm.cpp generator.cpp validator.cpp metrics.cpp
TSRC  := tests.cpp algorithm.cpp validator.cpp

all: fjsp tests

fjsp: $(SRC) *.h
	$(CXX) $(CXXFLAGS) -o fjsp $(SRC)

tests: $(TSRC) *.h
	$(CXX) $(CXXFLAGS) -o tests $(TSRC)

run: fjsp
	./fjsp

check: tests
	./tests

clean:
	rm -f fjsp tests
	rm -rf results instances
