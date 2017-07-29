# test of libaji

CC = gcc
CFLAGS = -L../lib/ -I../include/ -laji -lrt -pthread -g -std=c11

all: threadpool.elf logger.elf configman.elf
	@echo "Done!"

threadpool.elf: threadpool.c
	$(CC) threadpool.c $(CFLAGS) -o $@
	
logger.elf: logger.c
	$(CC) logger.c $(CFLAGS) -o $@
	
configman.elf: configman.c
	$(CC) configman.c $(CFLAGS) -o $@

clean:
	rm *.elf

.PHONY: clean all 
