VERSION = 0.2.2

CFLAGS = -O2
LDFLAGS =

SRCS = $(shell find src -name '*.c' | sort)

CHTTPD_CFLAGS = -std=c11 -D_XOPEN_SOURCE=700
CHTTPD_LDFLAGS =

TESTS = $(shell find test -name '*.sh' | sort)


.PHONY: all
all: chttpd

FORCE:
out/version.c: FORCE
	mkdir -p $(@D)
	scripts/update-version.sh . $@

ifndef UNIVERSAL_BINARY

OBJS = $(SRCS:src/%.c=out/%.o) out/version.o
DEPS = $(SRCS:src/%.c=out/%.d)
DEPFLAGS = -MT $@ -MMD -MP -MF out/$*.d

-include $(DEPS)

chttpd: $(OBJS)
	$(CC) $^ -o $@ $(CHTTPD_LDFLAGS) $(LDFLAGS)

out/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(DEPFLAGS) $(CHTTPD_CFLAGS) $(CFLAGS) -c -o $@ $<

out/version.o: out/version.c
	mkdir -p $(@D)
	$(CC) $(CHTTPD_CFLAGS) $(CFLAGS) -c -o $@ $<

else  # UNIVERSAL_BINARY

LIPO = lipo

OBJS_X86_64 = $(SRCS:src/%.c=out/x86_64/%.o) out/x86_64/version.o
OBJS_ARM64 = $(SRCS:src/%.c=out/arm64/%.o) out/arm64/version.o
DEPS_X86_64 = $(SRCS:src/%.c=out/x86_64/%.d)
DEPS_ARM64 = $(SRCS:src/%.c=out/arm64/%.d)
DEPFLAGS_X86_64 = -MT $@ -MMD -MP -MF out/x86_64/$*.d
DEPFLAGS_ARM64 = -MT $@ -MMD -MP -MF out/arm64/$*.d

TARGET_FLAG_X86_64 = -target x86_64-apple-macos10.15
TARGET_FLAG_ARM64 = -target arm64-apple-macos11

-include $(DEPS_X86_64) $(DEPS_ARM64)

chttpd: out/chttpd-x86_64 out/chttpd-arm64
	$(LIPO) -create -output $@ $^

out/chttpd-x86_64: $(OBJS_X86_64)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(CHTTPD_LDFLAGS) $(LDFLAGS) $(TARGET_FLAG_X86_64)

out/chttpd-arm64: $(OBJS_ARM64)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(CHTTPD_LDFLAGS) $(LDFLAGS) $(TARGET_FLAG_ARM64)

out/x86_64/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(DEPFLAGS_X86_64) $(CHTTPD_CFLAGS) $(CFLAGS) -c -o $@ $< $(TARGET_FLAG_X86_64)

out/arm64/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(DEPFLAGS_ARM64) $(CHTTPD_CFLAGS) $(CFLAGS) -c -o $@ $< $(TARGET_FLAG_ARM64)

out/x86_64/version.o: out/version.c
	mkdir -p $(@D)
	$(CC) $(CHTTPD_CFLAGS) $(CFLAGS) -c -o $@ $< $(TARGET_FLAG_X86_64)

out/arm64/version.o: out/version.c
	mkdir -p $(@D)
	$(CC) $(CHTTPD_CFLAGS) $(CFLAGS) -c -o $@ $< $(TARGET_FLAG_ARM64)

endif  # UNIVERSAL_BINARY


.PHONY: test
.NOTPARALLEL: test
test: all
	@$(MAKE) $(TESTS) --no-print-directory
	@echo "All tests passed."

.PHONY: test-asan
test-asan:
	@$(MAKE) CFLAGS="-O0 -g -fsanitize=address -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address" test

.PHONY: $(TESTS)
$(TESTS):
	@set -e; \
		test -t 1 && red="\033[31m" || red=""; \
		test -t 1 && green="\033[32m" || green=""; \
		test -t 1 && yellow="\033[33m" || yellow=""; \
		test -t 1 && reset="\033[0m" || reset=""; \
		echo "[ $${yellow}TESTING$${reset} ] $@"; \
		( \
			bash $@ && \
			echo "[ $${green}PASSED$${reset}  ] $@" \
		) || ( \
			echo "[ $${red}FAILED$${reset}  ] $@" && \
			exit 1 \
		)


.PHONY: clean
clean:
	rm -rf out chttpd
