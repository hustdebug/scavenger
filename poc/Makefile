ALL = poc
CXX ?= g++
CXXFLAGS = -std=c++11 -g 

all: $(ALL)


poc: poc.o  common.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<


.PHONY: clean
clean:
	rm -rf *.o
	rm -rf $(ALL)


