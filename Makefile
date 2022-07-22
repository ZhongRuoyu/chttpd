CFLAGS = -O2
LDFLAGS =

SRCS = $(shell find src -name *.c | sort)
OBJS = $(SRCS:src/%.c=out/%.o)
DEPS = $(SRCS:src/%.c=out/%.d)

CHTTPD_CFLAGS = -std=c11 -D_XOPEN_SOURCE=700
CHTTPD_LDFLAGS =
CHTTPD_DEPFLAGS = -MT $@ -MMD -MP -MF out/$*.d

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
