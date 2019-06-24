CC=gcc
CFLAGS = `mysql_config --cflags` -g -Wall -Wextra
LFLAGS = `mysql_config --libs`

all: RunDB

RunDB: RunDB.c makefile
	$(CC) $(CFLAGS) -o RunDB RunDB.c $(LFLAGS)

clean:
	rm -f RunDB
