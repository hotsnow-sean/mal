STEPS = step0_repl

all: $(STEPS)

%: src/%.rs
	cargo build --release --bin $*
	cp target/release/$* $@

.PHONY: clean

clean:
	cargo clean
	rm -f $(STEPS)