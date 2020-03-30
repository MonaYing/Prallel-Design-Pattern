CC = mpicc
CFLAGS = -std=gnu99 -O3
LFLAGS = -lm

INC = ran2.h squirrel-functions.h  biology_factory.h biology_config.h
SRC = biology_timer.c biology_cell.c ran2.c biology_squirrel.c squirrel-functions.c biology_main.c biology_factory.c
OBJ = $(SRC:.c=.o)

biology: $(OBJ)

$(OBJ): %o: %c $(INC)
	$(CC) $(CFLAGS) -c $< $(LFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJ)
