OBJS      = myhttpd.o server.o
SOURCE    = myhttpd.c server.c
HEADER    = server.h
OUT       = myhttpd
CC        = gcc
FLAGS     = -g -c

$(OUT): $(OBJS)
	@$(CC) -g -o $(OUT) $(OBJS) -lm
	@echo Object files linked, $(OUT) created.

myhttpd.o: myhttpd.c server.o
	@$(CC) $(FLAGS) myhttpd.c
	@echo myhttpd.o created.

server.o: server.c server.h
	@$(CC) $(FLAGS) -o server.o server.c
	@echo server.o created.

clean:
	@rm -f $(OBJS) $(OUT)
	@echo Object files and \'$(OUT)\' removed.

count:
	@wc $(SOURCE) $(HEADER)

touch:
	@touch */*.c
