CC = gcc
CFLAGS = -c
OBJECTS = client.o

DEBUG_FLAGS = -g
DEBUG_OBJECTS = client.c

app: $(OBJECTS)
	$(CC) $(OBJECTS) -Wall -o client

debug: $(DEBUG_OBJECTS)
	$(CC) $(DEBUG_FLAGS) $(DEBUG_OBJECTS) -Wall -o client

clean:
	rm $(OBJECTS)



client.o: client.c
	$(CC) $(CFLAGS) client.c
