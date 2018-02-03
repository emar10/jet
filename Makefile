LIBS=-lncurses

ned: ned.c
	$(CC) ned.c -o ned -Wall -Wextra -pedantic -std=c99 $(LIBS)

clean:
	rm -rf ned
