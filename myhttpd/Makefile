OBJS      = myhttpd.o server.o threads.o queue.o
SOURCE    = myhttpd.c server.c threads.c queue.c
HEADER    = server.h threads.h queue.h
OUT       = myhttpd
CC        = gcc
FLAGS     = -g -c

$(OUT): $(OBJS)
	@$(CC) -g -o $(OUT) $(OBJS) -lm -pthread
	@echo Object files linked, $(OUT) created.

myhttpd.o: myhttpd.c server.o
	@$(CC) $(FLAGS) myhttpd.c
	@echo myhttpd.o created.

server.o: server.c server.h threads.o queue.o
	@$(CC) $(FLAGS) -o server.o server.c
	@echo server.o created.

queue.o: queue.c queue.h
	@$(CC) $(FLAGS) -o queue.o queue.c
	@echo queue.o created.

threads.o: threads.c threads.h
	@$(CC) $(FLAGS) -o threads.o threads.c -pthread
	@echo threads.o created.

clean:
	@rm -f $(OBJS) $(OUT)
	@echo Object files and \'$(OUT)\' removed.

count:
	@wc $(SOURCE) $(HEADER)

touch:
	@touch */*.c
