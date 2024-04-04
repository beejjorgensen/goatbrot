# This is a GNU Makefile for gcc 4.2+

NAME=goatbrot

OS=$(shell uname)

#DEBUG=-g

ifeq ($(OS),Linux)
	CCOPTS=-Wall -Wextra -O2 -fopenmp $(DEBUG)
	LDOPTS=-fopenmp -lm
else ifeq ($(OS),Darwin)
	CCOPTS=-Wall -Wextra -O2 -Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include/ $(DEBUG)
	LDOPTS=-Xpreprocessor -fopenmp -L/usr/local/opt/libomp/lib/ -lomp
endif

all: $(NAME)

$(NAME): $(NAME).o
	$(CC) $(LDOPTS) -o $@ $< $(GBLIBS)

$(NAME).o: $(NAME).c
	$(CC) -c $(CCOPTS) $<

clean:
	rm -f $(NAME).o

.PHONY: clean all

