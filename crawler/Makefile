OBJS      = mycrawler.o crawler.o queue.o threads.o
SOURCE    = mycrawler.c crawler.c queue.c threads.c
HEADER    = crawler.h queue.h threads.h
OUT       = mycrawler
CC        = gcc
FLAGS     = -g -c

$(OUT): $(OBJS)
	@$(CC) -g -o $(OUT) $(OBJS) -lm -pthread
	@echo Object files linked, $(OUT) created.

mycrawler.o: mycrawler.c crawler.o
	@$(CC) $(FLAGS) mycrawler.c
	@echo mycrawler.o created.

crawler.o: crawler.c crawler.h threads.o queue.o
	@$(CC) $(FLAGS) -o crawler.o crawler.c
	@echo crawler.o created.

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
