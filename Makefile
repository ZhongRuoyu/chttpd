CFLAGS	= -std=c11

SRC	= src/chttpd.c
BIN	= chttpd

DEBUG ?= 0
ifeq ($(DEBUG), 0)
    CFLAGS += -O2
else
    CFLAGS += -O0
endif

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	$(RM) $(BIN)
