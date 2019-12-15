LOGLEVEL?=2 
REMOTE?=pi@pi:~/oku/

SPI_BACKEND?=wp
DEVICE?=emulated

CC=cc
CFLAGS= -Wall -Wextra -Wfatal-errors -g3 -O0 -DLOGLEVEL=$(LOGLEVEL) -I./src

LIBS=-lwiringPi

TARGET=oku_test
OBJ=spi_${SPI_BACKEND}.o epd_${DEVICE}.o bitmap.o

.PHONY: all clean tags test sync

all: $(TARGET)

$(TARGET): $(OBJ) $(TARGET).o
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIBS)

%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f display.pbm

tags:
	etags src/*.c src/*.h

test: clean all
	-valgrind --leak-check=full --errors-for-leak-kinds=all \
	--error-exitcode=33 --quiet ./$(TARGET)

sync: clean
	rsync -rav --exclude '.git' -e ssh --delete . $(REMOTE)

emulate: test
	mupdf display.pbm
