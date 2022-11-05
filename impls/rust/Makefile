STEPS = step0_repl step1_read_print step2_eval step3_env step4_if_fn_do

all: $(STEPS)

%: src/%.rs
	cargo build --release --bin $*
	cp target/release/$* $@

step1_read_print: src/types.rs src/reader.rs
step2_eval: src/types.rs src/reader.rs
step3_env: src/types.rs src/reader.rs src/env.rs
step4_if_fn_do: src/types.rs src/reader.rs src/env.rs src/malcore.rs

.PHONY: clean

clean:
	cargo clean
	rm -f $(STEPS)