STEPS = step0_repl step1_read_print step2_eval step3_env step4_if_fn_do step5_tco step6_file step7_quote step8_macros step9_try stepA_mal

all: $(STEPS)

%: src/%.rs
	cargo build --release --bin $*
	cp target/release/$* $@

step1_read_print: src/types.rs src/reader.rs
step2_eval: src/types.rs src/reader.rs
step3_env: src/types.rs src/reader.rs src/env.rs
step4_if_fn_do: src/types.rs src/reader.rs src/env.rs src/malcore.rs
step5_tco: src/types.rs src/reader.rs src/env.rs src/malcore.rs
step6_file: src/types.rs src/reader.rs src/env.rs src/malcore.rs
step7_quote: src/types.rs src/reader.rs src/env.rs src/malcore.rs
step8_macros: src/types.rs src/reader.rs src/env.rs src/malcore.rs
step9_try: src/types.rs src/reader.rs src/env.rs src/malcore.rs
stepA_mal: src/types.rs src/reader.rs src/env.rs src/malcore.rs

.PHONY: clean

clean:
	cargo clean
	rm -f $(STEPS)