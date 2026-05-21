.PHONY: all run gdb clean latest $(ALL)

# Auto-detect AM_HOME and NEMU_HOME if not provided via environment, so the
# verifier can run `make ARCH=riscv32-nemu run` without sourcing a shell rc.
AM_HOME    ?= $(abspath $(CURDIR)/../../../abstract-machine)
NEMU_HOME  ?= $(abspath $(CURDIR)/../../../nemu)
export AM_HOME
export NEMU_HOME

RESULT = .result
$(shell > $(RESULT))

COLOR_RED   = \033[1;31m
COLOR_GREEN = \033[1;32m
COLOR_NONE  = \033[0m

ALL = $(basename $(notdir $(shell find tests/. -name "*.c")))

all: $(addprefix Makefile., $(ALL))
	@echo "test list [$(words $(ALL)) item(s)]:" $(ALL)

$(ALL): %: Makefile.%

Makefile.%: tests/%.c latest
	@/bin/echo -e "NAME = $*\nSRCS = $<\ninclude $${AM_HOME}/Makefile" > $@
	@if make -s -f $@ ARCH=$(ARCH) $(MAKECMDGOALS); then \
		printf "[%14s] $(COLOR_GREEN)PASS$(COLOR_NONE)\n" $* >> $(RESULT); \
	else \
		printf "[%14s] $(COLOR_RED)***FAIL***$(COLOR_NONE)\n" $* >> $(RESULT); \
	fi
	-@rm -f Makefile.$*

run: all
	@cat $(RESULT)
	@rm $(RESULT)

gdb: all

clean:
	rm -rf Makefile.* build/

latest:
