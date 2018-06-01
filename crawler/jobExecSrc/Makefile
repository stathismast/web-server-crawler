OBJS1      = jobExecSrc/jobExecutor.o jobExecSrc/commands.o jobExecSrc/signalHandler.o jobExecSrc/ioManager.o jobExecSrc/pipes.o
SOURCE1    = jobExecSrc/jobExecutor.c jobExecSrc/commands.c jobExecSrc/signalHandler.c jobExecSrc/ioManager.c jobExecSrc/pipes.c
HEADER1    = jobExecSrc/commands.h jobExecSrc/signalHandler.h jobExecSrc/ioManager.h jobExecSrc/pipes.h
OUT1       = jobExecutor

OBJS2      = workersSrc/worker.o workersSrc/commands.o workersSrc/signalHandler.o workersSrc/pipes.o workersSrc/searchInfo.o workersSrc/dirInfo.o workersSrc/trie.o
SOURCE2    = workersSrc/worker.c workersSrc/commands.c workersSrc/signalHandler.c workersSrc/pipes.c workersSrc/searchInfo.c workersSrc/dirInfo.c workersSrc/trie.c
HEADER2    = workersSrc/commands.h workersSrc/signalHandler.h workersSrc/pipes.h workersSrc/searchInfo.h workersSrc/dirInfo.h workersSrc/trie.h
OUT2       = worker

CC        = gcc
FLAGS     = -g -c

all: $(OUT1) $(OUT2)

$(OUT1): $(OBJS1)
	@$(CC) -g -o $(OUT1) $(OBJS1) -lm
	@echo Object files linked, $(OUT1) created.

$(OUT2): $(OBJS2)
	@$(CC) -g -o $(OUT2) $(OBJS2) -lm
	@echo Object files linked, $(OUT2) created.

jobExecSrc/jobExecutor.o: jobExecSrc/jobExecutor.c jobExecSrc/commands.o
	@$(CC) $(FLAGS) jobExecSrc/jobExecutor.c
	@echo jobExecutor.o created.

jobExecSrc/commands.o: jobExecSrc/commands.c jobExecSrc/commands.h jobExecSrc/signalHandler.o
	@$(CC) $(FLAGS) -o jobExecSrc/commands.o jobExecSrc/commands.c
	@echo jobExecSrc/commands.o created.

jobExecSrc/signalHandler.o: jobExecSrc/signalHandler.c jobExecSrc/signalHandler.h jobExecSrc/ioManager.o
	@$(CC) $(FLAGS) -o jobExecSrc/signalHandler.o jobExecSrc/signalHandler.c
	@echo jobExecSrc/signalHandler.o created.

jobExecSrc/ioManager.o: jobExecSrc/ioManager.c jobExecSrc/ioManager.h jobExecSrc/pipes.o
	@$(CC) $(FLAGS) -o jobExecSrc/ioManager.o jobExecSrc/ioManager.c
	@echo jobExecSrc/ioManager.o created.

jobExecSrc/pipes.o: jobExecSrc/pipes.c jobExecSrc/pipes.h
	@$(CC) $(FLAGS) -o jobExecSrc/pipes.o jobExecSrc/pipes.c
	@echo jobExecSrc/pipes.o created.


###############################################################################


workersSrc/worker.o: workersSrc/worker.c workersSrc/commands.o
	@$(CC) $(FLAGS) workersSrc/worker.c
	@echo workersSrc/worker.o created.

workersSrc/commands.o: workersSrc/commands.c workersSrc/commands.h workersSrc/signalHandler.o
	@$(CC) $(FLAGS) -o workersSrc/commands.o workersSrc/commands.c
	@echo workersSrc/commands.o created.

workersSrc/signalHandler.o: workersSrc/signalHandler.c workersSrc/signalHandler.h workersSrc/pipes.o workersSrc/searchInfo.o
	@$(CC) $(FLAGS) -o workersSrc/signalHandler.o workersSrc/signalHandler.c
	@echo workersSrc/signalHandler.o created.

workersSrc/pipes.o: workersSrc/pipes.c workersSrc/pipes.h
	@$(CC) $(FLAGS) -o workersSrc/pipes.o workersSrc/pipes.c
	@echo workersSrc/pipes.o created.

workersSrc/searchInfo.o: workersSrc/searchInfo.c workersSrc/searchInfo.h workersSrc/dirInfo.o
	@$(CC) $(FLAGS) -o workersSrc/searchInfo.o workersSrc/searchInfo.c
	@echo workersSrc/searchInfo.o created.

workersSrc/dirInfo.o: workersSrc/dirInfo.c workersSrc/dirInfo.h workersSrc/trie.o
	@$(CC) $(FLAGS) -o workersSrc/dirInfo.o workersSrc/dirInfo.c
	@echo workersSrc/dirInfo.o created.

workersSrc/trie.o: workersSrc/trie.c workersSrc/trie.h
	@$(CC) $(FLAGS) -o workersSrc/trie.o workersSrc/trie.c
	@echo workersSrc/trie.o created.



clean:
	@rm -f $(OBJS1) $(OUT1)
	@rm -f $(OBJS2) $(OUT2)
	@echo Object files, \'$(OUT1)\' and \'$(OUT2)\' removed.

count:
	@wc $(SOURCE1) $(HEADER1) $(SOURCE2) $(HEADER2)

touch:
	@touch */*.c
