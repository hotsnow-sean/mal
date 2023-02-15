STEPS = step0_repl

all: $(STEPS)

%: src/%.cpp
	xmake build $*

.PHONY: all clean

clean:
	xmake clean
	rm -f $(STEPS)
