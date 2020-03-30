CC = mpicc
CFLAGS = -std=gnu99 -O3
LFLAGS = -lm

INC = actor.h factory.h pool.h
SRC = actor.c main.c pool.c
OBJ = $(SRC:.c=.o)

framework: $(OBJ)

$(OBJ): %o: %c $(INC)
	$(CC) $(CFLAGS) -c $< $(LFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJ)