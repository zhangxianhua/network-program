CC = arm-histbv310-linux-gcc $(CCFlags)
CCFlags = -g -Wall

.PRECIOUS: *.o

%.o: %.c
	$(CC) -c $<

boardDaemon: boardDaemon.o
	$(CC) -o boardDaemon $<


all: boardDaemon 

clean:
	rm *.o boardDaemon 
