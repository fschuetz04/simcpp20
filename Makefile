C=$(shell command -v g++-10 || command -v g++)
EXE=test

.PHONY: clean

all: $(EXE)

%: %.cpp
	$(C) -Wall -std=c++20 -fcoroutines $< -o $@

clean:
