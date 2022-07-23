VERSION =

CFLAGS = -O2
LDFLAGS =

SRCS = $(shell find src -name '*.c' | sort)
OBJS = $(SRCS:src/%.c=out/%.o)
DEPS = $(SRCS:src/%.c=out/%.d)

CHTTPD_CFLAGS = -std=c11 -D_XOPEN_SOURCE=700 -DCHTTPD_VERSION=\"$(VERSION)\"
CHTTPD_LDFLAGS =
CHTTPD_DEPFLAGS = -MT $@ -MMD -MP -MF out/$*.d

GIT_HASH := $(shell test -f .git/HEAD && if grep -q '^ref:' .git/HEAD; then cat .git/`sed 's/^ref: //' .git/HEAD`; else cat .git/HEAD; fi)
ifneq ($(GIT_HASH),)
	CHTTPD_CFLAGS += -DGIT_HASH=\"$(GIT_HASH)\"
endif

.PHONY: all
all: chttpd

-include $(DEPS)

chttpd: $(OBJS)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(CHTTPD_LDFLAGS) $(LDFLAGS)

out/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CHTTPD_DEPFLAGS) $(CHTTPD_CFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf out chttpd
