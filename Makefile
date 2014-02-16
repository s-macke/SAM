OBJS = reciter.o sam.o main.o

CC = gcc
CFLAGS =  -Wall -O2 -DUSESDL `sdl-config --cflags`
LFLAGS = `sdl-config --libs`
#CFLAGS =  -Wall -O2
#LFLAGS = 

sam: $(OBJS)
	$(CC) -o sam $(OBJS) $(LFLAGS)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $<

tar: 
	tar -cvzf sam.tar.gz README.md Makefile sing src/main.c src/reciter.c src/reciter.h src/ReciterTabs.h src/sam.c src/sam.h src/SamTabs.h

clean:
	rm *.o

