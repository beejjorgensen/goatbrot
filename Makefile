# This is a GNU Makefile for gcc 4.2+

NAME=goatbrot

OS=$(shell uname)

#DEBUG=-g

ifeq ($(OS),Linux)
	CCOPTS=-Wall -Wextra -O2 -fopenmp $(DEBUG)
	LDOPTS=-fopenmp -lm

else ifeq ($(OS),Darwin)
	SEARCHDIRS=/usr/local/opt/libomp /opt/homebrew/opt/libomp
	FOUNDDIRS=$(wildcard $(SEARCHDIRS))
	BASELIBDIR=$(word 1, $(FOUNDDIRS))
	CCOPTS=-Wall -Wextra -O2 -Xpreprocessor -fopenmp -I$(BASELIBDIR)/include/ $(DEBUG)
	LDOPTS=-Xpreprocessor -fopenmp -L$(BASELIBDIR)/lib/ -lomp
endif

all: $(NAME)

$(NAME): $(NAME).o
	$(CC) $(LDOPTS) -o $@ $<

$(NAME).o: $(NAME).c
	$(CC) -c $(CCOPTS) $<

clean:
	rm -f $(NAME).o

.PHONY: clean all

