OBJS = reciter.o sam.o main.o
		
CC = gcc
CFLAGS =  -Wall -O2 -DUSESDL `sdl-config --cflags`
LFLAGS = `sdl-config --libs`

Sam: $(OBJS)
	$(CC) -osam $(OBJS) $(LFLAGS)

main.o: main.c sam.h
	$(CC) $(CFLAGS) -c main.c
	
reciter.o: reciter.c reciter.h
	$(CC) $(CFLAGS) -c reciter.c
	
sam.o: sam.c sam.h
	$(CC) $(CFLAGS) -c sam.c	
	
tar: 
	tar -cvzf sam.tar.gz Makefile main.c reciter.c reciter.h ReciterTabs.h sam.c sam.h SamTabs.h

clean:
	rm *.o