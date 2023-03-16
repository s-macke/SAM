OBJS = reciter.o sam.o render.o main.o debug.o

CC = gcc

# libsdl present
CFLAGS =  -Wall -Os -DUSESDL `sdl2-config --cflags`
LFLAGS = `sdl2-config --libs`

# no libsdl present
#CFLAGS =  -Wall -Os
#LFLAGS =

sam: $(OBJS)
	$(CC) -o sam $(OBJS) $(LFLAGS)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $<

package:
	tar -cvzf sam.tar.gz README.md Makefile sing src/

clean:
	rm -f *.o

archive:
	rm -f sam_windows.zip
	cd ..; zip SAM/sam_windows.zip	SAM/sam.exe SAM/SDL.dll SAM/README.md SAM/demos/*.bat
