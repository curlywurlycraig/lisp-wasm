BUILD=out

.DEFAULT_GOAL := all
.PHONY: all clean watch
.PRECIOUS: $(BUILD)/%.o

CFLAGS+=-std=c11 -Iinclude/ -MMD -Wall -Wextra -Werror -pedantic

SRCS=$(shell find src/ -name '*.c')
OBJS=$(SRCS:%.c=$(BUILD)/%.o)
DEPS=$(SRCS:%.c=$(BUILD)/%.d)

BINSRC=$(shell find bins/ -name '*.c')
BINDEPS=$(BINSRC:%.c=$(BUILD)/%.d)
BINS=$(addprefix $(BUILD)/,$(basename $(BINSRC)))

-include $(DEPS)
-include $(BINDEPS)

$(BUILD)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/bins/%: $(BUILD)/bins/%.o $(OBJS) Makefile
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@

all: $(BINS)
	@echo 'Build successful'

clean:
	rm -rf $(BUILD)

watch:
	@type entr 1>/dev/null 2>&1 || (echo "target watch requires entr to be installed" && exit 127)
	find . -name '*.[ch]' | entr sh -c 'make all'
