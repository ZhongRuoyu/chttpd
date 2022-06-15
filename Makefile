SRCDIR = src
OUTDIR = out
BINDIR = bin

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OUTDIR)/%.o)

CFLAGS = -std=c11 -O2

all: chttpd

chttpd: $(BINDIR)/chttpd
$(BINDIR)/chttpd: $(OBJS)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $^

$(OUTDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf $(BINDIR) $(OUTDIR)

.PHONY: clean
