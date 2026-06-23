# Project: cvemu2
# Makefile modernizzato e semplificato


BINDIR = /usr/local/bin

DEFINES+=PROGRAMRELEASE

CPP  = g++
BIN  = bin/cvemu2
LIBS = -lSDL2

# CFLAGS include "-I." così i file cpp possono includere gli header senza percorsi complessi
CFLAGS = -w -g -O2 -I.

OBJ  = video/tms9918.o mem/cvmemory.o cpu/cpu6502.o pia/6821pia.o \
       video/drv9918.o font/font.o main.o audio/sn76496.o \
       menu/menu.o savestate/savestate.o keyboard/keyboard.o

.PHONY: all clean install

all: $(BIN)

# Regola generica: spiega a Make come trasformare un .cpp in un .o
%.o: %.cpp
	$(CPP) -c $< -o $@ $(CFLAGS)

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CPP) $(OBJ) -o $(BIN) $(LIBS)

clean:
	rm -f $(OBJ) $(BIN)

install:
	cp $(BIN) $(BINDIR)
