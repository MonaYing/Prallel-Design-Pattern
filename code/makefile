CC = mpicc
CFLAGS = -std=gnu99 -O3
LFLAGS = -lm

EXE = model

all: 
	$(MAKE) -C framework
	$(MAKE) -C biology
	$(CC) $(CFLAGS) -o $(EXE)  framework/*.o biology/*.o $(LFLAGS)

.PHONY: clean
clean:
	$(MAKE) clean -C framework
	$(MAKE) clean -C biology
	rm -rf model
