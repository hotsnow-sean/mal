STEPS = step0_repl

all: $(STEPS)

%:
	xmake build $*

.PHONY: all clean

clean:
	xmake clean
	rm -f $(STEPS)
