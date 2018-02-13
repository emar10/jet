LIBS=-lncurses
ARGS=-o jet -Wall -Wextra -pedantic -std=c99

jet: jet.c
	$(CC) jet.c $(ARGS) $(LIBS)

debug: jet.c
	$(CC) jet.c -g $(ARGS) $(LIBS)

clean:
	rm -rf jet
