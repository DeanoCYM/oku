LOGLEVEL?=2
REMOTE?=pi@pi:~/oku/

# Define backends
SPI_BACKEND?=wp
DEVICE?=emulated
RENDER?=freetype

# Complication arguements
CC=cc
LIBS= -lwiringPi -lfreetype -lm
INCLUDE= -I./src -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include 
CFLAGS= -Wall -Wextra -Wfatal-errors -g3 -O0 -DLOGLEVEL=$(LOGLEVEL) $(INCLUDE)

# Definition of target executable and libraries
TARGET=oku_test
OBJ=oku_mem.o spi_${SPI_BACKEND}.o epd_${DEVICE}.o bitmap.o #text_${RENDER}.o

.PHONY: all clean tags test sync emulate

# Compalation of Target Executable
all: $(TARGET)
$(TARGET): $(OBJ) $(TARGET).o
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIBS)

# Object Compilation
%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

# Utilities
clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f display.pbm char.pbm
	rm -f vgcore.*
tags:
	etags src/*.c src/*.h

test: clean all
	-valgrind --leak-check=full --errors-for-leak-kinds=all \
	--error-exitcode=33 --quiet ./$(TARGET)

sync: clean
	rsync -rav --exclude '.git' -e ssh --delete . $(REMOTE)

emulate: DEVICE=emulated
emulate: test 
	mupdf display.pbm

remote: sync
	ssh pi@pi "cd oku && sed -i 's/emulated/ws29bw/' Makefile && make test"

# Debugging
mwe: mwe.c
	$(CC) $(CFLAGS) -o $@ mwe.c $(LIBS)
