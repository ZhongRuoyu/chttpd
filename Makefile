VERSION = 0.1.0

CFLAGS = -O2
LDFLAGS =

SRCS = $(shell find src -name '*.c' | sort)
OBJS = $(SRCS:src/%.c=out/%.o)
DEPS = $(SRCS:src/%.c=out/%.d)

CHTTPD_CFLAGS = -std=c11 -D_XOPEN_SOURCE=700 -DCHTTPD_VERSION=\"$(VERSION)\"
CHTTPD_LDFLAGS =
CHTTPD_DEPFLAGS = -MT $@ -MMD -MP -MF out/$*.d

TESTS = $(shell find test -name '*.sh' | sort)

GIT_HASH := $(shell test -f .git/HEAD && if grep -q '^ref:' .git/HEAD; then cat .git/`sed 's/^ref: //' .git/HEAD`; else cat .git/HEAD; fi)
ifneq ($(GIT_HASH),)
	CHTTPD_CFLAGS += -DGIT_HASH=\"$(GIT_HASH)\"
endif


.PHONY: all
all: chttpd

-include $(DEPS)

.PHONY: test
.NOTPARALLEL: test
test: all
	@$(MAKE) $(TESTS) --no-print-directory
	@echo "All tests passed."

.PHONY: test-asan
test-asan:
	@$(MAKE) CFLAGS="-O0 -g -fsanitize=address -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address" test


chttpd: $(OBJS)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(CHTTPD_LDFLAGS) $(LDFLAGS)

chttpd-asan: $(OBJS)
	$(MAKE) CFLAGS="-O0 -g -fsanitize=address -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address" chttpd

out/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CHTTPD_DEPFLAGS) $(CHTTPD_CFLAGS) $(CFLAGS) -c -o $@ $<


.PHONY: $(TESTS)
$(TESTS):
	@set -e; \
		test -t 1 && red="\033[31m" || red=""; \
		test -t 1 && green="\033[32m" || green=""; \
		test -t 1 && yellow="\033[33m" || yellow=""; \
		test -t 1 && reset="\033[0m" || reset=""; \
		echo "[ $${yellow}TESTING$${reset} ] $@"; \
		( \
			./$@ && \
			echo "[ $${green}PASSED$${reset}  ] $@" \
		) || ( \
			echo "[ $${red}FAILED$${reset}  ] $@" && \
			exit 1 \
		)


.PHONY: clean
clean:
	rm -rf out chttpd
