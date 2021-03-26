EXE=test
C=$(shell command -v g++-10 || command -v g++)

.PHONY: all clean

all: $(EXE)

%: %.cpp simcpp2.cpp
	$(C) -Wall -std=c++20 -fcoroutines $^ -o $@

clean:
	rm -f $(EXE)
