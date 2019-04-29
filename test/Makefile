CC := clang++
CFLAGS := -g -Wall -lpthread -O0
SRCS := $(wildcard *.cc)

all: $(SRCS:.cc=)

clean:
	    rm -rf $(SRCS:.cc=)

%: %.cc
	    $(CC) $(CFLAGS) -o $@ $^
