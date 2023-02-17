STEPS = step0_repl step1_read_print step2_eval step3_env step4_if_fn_do

all:
	xmake

$(STEPS):
	xmake build $@

.PHONY: all clean $(STEPS)

clean:
	xmake clean
