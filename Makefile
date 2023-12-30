CC=gcc
OBJDIR:=$(shell [ -d obj ] || mkdir obj && echo "obj")
CFLAGS=-Wall -Wextra -std=gnu11 -D_POSIX_C_SOURCE=200112L $(shell pkg-config openssl --cflags)
LDFLAGS=$(shell pkg-config openssl --libs)

TARGETS=crawler.out
crawler.out_OBJ= crawler.o tcp_helper.o

.PHONY: all

all: CFLAGS:=$(CFLAGS) -O3
all: $(TARGETS) 

debug: CFLAGS:=$(CFLAGS) -g -DDEBUG -fsanitize=leak -fsanitize=undefined
debug: LDFLAGS:=$(LDFLAGS) -fsanitize=address -lubsan -lasan 
debug: $(TARGETS)

dev: CFLAGS:=$(CFLAGS) -g -DDEBUG
dev: $(TARGETS)

.SECONDEXPANSION:
$(TARGETS): $$(patsubst %, $(OBJDIR)/%, $$($$@_OBJ))
	$(CC) $(filter %.o, $^) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm -rf $(TARGETS) $(BUILD_DIR) obj