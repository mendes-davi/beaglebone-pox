CC=gcc
SRCDIR=./src
ODIR=./src/obj
BINDIR=./bin
INCDIR =./include

CFLAGS=-I$(INCDIR) -Wall
LIBS=-L./lib -lm -lpthread

_DEPS = I2C.h log.h MAX30100.h MAX30100_BeatDetector.h MAX30100_Filters.h MAX30100_Log.h MAX30100_Timer.h wiringSerial.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = I2C.o MAX30100.o MAX30100_BeatDetector.o MAX30100_Filters.o MAX30100_PulseOximeter.o MAX30100_Timer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ2 = I2C.o log.o MAX30100.o MAX30100_Filters.o MAX30100_Log.o MAX30100_Master.o MAX30100_Timer.o wiringSerial.o
OBJ2 = $(patsubst %,$(ODIR)/%,$(_OBJ2))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

MAX30100_PulseOximeter: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	mv $@ $(BINDIR)/pox

MAX30100_Master: $(OBJ2)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	mv $@ $(BINDIR)/mpox

# https://github.com/nickdiego/compiledb used to generate compile_commands.json for Clangd LSP
compile_commands:
	compiledb --no-build make

TAGS:
	find . -type f -name "*.[ch]" | xargs ctags

all: compile_commands TAGS MAX30100_PulseOximeter MAX30100_Master

clean:
	rm -f $(ODIR)/*.o

.PHONY: clean all TAGS MAX30100_Master MAX30100_PulseOximeter
# vim: set noexpandtab
