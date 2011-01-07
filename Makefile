# This is a GNU Makefile for gcc 4.2+

NAME=goatbrot

# options for OpenMP support
GBCCOPTS=-Wall -Wextra -O2 -fopenmp -DGBOPENMP -g
GBLDOPTS=-fopenmp
GBLIBS=-lm

# options for single-threaded (no OpenMP):
#GBCCOPTS=-Wall -Wextra -O2
#GBLDOPTS=
#GBLIBS=-lm

all: $(NAME)

$(NAME): $(NAME).o
	$(CC) $(GBLDOPTS) -o $@ $< $(GBLIBS)

$(NAME).o: $(NAME).c
	$(CC) -c $(GBCCOPTS) $<

clean:
	rm -f $(NAME).o

.PHONY: clean all

