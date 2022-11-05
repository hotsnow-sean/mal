STEPS = step0_repl step1_read_print step2_eval

all: $(STEPS)

%: src/%.rs
	cargo build --release --bin $*
	cp target/release/$* $@

step1_read_print: src/types.rs src/reader.rs
step2_eval: src/types.rs src/reader.rs

.PHONY: clean

clean:
	cargo clean
	rm -f $(STEPS)