EXE=example
CXX=$(shell command -v g++-10 || command -v g++)
SRC=simcpp20.cpp
HDR=simcpp20.hpp

.PHONY: all clean

all: $(EXE)

%: %.cpp $(SRC) $(HDR)
	$(CXX) -Wall -Wextra -fcoroutines $< $(SRC) -o $@

clean:
	rm -f $(EXE)
