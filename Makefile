LIBS=-lncurses
ARGS=ned.c -o ned -Wall -Wextra -pedantic -std=c99

ned: ned.c
	$(CC) $(ARGS) $(LIBS)

debug: ned.c
	$(CC) -g $(ARGS) $(LIBS)

clean:
	rm -rf ned
