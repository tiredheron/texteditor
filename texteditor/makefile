CC = gcc
CFLAGS = -I./lib -Wall -g 
LDFLAGS =
LIBS =

ifeq ($(OS),Windows_NT)
    # Windows
    LIBS = ./lib/pdcurses.a
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        # Linux
        CFLAGS += -I/usr/include/ncurses
        LDFLAGS += -L/usr/lib
        LIBS = -lncurses
        CHECK_NCURSES := $(shell ldconfig -p | grep libncurses)
    else ifeq ($(UNAME_S),Darwin)
        # MacOS
        CFLAGS += -I/opt/homebrew/include
        LDFLAGS += -L/opt/homebrew/lib
        LIBS = -lncurses
    endif
endif


TARGET = viva
SRCS = viva.c
OBJS = $(SRCS:.c=.o)


$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)