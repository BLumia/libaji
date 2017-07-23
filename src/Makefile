# libaji
CC = gcc
CFLAGS = -c -O3 -I../include/

libaji.a: configman.o logger.o threadpool.o
	ar -rv libaji.a configman.o logger.o threadpool.o
	mv libaji.a ../lib/

all: libaji.a
	@echo "DONE"

clean:
	rm *.o  
	rm ../lib/libaji.a

configman.o: configman.c
	$(CC) $(CFLAGS) configman.c

logger.o: logger.c
	$(CC) $(CFLAGS) logger.c

threadpool.o: threadpool.c
	$(CC) $(CFLAGS) threadpool.c

.PHONY: clean all 
